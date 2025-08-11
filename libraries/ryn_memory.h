/*
  ryn_memory v0.00 - Memory arena utilities. Very early in development...

  TODO:
    [x] Implement BeginArena/EndArena functions instead of forcing the user to store arena-offsets in variables, which introduces too many un-needed names into their codebase.
*/
#ifndef __RYN_MEMORY__
#define __RYN_MEMORY__

/*
  Prefix Definition
  =================
  By default, this library will prefix all exported types/functions with "ryn_memory_" to avoid clashing with other identifiers in the codebase. So, types like "arena" are called "ryn_memory_arena".

  Below is a hack that attempts to give the user some namespacing control. This library defines all exported identifiers via the "ryn_memory_" macro, so that the user can redefine "ryn_memory_" and change the prefix, including having no prefix at all.

  This method makes the library code a little less readible, but it ain't so bad once you get used to it...
  Also, we cannot use this methos on macros, so you have to manually define your own, but that's not so bad, right?

  Examples:
  - No Prefix
    #define ryn_memory_(identifier) identifier
    #include "ryn_memory.h"
    void MyArenaFunction(arena Arena)
    {
        ...
    }

  - Custom Prefix
    #define ryn_memory_(identifier) my_cool_prefix##identifier
    #include "ryn_memory.h"
    void MyArenaFunction(my_cool_prefix_arena Arena)
    {
        ...
    }
*/
#ifndef ryn_memory_
#define ryn_memory_(identifier) ryn_memory_##identifier
#endif

/*
  The following are macros that you would want to define wrappers for, in order to change the macro's prefix.

  Examples:
  - No Prefix
    #define PushStruct(arena, type) ryn_memory_PushStruct(arena, type)

  - Custom Prefix
    #define my_cool_prefix_PushStruct(arena, type) ryn_memory_PushStruct(arena, type)
*/
#define ryn_memory_PushStruct(arena, type) \
    (type *)(ryn_memory_(PushSize)((arena), sizeof(type)))

#define ryn_memory_PushArray(arena, type, count) \
    (type *)(ryn_memory_(PushSize)((arena), (count)*sizeof(type)))

#define ryn_memory_PushZeroStruct(arena, type) \
    (type *)(ryn_memory_(PushZeroArena)((arena), sizeof(type)))

#define ryn_memory_PushZeroArray(arena, type, count) \
    (type *)(ryn_memory_(PushZeroArena)((arena), (count)*sizeof(type)))

#define ryn_memory_BeginArena(arena) uint64_t ryn_memory_##arena##_OldOffset = (arena)->Offset
#define ryn_memory_EndArena(arena) (arena)->Offset = ryn_memory_##arena##_OldOffset




#if defined(__MACH__) || defined(__APPLE__)
#define ryn_memory_Mac 1
#define ryn_memory_Operating_System 1
#elif defined(_WIN32)
#define ryn_memory_Windows 1
#define ryn_memory_Operating_System 1
#endif

#ifndef ryn_memory_Operating_System
#error Unhandled operating system.
#endif

#if ryn_memory_Windows
#include <windows.h>
#include <memoryapi.h>
#elif ryn_memory_Mac
#include <sys/mman.h>
#include "memory.h"
#include <errno.h>
#endif

#include <stdio.h> /* TODO: Create a way to disable or replace printf. */

#include <stdint.h>



typedef struct
{
    uint64_t Offset;
    uint64_t Capacity;
    uint8_t *Data;
    uint64_t ParentOffset;
} ryn_memory_(arena);

void *ryn_memory_AllocateVirtualMemory(size_t Size);

ryn_memory_(arena) ryn_memory_(CreateArena)(uint64_t Size);
void *ryn_memory_(PushSize)(ryn_memory_(arena) *Arena, uint64_t Size);
uint64_t ryn_memory_(GetArenaFreeSpace)(ryn_memory_(arena) *Arena);
ryn_memory_(arena) ryn_memory_(CreateSubArena)(ryn_memory_(arena) *Arena, uint64_t Size);
uint32_t ryn_memory_(IsArenaUsable)(ryn_memory_(arena) Arena);
int32_t ryn_memory_(WriteArena)(ryn_memory_(arena) *Arena, uint8_t *Data, uint64_t Size);
uint8_t *ryn_memory_(GetArenaWriteLocation)(ryn_memory_(arena) *Arena);
uint32_t ryn_memory_(FreeArena)(ryn_memory_(arena) Arena);




void ryn_memory_(CopyMemory)(uint8_t *Source, uint8_t *Destination, uint64_t Size)
{
    /* @Speed */
    for (uint64_t I = 0; I < Size; I++)
    {
        Destination[I] = Source[I];
    }
}

#if ryn_memory_Mac
void *ryn_memory_(AllocateVirtualMemory)(size_t Size)
{
    /* TODO allow setting specific address for debugging with stable pointer values */
    uint8_t *Address = 0;
    int Protections = PROT_READ | PROT_WRITE;
    int Flags = MAP_ANON | MAP_PRIVATE;
    int FileDescriptor = 0;
    int Offset = 0;

    uint8_t *Result = mmap(Address, Size, Protections, Flags, FileDescriptor, Offset);

    if (Result == MAP_FAILED)
    {
        char *ErrorName = 0;

        switch(errno)
        {

        case EACCES: ErrorName = "EACCES"; break;
        case EBADF: ErrorName = "EBADF"; break;
        case EINVAL: ErrorName = "EINVAL"; break;
        case ENODEV: ErrorName = "ENODEV"; break;
        case ENOMEM: ErrorName = "ENOMEM"; break;
        case ENXIO: ErrorName = "ENXIO"; break;
        case EOVERFLOW: ErrorName = "EOVERFLOW"; break;
        default: ErrorName = "<Unknown errno>"; break;
        }

        printf("Error in AllocateVirtualMemory: failed to map memory with errno = \"%s\"\n", ErrorName);
        return 0;
    }
    else
    {
        return Result;
    }
}
#elif ryn_memory_Windows
void *ryn_memory_(AllocateVirtualMemory)(size_t Size)
{
    void *Result = VirtualAlloc(0, Size, MEM_RESERVE, PAGE_READWRITE);
    return Result;
}
#endif

ryn_memory_(arena) ryn_memory_(CreateArena)(uint64_t Size)
{
    ryn_memory_(arena) Arena;

    Arena.Offset = 0;
    Arena.Capacity = Size;
    Arena.Data = ryn_memory_(AllocateVirtualMemory)(Size);
    Arena.ParentOffset = 0;

    return Arena;
}


void *ryn_memory_(PushSize)(ryn_memory_(arena) *Arena, uint64_t Size)
{
    uint8_t *Result = 0;

    if (Arena && (Size + Arena->Offset) < Arena->Capacity)
    {
        Result = &Arena->Data[Arena->Offset];
        Arena->Offset += Size;
    }

    return Result;
}


void *ryn_memory_(PushZeroArena)(ryn_memory_(arena) *Arena, uint64_t Size)
{
    uint8_t *Result = 0;

    if (Arena && (Size + Arena->Offset) < Arena->Capacity)
    {
        Result = &Arena->Data[Arena->Offset];
        Arena->Offset += Size;
        memset(Result, 0, Size);
    }

    return Result;
}


uint64_t ryn_memory_(GetArenaFreeSpace)(ryn_memory_(arena) *Arena)
{
    uint64_t FreeSpace = 0;

    if (Arena->Capacity > Arena->Offset)
    {
        FreeSpace = Arena->Capacity - Arena->Offset;
    }

    return FreeSpace;
}


uint64_t ryn_memory_(PushChar)(ryn_memory_(arena) *Arena, uint8_t Char)
{
    uint64_t BytesWritten = 0;

    if (Arena && ryn_memory_(GetArenaFreeSpace)(Arena))
    {
        uint8_t *WriteLocation = ryn_memory_(PushSize)(Arena, 1);
        WriteLocation[0] = Char;
        BytesWritten = 1;
    }

    return BytesWritten;
}

uint64_t ryn_memory_(PushBytes)(ryn_memory_(arena) *Arena, uint8_t *Bytes, uint64_t Size)
{
    uint64_t BytesWritten = 0;

    if (Size <= ryn_memory_(GetArenaFreeSpace)(Arena))
    {
        uint8_t *WriteLocation = ryn_memory_(PushSize)(Arena, Size);
        BytesWritten = Size;

        for (uint64_t I = 0; I < Size; ++I)
        {
            WriteLocation[I] = Bytes[I];
        }
    }

    return BytesWritten;
}

uint64_t ryn_memory_(PushNullTerminatedBytes)(ryn_memory_(arena) *Arena, uint8_t *Bytes, uint64_t Size)
{
    uint64_t TotalBytesWritten = 0;
    uint64_t NullWritten = 0;

    uint64_t BytesWritten = ryn_memory_(PushBytes)(Arena, Bytes, Size);
    if (BytesWritten)
    {
        NullWritten = ryn_memory_(PushBytes)(Arena, (uint8_t *)"", 1);
    }

    if (BytesWritten != 0 && NullWritten != 0)
    {
        TotalBytesWritten = BytesWritten + NullWritten;
    }

    return TotalBytesWritten;
}

ryn_memory_(arena) ryn_memory_(CreateSubArena)(ryn_memory_(arena) *Arena, uint64_t Size)
{
    ryn_memory_(arena) SubArena = {0};

    if (Size <= ryn_memory_(GetArenaFreeSpace)(Arena))
    {
        SubArena.Capacity = Size;
        SubArena.Data = ryn_memory_(GetArenaWriteLocation)(Arena);
        ryn_memory_(PushSize)(Arena, Size);
    }

    return SubArena;
}

inline uint32_t ryn_memory_(IsArenaUsable)(ryn_memory_(arena) Arena)
{
    uint32_t IsUsable = Arena.Capacity && Arena.Data;
    return IsUsable;
}

int32_t ryn_memory_(WriteArena)(ryn_memory_(arena) *Arena, uint8_t *Data, uint64_t Size)
{
    int32_t ErrorCode = 0;
    uint8_t *WhereToWrite = ryn_memory_(PushSize)(Arena, Size);

    if (WhereToWrite)
    {
        ryn_memory_(CopyMemory)(Data, WhereToWrite, Size);
    }
    else
    {
        ErrorCode = 1;
    }

    return ErrorCode;
}

uint8_t *ryn_memory_(GetArenaWriteLocation)(ryn_memory_(arena) *Arena)
{
    uint8_t *WriteLocation = Arena->Data + Arena->Offset;
    return WriteLocation;
}

#if ryn_memory_Mac
uint32_t ryn_memory_(FreeArena)(ryn_memory_(arena) Arena)
{
    uint32_t Error = 0;

    if (Arena.Data && Arena.Capacity)
    {
        munmap(Arena.Data, Arena.Capacity);
    }
    else
    {
        Error = 1;
    }

    return Error;
}
#elif ryn_memory_Windows
uint32_t ryn_memory_(FreeArena)(ryn_memory_(arena) Arena)
{
    exit(1); /* TODO: Implement this... */
    return 1;
}
#endif











/* TODO: These push and pop functions have not been tested very much. We should
   write some nested use-cases to make sure push/pop work properly. */
void ryn_memory_(ArenaStackPush)(ryn_memory_(arena) *Arena)
{
    uint64_t NewParentOffset = Arena->Offset;
    ryn_memory_(WriteArena)(Arena, (uint8_t *)&Arena->ParentOffset, sizeof(Arena->ParentOffset));
    Arena->ParentOffset = NewParentOffset;
}

void ryn_memory_(ArenaStackPop)(ryn_memory_(arena) *Arena)
{
    Arena->Offset = Arena->ParentOffset;
    uint64_t *WriteLocation = (uint64_t *)ryn_memory_(GetArenaWriteLocation)(Arena);
    Arena->ParentOffset = *WriteLocation;
}







#endif /* __RYN_MEMORY__ */
