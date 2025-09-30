/*
    This is a copy of the base-layer provided by https://mr4th.com/

    The original base-layer was built for Windows, and this copy is slowly filling out code for Mac/Linux.

    TODO:
    [ ] Implement os_memory_* functions. Doing this will give access to arena functionality on Mac/Linux.
    [ ] Add linked-list macros for index based lists. (This is only important if we end up using index-based lists for processes in the app, which is only used to make saving to disk more convenient.)
*/

////////////////////////////////////////////////
////////////////////////////////////////////////
///////////////   INTRODUCTION   ///////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/*
**
** TODO: instructions will go here
**
*/

#if !defined(MR4TH_SYMBOL)
# define MR4TH_SYMBOL MR4TH_SYM_LINKTIME
#endif

#if !defined(MR4TH_DEFINE_RUNTIME_SYMBOLS)
# define MR4TH_DEFINE_RUNTIME_SYMBOLS 0
#endif


#if !defined(MR4TH_BASE_DECLARATIONS)
#define MR4TH_BASE_DECLARATIONS 1

////////////////////////////////////////////////
////////////////////////////////////////////////
///////////////   DECLARATIONS   ///////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

////////////////////////////////
// Context Cracking

// development settings
#if !defined(MR4TH_ASSERTS)
# define MR4TH_ASSERTS 0
#endif
#if !defined(MR4TH_SANITIZER)
# define MR4TH_SANITIZER 0
#endif
#if !defined(MR4TH_PROFILING_MANUAL)
# define MR4TH_PROFILING_MANUAL 0
#endif
#if !defined(MR4TH_PROFILING_AUTO)
# define MR4TH_PROFILING_AUTO 0
#endif

#if defined(MR4TH_PROFILING_USER)
# error user should not configure MR4TH_PROFILING_USER, instead set MR4TH_PROFILING_MANUAL or MR4TH_PROFILING_AUTO
#endif

#if MR4TH_PROFILING_MANUAL || MR4TH_PROFILING_AUTO
# define MR4TH_PROFILING_USER 1
#else
# define MR4TH_PROFILING_USER 0
#endif

// untangle compiler, os, & architecture
#if defined(__clang__)
# define COMPILER_CLANG 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__gnu_linux__)
#  define OS_LINUX 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# else
#  error missing OS detection
# endif

# if defined(__amd64__)
#  define ARCH_X64 1
// TODO(allen): verify this works on clang
# elif defined(__i386__)
#  define ARCH_X86 1
// TODO(allen): verify this works on clang
# elif defined(__arm__)
#  define ARCH_ARM 1
// TODO(allen): verify this works on clang
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# else
#  error missing ARCH detection
# endif

#elif defined(_MSC_VER)
# define COMPILER_CL 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# else
#  error missing OS detection
# endif

#if 0
# if defined(_M_AMD64)
#  define ARCH_X64 1
# elif defined(_M_I86)
#  define ARCH_X86 1
# elif defined(_M_ARM)
#  define ARCH_ARM 1
// TODO(allen): ARM64?
# else
#  error missing ARCH detection
# endif
#endif

#elif defined(__GNUC__)
# define COMPILER_GCC 1

# if defined(_WIN32)
#  define OS_WINDOWS 1
# elif defined(__gnu_linux__)
#  define OS_LINUX 1
# elif defined(__APPLE__) && defined(__MACH__)
#  define OS_MAC 1
# else
#  error missing OS detection
# endif

# if defined(__amd64__)
#  define ARCH_X64 1
# elif defined(__i386__)
#  define ARCH_X86 1
# elif defined(__arm__)
#  define ARCH_ARM 1
# elif defined(__aarch64__)
#  define ARCH_ARM64 1
# else
#  error missing ARCH detection
# endif

#else
# error no context cracking for this compiler
#endif

#if !defined(COMPILER_CL)
# define COMPILER_CL 0
#endif
#if !defined(COMPILER_CLANG)
# define COMPILER_CLANG 0
#endif
#if !defined(COMPILER_GCC)
# define COMPILER_GCC 0
#endif
#if !defined(OS_WINDOWS)
# define OS_WINDOWS 0
#endif
#if !defined(OS_LINUX)
# define OS_LINUX 0
#endif
#if !defined(OS_MAC)
# define OS_MAC 0
#endif
#if !defined(ARCH_X64)
# define ARCH_X64 0
#endif
#if !defined(ARCH_X86)
# define ARCH_X86 0
#endif
#if !defined(ARCH_ARM)
# define ARCH_ARM 0
#endif
#if !defined(ARCH_ARM64)
# define ARCH_ARM64 0
#endif


// language
#if defined(__cplusplus)
# define LANG_CXX 1
#else
# define LANG_C 1
#endif

#if !defined(LANG_CXX)
# define LANG_CXX 0
#endif
#if !defined(LANG_C)
# define LANG_C 0
#endif


// profiler
#if !defined(MR4TH_PROFILING_PROVIDER)
# define MR4TH_PROFILING_PROVIDER 0
#endif


// determine intrinsics mode
#if OS_WINDOWS
# if COMPILER_CL || COMPILER_CLANG
#  define INTRINSICS_MICROSOFT 1
# endif
#endif

#if !defined(INTRINSICS_MICROSOFT)
# define INTRINSICS_MICROSOFT 0
#endif


// setup pointer size macro
#if ARCH_X64 || ARCH_ARM64
# define ARCH_ADDRSIZE 64
#else
# define ARCH_ADDRSIZE 32
#endif


#define ARCH_LITTLE_ENDIAN 1
#define ARCH_BIG_ENDIAN    0


////////////////////////////////
// Macros: Helper

// macro writing utilities

#define Stmnt(S) do{ S }while(0)

#define Stringify_(S) #S
#define Stringify(S) Stringify_(S)
#define Glue_(A,B) A##B
#define Glue(A,B) Glue_(A,B)

// assert

#if !defined(AssertBreak)
# define AssertBreak() (*(volatile int*)0 = 0)
#endif

#if MR4TH_ASSERTS
# define Assert(c) Stmnt( if (!(c)){ AssertBreak(); } )
#else
# define Assert(c)
#endif

#define StaticAssert(c,l) typedef U8 Glue(l,__LINE__) [(c)?1:-1]

// linkage, storage, & other "attribute" like things

#define MR4TH_SYM_COMPTIME static

#if LANG_CXX
# define MR4TH_SYM_LINKTIME extern "C"
#else
# define MR4TH_SYM_LINKTIME extern
#endif

#if COMPILER_CL
# define MR4TH_SYM_RUNTIME MR4TH_SYM_LINKTIME __declspec(dllexport)
#elif COMPILER_CLANG
# if OS_WINDOWS
#  define MR4TH_SYM_RUNTIME MR4TH_SYM_LINKTIME __declspec(dllexport)
# else
#  define MR4TH_SYM_RUNTIME MR4TH_SYM_LINKTIME __attribute__((visibility("default")))
# endif
#elif COMPILER_GCC
# define MR4TH_SYM_RUNTIME MR4TH_SYM_LINKTIME __attribute__((visibility("default")))
#else
# error MR4TH_SYM_RUNTIME not defined for this compiler
#endif

#if COMPILER_CL
# define MR4TH_THREADVAR __declspec(thread)
#elif COMPILER_CLANG || COMPILER_GCC
# define MR4TH_THREADVAR __thread
#else
# error MR4TH_THREADVAR not defined for this compiler
#endif

// NOTE: For now, we add __attribute__((used)) for mac to prevent the optimizer from removing the sections
#if COMPILER_CLANG || COMPILER_GCC
# if OS_MAC
#  define MR4TH_SECNAME_R(N)  __attribute__((__section__("__TEXT,"N)))
#  define MR4TH_SECNAME_RW(N) __attribute__((__section__("__DATA,"N)))
#  define MR4TH_SECNAME(N) MR4TH_SECNAME_RW(N)
# else
#  define MR4TH_SECNAME_R(N) __attribute__((__section__(N)))
#  define MR4TH_SECNAME_RW(N) __attribute__((__section__(N)))
#  define MR4TH_SECNAME(N) MR4TH_SECNAME_RW(N)
# endif
#elif COMPILER_CL
# define MR4TH_SECNAME(N) __declspec(allocate(N))
#else
# error MR4TH_SECNAME not defined for this compiler
#endif

#if COMPILER_CLANG || COMPILER_GCC
#  define MR4TH_READ_ONLY MR4TH_SECNAME_R(".rdata")
#elif COMPILER_CL
# pragma section(".m4rdata",read)
# define MR4TH_READ_ONLY MR4TH_SECNAME(".m4rdata")
#else
# define MR4TH_READ_ONLY
# error MR4TH_READ_ONLY not defined for this compiler
#endif

#if MR4TH_DEFINE_RUNTIME_SYMBOLS
# define MR4TH_SHARED_FUNC(n,r,p) MR4TH_SYM_RUNTIME r n p
# define MR4TH_SHARED_FUNC_INITIALIZER(n) (Error!)
#else
# define MR4TH_SHARED_FUNC(n,r,p) typedef r n##_Type p; MR4TH_SYMBOL n##_Type *n
# define MR4TH_SHARED_FUNC_INITIALIZER(n) n##_Type *n = 0
#endif

// do-not-eliminate

#if OS_WINDOWS

# define MR4TH_DO_NOT_ELIMINATE__S0(N) __pragma(comment(linker,"/include:" #N))
# define MR4TH_DO_NOT_ELIMINATE__S1(N) MR4TH_DO_NOT_ELIMINATE__S0(N)
# define MR4TH_DO_NOT_ELIMINATE(N)     MR4TH_DO_NOT_ELIMINATE__S1(N)

#else

# define MR4TH_DO_NOT_ELIMINATE(N)

#endif

// before-main abstraction

#if OS_WINDOWS

# pragma section(".CRT$XCU", read)

# if LANG_CXX

#  define MR4TH_BEFORE_MAIN_(n) \
static void n(void);                          \
__declspec(allocate(".CRT$XCU"))              \
__pragma(comment(linker,"/include:" #n "__")) \
extern "C" void (*n##__)(void);               \
void (*n##__)(void) = n;                      \
static void n(void)

# else

#  define MR4TH_BEFORE_MAIN_(n) \
static void n(void);                          \
__declspec(allocate(".CRT$XCU"))              \
__pragma(comment(linker,"/include:" #n "__")) \
void (*n##__)(void) = n;                      \
static void n(void)

# endif

# define MR4TH_BEFORE_MAIN(n) MR4TH_BEFORE_MAIN_(n)

#elif OS_LINUX

# define MR4TH_BEFORE_MAIN(n) \
__attribute__((constructor)) static void n(void)

#elif OS_MAC
# define MR4TH_BEFORE_MAIN(n) \
__attribute__((constructor)) static void n(void)

#else
# error MR4TH_BEFORE_MAIN missing for this OS
#endif

// sanitizer

#if COMPILER_CLANG && MR4TH_SANITIZER
# include <sanitizer/asan_interface.h>
#endif

#if MR4TH_SANITIZER
# define AsanPoison(p,z)   __asan_poison_memory_region((p),(z))
# define AsanUnpoison(p,z) __asan_unpoison_memory_region((p),(z))
#else
# define AsanPoison(p,z)
# define AsanUnpoison(p,z)
#endif

// helpful expressions

#define ArrayCount(a) (sizeof(a)/sizeof(*(a)))

#define IntFromPtr(p) (UAddr)(p)
#define PtrFromInt(n) (void*)((UAddr)(n))
#define PtrDif(a,b) ((U8*)(a) - (U8*)(b))

#define Member(T,m) (((T*)0)->m)
#define AddrOfMember(T,m)   (void*)(&Member(T,m))
#define OffsetOfMember(T,m) IntFromPtr(&Member(T,m))
#define OffsetOfMemberV(s,m) PtrDif(&s->m, s)

#define MemberAddr(T,m) AddrOfMember(T,m)
#define MemberOff(T,m)  OffsetOfMember(T,m)

#define BasePlusMemberOff(base,s,m) ((base) + OffsetOfMemberV((s),(m)))

#define When(a,b) (!(a) || (b))

#define Min(a,b) (((a)<(b))?(a):(b))
#define Max(a,b) (((a)>(b))?(a):(b))
// TODO: HACK: hacky hacky hacky hack.........
#if !MR4TH_NO_CLAMP
# define Clamp(a,x,b) (((x)<(a))?(a):\
((b)<(x))?(b):(x))
#endif
#define ClampTop(a,b) Min(a,b)
#define ClampBot(a,b) Max(a,b)

#define Swap(T,a,b) Stmnt( T t_ = (a); (a) = (b); (b) = t_; )

#define SignedIntFromCompare(a,b) (S32)( ((b)<(a)) - ((a)<(b)) )

#define AlignUpPow2(x,p) (((x) + (p) - 1)&~((p) - 1))
#define AlignDownPow2(x,p) ((x)&~((p) - 1))
#define IsPow2OrZero(x) (((x)&((x)-1)) == 0)

#define CeilIntDiv(n,d) (((n) + (d) - 1)/(d))

#define Boolify(x) ((x) != 0)

#define KB(x) ((U64)(x) << 10)
#define MB(x) ((U64)(x) << 20)
#define GB(x) ((U64)(x) << 30)
#define TB(x) ((U64)(x) << 40llu)

#define Thousand(x) ((x)*1000)
#define Million(x)  ((x)*1000000llu)
#define Billion(x)  ((x)*1000000000llu)
#define Trillion(x) ((x)*1000000000000llu)

#define AsciiID4(a,b,c,d) (((d) << 24) | ((c) << 16) | ((b) << 8) | (a))
#define ExpandAsciiID(x) (int)(sizeof(x)), (char*)(&(x))

// memory operations

#define MemoryZero(p,z)      memory_zero((p), (z))
#define MemoryZeroStruct(p)  MemoryZero((p), sizeof(*(p)))
#define MemoryZeroArray(p)   MemoryZero((p), sizeof(p))
#define MemoryZeroTyped(p,c) MemoryZero((p), sizeof(*(p))*(c))

#define MemoryMatch(a,b,z) (memory_match((a),(b),(z)))

#define MemoryCopy(d,s,z) memory_move((d), (s), (z))
#define MemoryCopyStruct(d,s)  MemoryCopy((d),(s),Min(sizeof(*(d)),sizeof(*(s))))
#define MemoryCopyArray(d,s)   MemoryCopy((d),(s),Min(sizeof(s),sizeof(d)))
#define MemoryCopyTyped(d,s,c) MemoryCopy((d),(s),Min(sizeof(*(d)),sizeof(*(s)))*(c))


////////////////////////////////
// Macros: Linked Lists

#define DLLPushBack_NPZ(f,l,n,next,prev,nil)\
(((f) == (nil))?\
((f)=(l)=(n),(n)->next=(n)->prev=(nil)):\
((n)->prev=(l),(l)->next=(n),(l)=(n),(n)->next=(nil)))

#define DLLPushBack(f,l,n) DLLPushBack_NPZ(f,l,n,next,prev,0)
#define DLLPushFront(f,l,n) DLLPushBack_NPZ(l,f,n,prev,next,0)

#define DLLInsert_NPZ(f,l,p,n,next,prev,nil) \
(((p) != (l))?\
((n)->next = (p)->next,\
(n)->prev = (p),\
(p)->next->prev = (n),\
(p)->next = (n))\
:((n)->next = (nil),\
(n)->prev = (l),\
(l)->next = (n),\
(l) = (n)))

#define DLLInsert(f,l,p,n) DLLInsert_NPZ(f,l,p,n,next,prev,0)

#define DLLRemove_NPZ(f,l,n,next,prev,nil)\
((f)==(n)?\
((f)==(l)?\
((f)=(l)=(nil)):\
((f)=(f)->next,(f)->prev=(nil))):\
(l)==(n)?\
((l)=(l)->prev,(l)->next=(nil)):\
((n)->next->prev=(n)->prev,\
(n)->prev->next=(n)->next))

#define DLLRemove(f,l,n) DLLRemove_NPZ(f,l,n,next,prev,0)

#define SLLQueuePush_NZ(f,l,n,next,nil) (((f)==(nil)?\
(f)=(l)=(n):\
((l)->next=(n),(l)=(n))),\
(n)->next=(nil))
#define SLLQueuePush(f,l,n) SLLQueuePush_NZ(f,l,n,next,0)

#define SLLQueuePushFront_NZ(f,l,n,next,nil) ((f)==(nil)?\
((f)=(l)=(n),(n)->next=(nil)):\
((n)->next=(f),(f)=(n)))
#define SLLQueuePushFront(f,l,n) SLLQueuePushFront_NZ(f,l,n,next,0)

#define SLLQueuePop_NZ(f,l,next,nil) ((f)==(l)?\
(f)=(l)=(nil):\
((f)=(f)->next))
#define SLLQueuePop(f,l) SLLQueuePop_NZ(f,l,next,0)

#define SLLStackPush_N(f,n,next) ((n)->next=(f),(f)=(n))
#define SLLStackPush(f,n) SLLStackPush_N(f,n,next)

#define SLLStackPop_NZ(f,next,nil) ((f)==(nil)?(nil):\
((f)=(f)->next))
#define SLLStackPop(f) SLLStackPop_NZ(f,next,0)



////////////////////////////////
// Types: Basic

#include <stdint.h>
#include <stdarg.h>

typedef int8_t  S8;
typedef int16_t S16;
typedef int32_t S32;
typedef int64_t S64;
typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef S8 B8;
typedef S16 B16;
typedef S32 B32;
typedef S64 B64;
typedef float F32;
typedef double F64;

#if ARCH_ADDRSIZE == 32
typedef U32 UAddr;
typedef S32 SAddr;
#elif ARCH_ADDRSIZE == 64
typedef U64 UAddr;
typedef S64 SAddr;
#else
# error UAddr and SAddr not defined for this architecture
#endif

typedef void VoidFunc(void);

////////////////////////////////
// Types: Basic Constants

MR4TH_SYM_COMPTIME S8  min_S8  = (S8) 0x80;
MR4TH_SYM_COMPTIME S16 min_S16 = (S16)0x8000;
MR4TH_SYM_COMPTIME S32 min_S32 = (S32)0x80000000;
MR4TH_SYM_COMPTIME S64 min_S64 = (S64)0x8000000000000000llu;

MR4TH_SYM_COMPTIME S8  max_S8  = (S8) 0x7f;
MR4TH_SYM_COMPTIME S16 max_S16 = (S16)0x7fff;
MR4TH_SYM_COMPTIME S32 max_S32 = (S32)0x7fffffff;
MR4TH_SYM_COMPTIME S64 max_S64 = (S64)0x7fffffffffffffffllu;

MR4TH_SYM_COMPTIME U8  max_U8  = 0xff;
MR4TH_SYM_COMPTIME U16 max_U16 = 0xffff;
MR4TH_SYM_COMPTIME U32 max_U32 = 0xffffffff;
MR4TH_SYM_COMPTIME U64 max_U64 = 0xffffffffffffffffllu;

MR4TH_SYM_COMPTIME F32 machine_epsilon_F32 = 1.1920929e-7f;
MR4TH_SYM_COMPTIME F32 pi_F32  = 3.14159265359f;
MR4TH_SYM_COMPTIME F32 tau_F32 = 6.28318530718f;
MR4TH_SYM_COMPTIME F32 e_F32 = 2.71828182846f;
MR4TH_SYM_COMPTIME F32 gold_big_F32 = 1.61803398875f;
MR4TH_SYM_COMPTIME F32 gold_small_F32 = 0.61803398875f;

MR4TH_SYM_COMPTIME F64 machine_epsilon_F64 = 2.220446e-16;
MR4TH_SYM_COMPTIME F64 pi_F64  = 3.14159265359;
MR4TH_SYM_COMPTIME F64 tau_F64 = 6.28318530718;
MR4TH_SYM_COMPTIME F64 e_F64 = 2.71828182846;
MR4TH_SYM_COMPTIME F64 gold_big_F64 = 1.61803398875;
MR4TH_SYM_COMPTIME F64 gold_small_F64 = 0.61803398875;

////////////////////////////////
// Types: Float Constants

MR4TH_SYM_COMPTIME U32     inf_F32_as_U32 = 0x7f800000;
MR4TH_SYM_COMPTIME U32 neg_inf_F32_as_U32 = 0xff800000;
MR4TH_SYM_COMPTIME U64     inf_F64_as_U64 = 0x7ff0000000000000;
MR4TH_SYM_COMPTIME U64 neg_inf_F64_as_U64 = 0xfff0000000000000;

#define     inf_F32 (*(F32*)(&inf_F32_as_U32))
#define neg_inf_F32 (*(F32*)(&neg_inf_F32_as_U32))
#define     inf_F64 (*(F64*)(&inf_F64_as_U64))
#define neg_inf_F64 (*(F64*)(&neg_inf_F64_as_U64))

////////////////////////////////
// Types: Symbolic Constants

typedef enum Axis{
  Axis_X,
  Axis_Y,
  Axis_Z,
  Axis_W
} Axis;

typedef enum Side{
  Side_Min,
  Side_Max
} Side;

typedef enum OperatingSystem{
  OperatingSystem_Null,
  OperatingSystem_Windows,
  OperatingSystem_Linux,
  OperatingSystem_Mac,
  OperatingSystem_COUNT
} OperatingSystem;

typedef enum Architecture{
  Architecture_Null,
  Architecture_X64,
  Architecture_X86,
  Architecture_Arm,
  Architecture_Arm64,
  Architecture_COUNT
} Architecture;

typedef enum Month{
  Month_Jan,
  Month_Feb,
  Month_Mar,
  Month_Apr,
  Month_May,
  Month_Jun,
  Month_Jul,
  Month_Aug,
  Month_Sep,
  Month_Oct,
  Month_Nov,
  Month_Dec
} Month;

typedef enum DayOfWeek{
  DayOfWeek_Sunday,
  DayOfWeek_Monday,
  DayOfWeek_Tuesday,
  DayOfWeek_Wednesday,
  DayOfWeek_Thursday,
  DayOfWeek_Friday,
  DayOfWeek_Saturday
} DayOfWeek;

typedef U32 DataAccessFlags;
enum{
  DataAccessFlag_Read    = (1 << 0),
  DataAccessFlag_Write   = (1 << 1),
  DataAccessFlag_Execute = (1 << 2),
};

////////////////////////////////
// Types: Compound Types

typedef struct V2S32{
  S32 x;
  S32 y;
} V2S32;

typedef struct V2F32{
  F32 x;
  F32 y;
} V2F32;

typedef struct V3F32{
  F32 x;
  F32 y;
  F32 z;
} V3F32;

typedef struct V4F32{
  F32 x;
  F32 y;
  F32 z;
  F32 w;
} V4F32;

typedef struct RangeF32{
  F32 min;
  F32 max;
} RangeF32;

typedef struct RangeAddr{
  U8 *first;
  U8 *opl;
} RangeAddr;

typedef struct RectS32{
  S32 x0;
  S32 y0;
  S32 x1;
  S32 y1;
} RectS32;

typedef struct RectF32{
  F32 x0;
  F32 y0;
  F32 x1;
  F32 y1;
} RectF32;

// TODO(allen): fix naming consistency here
typedef struct Array_U8{
  U8 *v;
  U64 count;
} Array_U8;

typedef struct Array_U32{
  U32 *vals;
  U64 count;
} Array_U32;

typedef struct Array_F32{
  F32 *vals;
  U64 count;
} Array_F32;

typedef struct Array_V2F32{
  V2F32 *vals;
  U64 count;
} Array_V2F32;

////////////////////////////////
// Types: Time

typedef U64 DenseTime;

typedef struct DateTime{
  U16 msec; // [0,999]
  U8 sec;   // [0,60]
  U8 min;   // [0,59]
  U8 hour;  // [0,23]
  U8 day;   // [1,31]
  U8 mon;   // [1,12]
  S16 year; // 1 = 1 CE; 2020 = 2020 CE; 0 = 1 BCE; -100 = 101 BCE; etc.
} DateTime;

////////////////////////////////
// Types: File Properties

typedef U32 FilePropertyFlags;
enum{
  FilePropertyFlag_IsFolder = (1 << 0),
};

typedef struct FileProperties{
  U64 size;
  FilePropertyFlags flags;
  DenseTime create_time;
  DenseTime modify_time;
  DataAccessFlags access;
} FileProperties;


////////////////////////////////
// Types: Arena

typedef struct Arena{
  struct Arena *current;
  struct Arena *prev;
  U64 alignment;
  B8  growing;
  U8  filler[7];
  U64 base_pos;
  U64 chunk_cap;
  U64 chunk_pos;
  U64 chunk_commit_pos;
} Arena;

typedef struct ArenaTemp{
  Arena *arena;
  U64 pos;
} ArenaTemp;


////////////////////////////////
// Types: String

typedef struct String8{
  U8 *str;
  U64 size;
} String8;

typedef struct String8Node{
  struct String8Node *next;
  String8 string;
} String8Node;

typedef struct String8List{
  String8Node *first;
  String8Node *last;
  U64 node_count;
  U64 total_size;
} String8List;

typedef struct StringJoin{
  String8 pre;
  String8 mid;
  String8 post;
} StringJoin;

typedef U32 StringMatchFlags;
enum{
  StringMatchFlag_NoCase      = (1 << 0),
  StringMatchFlag_PrefixMatch = (1 << 1),
};

typedef struct String16{
  U16 *str;
  U64 size;
} String16;

typedef struct String32{
  U32 *str;
  U64 size;
} String32;

#define str8_expand(s) (int)((s).size), ((s).str)

typedef struct StringDecode{
  U32 codepoint;
  U32 size;
} StringDecode;

////////////////////////////////
// Types: Command Line Parsing

typedef struct CMDLN_Params{
  String8 raw;
  String8List list;
} CMDLN_Params;

typedef struct CMDLN_Node{
  struct CMDLN_Node *next;
  String8 string;
  CMDLN_Params *params;
  // (params ==  0 ) -> 'input' (not a flag)
  // (params !=  0 ) -> flag
  // (params == nil) -> flag has no parameters
} CMDLN_Node;

typedef CMDLN_Node CMDLN_Flag;

typedef struct CMDLN{
  String8List raw;
  String8 program;
  CMDLN_Node *first;
  CMDLN_Node *last;
  U64 input_count;
  U64 flag_count;
  CMDLN_Node **inputs;
  CMDLN_Node **flags;
} CMDLN;

////////////////////////////////
// Types: Log

typedef void LOG_LogToProc(void *uptr, String8 str);

typedef struct LOG_Node{
  struct LOG_Node *next;
  U64 pos;
  String8List log;
  LOG_LogToProc *logto;
  void *uptr;
} LOG_Node;

typedef struct LOG_ThreadVars{
  Arena    *arena;
  LOG_Node *stack;
} LOG_ThreadVars;

////////////////////////////////
// Types: Error

typedef struct ER_Node{
  struct ER_Node *next;
  U64 pos;
  String8 error;
} ER_Node;

typedef struct ER_ThreadVars{
  Arena   *arena;
  ER_Node *stack;
  U64      over_stack;
} ER_ThreadVars;

////////////////////////////////
// Intrinsics

#if INTRINSICS_MICROSOFT

# include <intrin.h>
# define intrinsic_rdtsc() __rdtsc()

#endif


////////////////////////////////
// Functions: Numerical/Math

// infinity

MR4TH_SYMBOL B32 is_inf_or_nan_F32(F32 x);
MR4TH_SYMBOL B32 is_inf_or_nan_F64(F64 x);

// float signs, rounding, and modulus

MR4TH_SYMBOL F32 abs_F32(F32 x);
MR4TH_SYMBOL F64 abs_F64(F64 x);
MR4TH_SYMBOL F32 sign_F32(F32 x);
MR4TH_SYMBOL F64 sign_F64(F64 x);

MR4TH_SYMBOL F32 trunc_F32(F32 x);
MR4TH_SYMBOL F32 floor_F32(F32 x);
MR4TH_SYMBOL F32 ceil_F32(F32 x);
MR4TH_SYMBOL F32 mod_F32(F32 x, F32 m);
MR4TH_SYMBOL F32 frac_F32(F32 x);

// transcendental functions

MR4TH_SYMBOL F32 sin_rad_F32(F32 x);
MR4TH_SYMBOL F32 cos_rad_F32(F32 x);
MR4TH_SYMBOL F32 sin_F32(F32 x);
MR4TH_SYMBOL F32 cos_F32(F32 x);
MR4TH_SYMBOL F32 tan_F32(F32 x);
MR4TH_SYMBOL F32 atan_F32(F32 x);
MR4TH_SYMBOL F32 atan2_F32(F32 x, F32 y);

MR4TH_SYMBOL F32 sqrt_F32(F32 x);
MR4TH_SYMBOL F32 ln_F32(F32 x);
MR4TH_SYMBOL F32 pow_F32(F32 base, F32 x);

MR4TH_SYMBOL F64 sin_F64(F64 x);
MR4TH_SYMBOL F64 cos_F64(F64 x);
MR4TH_SYMBOL F64 tan_F64(F64 x);
MR4TH_SYMBOL F64 atan_F64(F64 x);
MR4TH_SYMBOL F64 atan2_F64(F64 x, F64 y);

MR4TH_SYMBOL F64 sqrt_F64(F64 x);
MR4TH_SYMBOL F64 ln_F64(F64 x);
MR4TH_SYMBOL F64 pow_F64(F64 base, F64 x);

// linear interpolation

MR4TH_SYMBOL F32 lerp(F32 a, F32 t, F32 b);
MR4TH_SYMBOL F32 unlerp(F32 a, F32 x, F32 b);

MR4TH_SYMBOL F32 lerp_range(RangeF32 range, F32 t);

// integer rounding & truncating

MR4TH_SYMBOL U32 next_pow2_U32(U32 x);
MR4TH_SYMBOL U64 next_pow2_U64(U32 x);
MR4TH_SYMBOL S64 sign_extend_S64(S64 x, U32 bitidx);

// gaussian

MR4TH_SYMBOL F32       math_gaussian(F32 sigma, F32 x);
MR4TH_SYMBOL Array_F32 math_gaussian_kernel(Arena *arena, F32 sigma,
                                            U32 extra_reach);


////////////////////////////////
// Functions: Compound Types

// compound type constructors

MR4TH_SYMBOL V2S32 v2s32(S32 x, S32 y);

MR4TH_SYMBOL V2F32 v2f32(F32 x, F32 y);
MR4TH_SYMBOL V3F32 v3f32(F32 x, F32 y, F32 z);
MR4TH_SYMBOL V4F32 v4f32(F32 x, F32 y, F32 z, F32 w);

MR4TH_SYMBOL RangeF32 rangef32(F32 min, F32 max);

MR4TH_SYMBOL RectS32 rects32(S32 x0, S32 y0, S32 x1, S32 y1);

MR4TH_SYMBOL RectF32 rectf32(F32 x0, F32 y0, F32 x1, F32 y1);
MR4TH_SYMBOL RectF32 rectf32_corners(V2F32 p0, V2F32 p1);
MR4TH_SYMBOL RectF32 rectf32_ranges(RangeF32 x, RangeF32 y);

#define v2_expanded(v) ((v).x), ((v).y)
#define v3_expanded(v) ((v).x), ((v).y), ((v).z)
#define v4_expanded(v) ((v).x), ((v).y), ((v).z), ((v).w)
#define rect_expanded(r) ((r).x0), ((r).y0), ((r).x1), ((r).y1)

// 2d vectors

MR4TH_SYMBOL V2F32 v2f32_add(V2F32 a, V2F32 b);
MR4TH_SYMBOL V2F32 v2f32_sub(V2F32 a, V2F32 b);
MR4TH_SYMBOL V2F32 v2f32_scalar_mul(F32 s, V2F32 v);

MR4TH_SYMBOL V2F32 v2f32_lerp(V2F32 a, F32 t, V2F32 b);

MR4TH_SYMBOL V2S32 v2s32_add(V2S32 a, V2S32 b);
MR4TH_SYMBOL V2S32 v2s32_sub(V2S32 a, V2S32 b);

MR4TH_SYMBOL V2F32 v2f32_spherical(F32 theta_xz, F32 theta_yz, F32 radius);
MR4TH_SYMBOL F32   angle_from_v2f32(V2F32 v);
MR4TH_SYMBOL F32   length_from_v2f32(V2F32 v);
MR4TH_SYMBOL V2F32 v2f32_unit(V2F32 v);

// 3d vectors

MR4TH_SYMBOL V3F32 v3f32_add(V3F32 a, V3F32 b);
MR4TH_SYMBOL V3F32 v3f32_sub(V3F32 a, V3F32 b);
MR4TH_SYMBOL V3F32 v3f32_scalar_mul(F32 s, V3F32 v);

MR4TH_SYMBOL V3F32 v3f32_polar(F32 theta_xz, F32 theta_yz, F32 radius);
MR4TH_SYMBOL F32   length_from_v3f32(V3F32 v);
MR4TH_SYMBOL V3F32 v3f32_unit(V3F32 v);

MR4TH_SYMBOL V3F32 v3f32_cross(V3F32 a, V3F32 b);

// 4x4 matrix

MR4TH_SYMBOL B32  mat4x4_inv(F32 *in, F32 *out);
MR4TH_SYMBOL void mat4x4_mul(F32 *a, F32 *b, F32 *out);

// rectangle geometry

MR4TH_SYMBOL B32 rectf32_contains_v2f32(RectF32 rect, V2F32 p);
MR4TH_SYMBOL B32 rectf32_overlaps_rectf32(RectF32 a, RectF32 b);

MR4TH_SYMBOL B32 rects32_contains_v2s32(RectS32 rect, V2S32 p);

MR4TH_SYMBOL V2S32 dim_from_rects32(RectS32 r);
MR4TH_SYMBOL V2F32 dim_from_rectf32(RectF32 r);

MR4TH_SYMBOL RectF32 rectf32_in_rectf32_clamp(RectF32 container, V2F32 p0, V2F32 dim);
MR4TH_SYMBOL RectF32 rectf32_in_rectf32_keep_aspect_ratio(RectF32 container, V2F32 dim);

MR4TH_SYMBOL RectF32 rectf32_intersect(RectF32 a, RectF32 b);

////////////////////////////////
// Functions: Memory Operations

MR4TH_SYMBOL void memory_zero(void *ptr, U64 size);
MR4TH_SYMBOL void memory_fill(void *ptr, U64 size, U8 fillbyte);
MR4TH_SYMBOL B32  memory_match(void *a, void *b, U64 size);
MR4TH_SYMBOL void*memory_move(void *a, void *b, U64 size);

////////////////////////////////
// Functions: Strings

// characters

MR4TH_SYMBOL B32 str8_char_is_whitespace(U8 c);
MR4TH_SYMBOL B32 str8_char_is_slash(U8 c);
MR4TH_SYMBOL B32 str8_char_is_digit(U8 c);

MR4TH_SYMBOL U8 str8_char_uppercase(U8 c);
MR4TH_SYMBOL U8 str8_char_lowercase(U8 c);

// in-place constructors

MR4TH_SYMBOL String8 str8(U8 *str, U64 size);
MR4TH_SYMBOL String8 str8_range(U8 *first, U8 *opl);
MR4TH_SYMBOL String8 str8_cstring(U8 *cstr);
MR4TH_SYMBOL String8 str8_cstring_capped(U8 *cstr, U8 *opl);

#define str8_lit(s) str8((U8*)(s), sizeof(s) - 1)
#define str8_comptime_lit(s) (String8){(U8*)(s), sizeof(s)-1}
#define str8_struct(s) str8((U8*)(s), sizeof(*(s)))
#define str8_array(s) str8((U8*)(s), sizeof(s))
#define str8_lit_const(s) { (U8*)(s), sizeof(s) - 1 }

MR4TH_SYMBOL String8 str8_prefix(String8 str, U64 size);
MR4TH_SYMBOL String8 str8_chop(String8 str, U64 amount);
MR4TH_SYMBOL String8 str8_postfix(String8 str, U64 size);
MR4TH_SYMBOL String8 str8_skip(String8 str, U64 amount);
MR4TH_SYMBOL String8 str8_substr_opl(String8 str, U64 first, U64 opl);
MR4TH_SYMBOL String8 str8_substr_size(String8 str, U64 first, U64 size);

MR4TH_SYMBOL String8 str8_skip_chop_whitespace(String8 str);

MR4TH_SYMBOL String16 str16(U16 *str, U64 size);
MR4TH_SYMBOL String16 str16_cstring(U16 *cstr);

MR4TH_SYMBOL String32 str32(U32 *str, U64 size);

// hash

MR4TH_SYMBOL U64 str8_hash(String8 str);

// path helpers

MR4TH_SYMBOL String8 str8_chop_last_slash(String8 string);

////////////////////////////////
// Functions: Serializer/Deserializer Helpers

MR4TH_SYMBOL B32 dser_read(String8 data, U64 off, void *dst, U64 size);
#define dser_read_typed(d,o,p) dser_read((d), (o), p, sizeof(*(p)))


////////////////////////////////
// Functions: Symbolic Constants

MR4TH_SYMBOL OperatingSystem operating_system_from_context(void);
MR4TH_SYMBOL Architecture architecture_from_context(void);

MR4TH_SYMBOL char* string_from_operating_system(OperatingSystem os);
MR4TH_SYMBOL char* string_from_architecture(Architecture arch);
MR4TH_SYMBOL char* string_from_month(Month month);
MR4TH_SYMBOL char* string_from_day_of_week(DayOfWeek day_of_week);

////////////////////////////////
// Functions: Time

MR4TH_SYMBOL DenseTime dense_time_from_date_time(DateTime *date_time);
MR4TH_SYMBOL DateTime  date_time_from_dense_time(DenseTime dense_time);

////////////////////////////////
// Functions: Arenas

// arena core

MR4TH_SYMBOL Arena* arena_alloc_reserve(U64 reserve_size, B32 growing);
MR4TH_SYMBOL Arena* arena_alloc(void);
MR4TH_SYMBOL void   arena_release(Arena *arena);
MR4TH_SYMBOL void*  arena_push_no_zero(Arena *arena, U64 size);
MR4TH_SYMBOL void   arena_pop_to(Arena *arena, U64 pos);
MR4TH_SYMBOL U64    arena_current_pos(Arena *arena);

MR4TH_SYMBOL void*  arena_push(Arena *arena, U64 size);
MR4TH_SYMBOL void   arena_align(Arena *arena, U64 pow2_align);
MR4TH_SYMBOL void   arena_pop_amount(Arena *arena, U64 amount);

#define push_array(a,T,c) (T*)arena_push((a), sizeof(T)*(c))
#define push_array_no_zero(a,T,c) (T*)arena_push_no_zero((a), sizeof(T)*(c))
#define push_array_copy(a,T,s,c) \
(T*)memory_move(push_array_no_zero(a,T,c), (s), sizeof(T)*(c))

// arena temp

MR4TH_SYMBOL ArenaTemp arena_begin_temp(Arena *arena);
MR4TH_SYMBOL void      arena_end_temp(ArenaTemp *temp);

// scratch

MR4TH_SHARED_FUNC(arena_get_scratch, ArenaTemp, (Arena **conflict_array, U32 count));
#define arena_release_scratch(temp) arena_end_temp(temp)

////////////////////////////////
// Functions: Strings

// compound constructors

MR4TH_SYMBOL void str8_list_push_explicit(String8List *list, String8 string,
                                          String8Node *node_memory);
MR4TH_SYMBOL void str8_list_push(Arena *arena, String8List *list, String8 string);

MR4TH_SYMBOL void str8_list_push_front_explicit(String8List *list, String8 string, String8Node *node_memory);
MR4TH_SYMBOL void str8_list_push_front(Arena *arena, String8List *list, String8 string);

MR4TH_SYMBOL String8List str8_list_copy(Arena *arena, String8List *list);

MR4TH_SYMBOL String8 str8_join(Arena *arena, String8List *list,
                               StringJoin *optional_join);

MR4TH_SYMBOL String8List str8_split(Arena *arena, String8 string,
                                    U8 *split_characters, U32 count);

MR4TH_SYMBOL String8 str8_pushfv(Arena *arena, char *fmt, va_list args);
MR4TH_SYMBOL String8 str8_pushf(Arena *arena, char *fmt, ...);
MR4TH_SYMBOL void    str8_list_pushf(Arena *arena, String8List *list, char *fmt, ...);

MR4TH_SYMBOL String8 str8_push_copy(Arena *arena, String8 string);

// operators

MR4TH_SYMBOL B32 str8_match(String8 a, String8 b, StringMatchFlags flags);

// unicode

MR4TH_SYMBOL StringDecode str_decode_utf8(U8 *str, U32 cap);
MR4TH_SYMBOL U32          str_encode_utf8(U8 *dst, U32 codepoint);
MR4TH_SYMBOL StringDecode str_decode_utf16(U16 *str, U32 cap);
MR4TH_SYMBOL U32          str_encode_utf16(U16 *dst, U32 codepoint);

MR4TH_SYMBOL String32     str32_from_str8(Arena *arena, String8 string);
MR4TH_SYMBOL String8      str8_from_str32(Arena *arena, String32 string);
MR4TH_SYMBOL String16     str16_from_str8(Arena *arena, String8 string);
MR4TH_SYMBOL String8      str8_from_str16(Arena *arena, String16 string);

// common dumping helpers

MR4TH_SYMBOL String8 str8_join_flags(Arena *arena, String8List *list);

// numeric conversion

MR4TH_SYMBOL B32 str8_is_u64(String8 string, U32 radix);

MR4TH_SYMBOL U64 u64_from_str8(String8 string, U32 radix);
MR4TH_SYMBOL U64 u64_from_str8_c_syntax(String8 string);
MR4TH_SYMBOL S64 s64_from_str8_c_syntax(String8 string);
MR4TH_SYMBOL F64 f64_from_str8(String8 string);

// path helpers

MR4TH_SYMBOL String8 str8_file_name_from_path(String8 full_file_name);
MR4TH_SYMBOL String8 str8_base_name_from_file_name(String8 file_name);

////////////////////////////////
// Functions: Command Line Parsing

MR4TH_SYMBOL CMDLN*        cmdln_from_args(Arena *arena, String8List *args);
MR4TH_SYMBOL CMDLN_Params* cmdln_params_from_string(Arena *arena, String8 flag_param);

MR4TH_SYMBOL U64           cmdln_input_count(CMDLN *cmdln);
MR4TH_SYMBOL String8       cmdln_input_from_idx(CMDLN *cmdln, U64 idx);

MR4TH_SYMBOL U64           cmdln_flag_count(CMDLN *cmdln);
MR4TH_SYMBOL CMDLN_Flag*   cmdln_flag_from_idx(CMDLN *cmdln, U64 idx);

MR4TH_SYMBOL CMDLN_Params* cmdln_get_params(CMDLN *cmdln, String8 flagstr, char abbrev);
MR4TH_SYMBOL B32           cmdln_has_flag(CMDLN *cmdln, String8 flagstr, char abbrev);
MR4TH_SYMBOL String8       cmdln_get_str8(CMDLN *cmdln, String8 flagstr, char abbrev);
MR4TH_SYMBOL S64           cmdln_get_s64(CMDLN *cmdln, String8 flagstr, char abbrev);
MR4TH_SYMBOL F64           cmdln_get_f64(CMDLN *cmdln, String8 flagstr, char abbrev);

MR4TH_SYMBOL S64           cmdln_s64_from_str8(String8 valstr);
MR4TH_SYMBOL F64           cmdln_f64_from_str8(String8 valstr);

MR4TH_SYMBOL void cmdln_dump(Arena *arena, String8List *out, CMDLN *cmdln, U32 indent);

////////////////////////////////
// Functions: Sort

typedef S32 SORT_Compare(void *a, void *b, void *udata);
MR4TH_SYMBOL void sort_merge(void *data, U32 itemsize, U32 count,
                             SORT_Compare *compare, void *udata);

////////////////////////////////
// Functions: Log

MR4TH_SHARED_FUNC(log_accum_begin, void, (LOG_LogToProc *proc, void *uptr));
MR4TH_SHARED_FUNC(log_gathering, B32, (void));
MR4TH_SHARED_FUNC(log_emit, void, (String8 message));
MR4TH_SHARED_FUNC(log_emitf, void, (char *fmt, ...));
MR4TH_SHARED_FUNC(log_accum_end, String8, (Arena *arena));

////////////////////////////////
// Functions: Errors

MR4TH_SHARED_FUNC(er_accum_begin, void, (void));
MR4TH_SHARED_FUNC(er_emit, void, (String8 error));
MR4TH_SHARED_FUNC(er_emitf, void, (char *fmt, ...));
MR4TH_SHARED_FUNC(er_accum_end, String8, (Arena *arena));



////////////////////////////////////////////////
////////////////////////////////////////////////
/////////   DECLARATIONS PROFILING   ///////////
////////////////////////////////////////////////
////////////////////////////////////////////////

////////////////////////////////
// Profiling: Implementable Interface

#if MR4TH_PROFILING_USER || MR4TH_PROFILING_PROVIDER

MR4TH_SYM_LINKTIME void prof_open(char *name);
MR4TH_SYM_LINKTIME void prof_close(void);
MR4TH_SYM_LINKTIME void prof_thread_begin(void);
MR4TH_SYM_LINKTIME void prof_thread_end(void);
MR4TH_SYM_LINKTIME void prof_thread_flush(void);
MR4TH_SYM_LINKTIME void prof_begin(char *name, U32 len);
MR4TH_SYM_LINKTIME void prof_end(void);

#endif

////////////////////////////////
// Profiling: User Interface

#if MR4TH_PROFILING_USER
# define ProfOpen(n)       prof_open((char*)(n))
# define ProfClose()       prof_close()
# define ProfThreadBegin() prof_thread_begin()
# define ProfThreadEnd()   prof_thread_end()
# define ProfThreadFlush() prof_thread_flush()
# define ProfBegin(n)      prof_begin((n), sizeof(n) - 1)
# define ProfEnd()         prof_end()
#else
# define ProfOpen(n)
# define ProfClose()
# define ProfThreadBegin()
# define ProfThreadEnd()
# define ProfThreadFlush()
# define ProfBegin(n)
# define ProfEnd()
#endif

#if MR4TH_PROFILING_MANUAL
# define ProfBeginManual(n) ProfBegin(n)
# define ProfEndManual()    ProfEnd()
#else
# define ProfBeginManual(n)
# define ProfEndManual()
#endif

#define ProfBeginFunc() ProfBeginManual(__FUNCTION__)
#define ProfEndFunc()   ProfEndManual()



////////////////////////////////////////////////
////////////////////////////////////////////////
///////   DECLARATIONS OS ABSTRACTION   ////////
////////////////////////////////////////////////
////////////////////////////////////////////////

////////////////////////////////
// Memory Configuration

#define MR4TH_MEM_DEFAULT_RESERVE_SIZE GB(1)
#define MR4TH_MEM_COMMIT_BLOCK_SIZE    MB(64)
#define MR4TH_MEM_MAX_ALIGN            64
#define MR4TH_MEM_SCRATCH_POOL_COUNT   2

////////////////////////////////
// Shared Types

typedef struct OS_FileIter{
  U8 v[640];
} OS_FileIter;

typedef enum OS_SystemPath{
  OS_SystemPath_CurrentDirectory,
  OS_SystemPath_Binary,
  OS_SystemPath_UserData,
  OS_SystemPath_TempData,
  OS_SystemPath_COUNT
} OS_SystemPath;

typedef void OS_Library;

////////////////////////////////
// Implementable Functions: Process Setup

MR4TH_SYMBOL void        os_main_init(int argc, char **argv);
MR4TH_SYMBOL String8List os_command_line_arguments(void);
MR4TH_SYMBOL void        os_exit_process(U32 code);

////////////////////////////////
// Implementable Functions: Memory Functions

// TODO(allen): memory protection options?
MR4TH_SYMBOL void* os_memory_reserve(U64 size);
MR4TH_SYMBOL B32   os_memory_commit(void *ptr, U64 size);
MR4TH_SYMBOL void  os_memory_decommit(void *ptr, U64 size);
MR4TH_SYMBOL void  os_memory_release(void *ptr, U64 size);

////////////////////////////////
// Implementable Functions: File Handling

MR4TH_SYMBOL String8 os_file_read(Arena *arena, String8 file_name);
MR4TH_SYMBOL B32     os_file_write_list(String8 file_name,
                                        String8Node *first_node);

MR4TH_SYMBOL FileProperties os_file_properties(String8 file_name);

MR4TH_SYMBOL B32 os_file_delete(String8 file_name);
MR4TH_SYMBOL B32 os_file_rename(String8 og_name, String8 new_name);
MR4TH_SYMBOL B32 os_file_make_directory(String8 path);
MR4TH_SYMBOL B32 os_file_delete_directory(String8 path);

MR4TH_SYMBOL OS_FileIter os_file_iter_init(String8 path);
MR4TH_SYMBOL B32  os_file_iter_next(Arena *arena, OS_FileIter *iter,
                                    String8 *name_out, FileProperties *prop_out);
MR4TH_SYMBOL void os_file_iter_end(OS_FileIter *iter);

MR4TH_SYMBOL String8 os_file_path(Arena *arena, OS_SystemPath path);

MR4TH_SYMBOL String8 os_get_absolute_path_from_str8_list(Arena *arena, String8List path_parts);
MR4TH_SYMBOL String8 os_get_absolute_path(Arena *arena, String8 path);

MR4TH_SYMBOL void    os_set_current_directory(String8 path);

////////////////////////////////
// Implementable Functions: Time

MR4TH_SYMBOL DateTime os_now_universal_time(void);
MR4TH_SYMBOL DateTime os_local_time_from_universal(DateTime *date_time);
MR4TH_SYMBOL DateTime os_universal_time_from_local(DateTime *date_time);

MR4TH_SYMBOL U32  os_time_stamp_32_from_date_time(DateTime *date_time);

MR4TH_SYMBOL U64  os_now_ticks(void);
MR4TH_SYMBOL void os_microseconds_over_ticks(U64 *usecs, U64 *ticks);
MR4TH_SYMBOL void os_sleep_milliseconds(U32 t);

////////////////////////////////
// Implementable Functions: Libraries

MR4TH_SYMBOL OS_Library* os_lib_load(String8 path);
MR4TH_SYMBOL VoidFunc*   os_lib_get_proc(OS_Library *lib, char *name);
MR4TH_SYMBOL void        os_lib_release(OS_Library *lib);

#define OS_PROC_ADDR(v,l,n) (*(VoidFunc**)(&(v))) = os_lib_get_proc((l),(n))

MR4TH_SYMBOL OS_Library* os_lib_from_addr(void *addr);
MR4TH_SYMBOL RangeAddr   os_lib_image_range(OS_Library *lib);
MR4TH_SYMBOL String8     os_this_image(void);

////////////////////////////////
// Implementable Functions: Entropy

MR4TH_SYMBOL void os_get_entropy(void *data, U64 size);

////////////////////////////////
// Implementable Functions: Clipboard

MR4TH_SYMBOL void os_clipboard_set_str8(String8 text);

////////////////////////////////
// Helper Functions: File Handling

MR4TH_SYMBOL B32 os_file_write(String8 file_name, String8 data);



////////////////////////////////////////////////
////////////////////////////////////////////////
///////////   DECLARATIONS WIN32   /////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

// TODO: HACK: MR4TH_NO_INCLUDES is a hack so that you can build/link to mr4th_base and avoid naming conflicts with OS names. There's probably a better way...
#if OS_WINDOWS && !MR4TH_NO_INCLUDES

////////////////////////////////
// Win32: Includes

#include <Windows.h>
#include <userenv.h>
#include <psapi.h>

////////////////////////////////
// Win32 Types

typedef struct W32_FileIter{
  HANDLE handle;
  WIN32_FIND_DATAW find_data;
  B32 done;
} W32_FileIter;
StaticAssert(sizeof(W32_FileIter) <= sizeof(OS_FileIter), w32_fileiter);

////////////////////////////////
// Win32 Functions: Specialized Init for WinMain

MR4TH_SYMBOL void w32_WinMain_init(HINSTANCE hInstance,
                                   HINSTANCE hPrevInstance,
                                   LPSTR     lpCmdLine,
                                   int       nShowCmd);

MR4TH_SYMBOL HINSTANCE w32_get_instance(void);

////////////////////////////////
// Win32 Functions: Time Helpers

MR4TH_SYMBOL DateTime   w32_date_time_from_system_time(SYSTEMTIME *in);
MR4TH_SYMBOL SYSTEMTIME w32_system_time_from_date_time(DateTime *in);
MR4TH_SYMBOL DenseTime  w32_dense_time_from_file_time(FILETIME *file_time);

////////////////////////////////
// Win32 Functions: File Helpers

MR4TH_SYMBOL FilePropertyFlags w32_prop_flags_from_attribs(DWORD attribs);
MR4TH_SYMBOL DataAccessFlags   w32_access_from_attributes(DWORD attribs);

////////////////////////////////
// Win32 Helper Macro

#define W32_PROC_ADDR(v,m,s) (*(PROC*)(&(v))) = GetProcAddress((m),(s))

#endif /* OS_WINDOWS */


////////////////////////////////////////////////
////////////////////////////////////////////////
///////////   DECLARATIONS LINUX   /////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

#if OS_LINUX && !MR4TH_NO_INCLUDES

#include <clfcn.h>

#endif /* OS_LINUX */


//////////////////////////////////////////////
//////////////////////////////////////////////
///////////   DECLARATIONS MAC   /////////////
//////////////////////////////////////////////
//////////////////////////////////////////////

#if OS_MAC && !MR4TH_NO_INCLUDES

#include <stdlib.h> // realpath
#include <fcntl.h> // open
#include <dlfcn.h> // dlopen, dlsym, etc.
// TODO: We no longer use "getsectbyname" because we finally introspect mach-o files! I guess we can delete the include and be happy?
#include <mach-o/getsect.h> // getsectbyname
#include <sys/mman.h> // mmap, munmap, madvise
#include <dirent.h> // opendir, readdir, closedir
#include <sys/stat.h> // lstat, stat struct
#include <unistd.h> // write
#include <sys/syslimits.h> // PATH_MAX

typedef struct Mac_FileIter{
  String8 base_path;
  DIR *handle;
  B32 done;
} Mac_FileIter;
StaticAssert(sizeof(Mac_FileIter) <= sizeof(OS_FileIter), mac_fileiter);


#endif /* OS_MAC */


////////////////////////////////////////////////
////////////////////////////////////////////////
///   stb_sprintf.h STB_SPRINTF_H_INCLUDE   ////
////////////////////////////////////////////////
////////////////////////////////////////////////

#define STB_SPRINTF_DECORATE(name) m4_##name

/*
Single file sprintf replacement.

Originally written by Jeff Roberts at RAD Game Tools - 2015/10/20.
Hereby placed in public domain.

This is a full sprintf replacement that supports everything that
the C runtime sprintfs support, including float/double, 64-bit integers,
hex floats, field parameters (%*.*d stuff), length reads backs, etc.

Why would you need this if sprintf already exists?  Well, first off,
it's *much* faster (see below). It's also much smaller than the CRT
versions code-space-wise. We've also added some simple improvements
that are super handy (commas in thousands, callbacks at buffer full,
for example). Finally, the format strings for MSVC and GCC differ
for 64-bit integers (among other small things), so this lets you use
the same format strings in cross platform code.

It uses the standard single file trick of being both the header file
and the source itself. If you just include it normally, you just get
the header file function definitions. To get the code, you include
it from a C or C++ file and define STB_SPRINTF_IMPLEMENTATION first.

It only uses va_args macros from the C runtime to do it's work. It
does cast doubles to S64s and shifts and divides U64s, which does
drag in CRT code on most platforms.

It compiles to roughly 8K with float support, and 4K without.
As a comparison, when using MSVC static libs, calling sprintf drags
in 16K.

API:
====
int stbsp_sprintf( char * buf, char const * fmt, ... )
int stbsp_snprintf( char * buf, int count, char const * fmt, ... )
  Convert an arg list into a buffer.  stbsp_snprintf always returns
  a zero-terminated string (unlike regular snprintf).

int stbsp_vsprintf( char * buf, char const * fmt, va_list va )
int stbsp_vsnprintf( char * buf, int count, char const * fmt, va_list va )
  Convert a va_list arg list into a buffer.  stbsp_vsnprintf always returns
  a zero-terminated string (unlike regular snprintf).

int stbsp_vsprintfcb( STBSP_SPRINTFCB * callback, void * user, char * buf, char const * fmt, va_list va )
    typedef char * STBSP_SPRINTFCB( char const * buf, void * user, int len );
  Convert into a buffer, calling back every STB_SPRINTF_MIN chars.
  Your callback can then copy the chars out, print them or whatever.
  This function is actually the workhorse for everything else.
  The buffer you pass in must hold at least STB_SPRINTF_MIN characters.
    // you return the next buffer to use or 0 to stop converting

void stbsp_set_separators( char comma, char period )
  Set the comma and period characters to use.

FLOATS/DOUBLES:
===============
This code uses a internal float->ascii conversion method that uses
doubles with error correction (double-doubles, for ~105 bits of
precision).  This conversion is round-trip perfect - that is, an atof
of the values output here will give you the bit-exact double back.

One difference is that our insignificant digits will be different than
with MSVC or GCC (but they don't match each other either).  We also
don't attempt to find the minimum length matching float (pre-MSVC15
doesn't either).

If you don't need float or doubles at all, define STB_SPRINTF_NOFLOAT
and you'll save 4K of code space.
NOTE(allen): MODIFICATION - I've hard coded that this copy *does* use floats

64-BIT INTS:
============
This library also supports 64-bit integers and you can use MSVC style or
GCC style indicators (%I64d or %lld).  It supports the C99 specifiers
for size_t and ptr_diff_t (%jd %zd) as well.

EXTRAS:
=======
Like some GCCs, for integers and floats, you can use a ' (single quote)
specifier and commas will be inserted on the thousands: "%'d" on 12345
would print 12,345.

For integers and floats, you can use a "$" specifier and the number
will be converted to float and then divided to get kilo, mega, giga or
tera and then printed, so "%$d" 1000 is "1.0 k", "%$.2d" 2536000 is
"2.53 M", etc. For byte values, use two $:s, like "%$$d" to turn
2536000 to "2.42 Mi". If you prefer JEDEC suffixes to SI ones, use three
$:s: "%$$$d" -> "2.42 M". To remove the space between the number and the
suffix, add "_" specifier: "%_$d" -> "2.53M".

In addition to octal and hexadecimal conversions, you can print
integers in binary: "%b" for 256 would print 100.

PERFORMANCE vs MSVC 2008 32-/64-bit (GCC is even slower than MSVC):
===================================================================
"%d" across all 32-bit ints (4.8x/4.0x faster than 32-/64-bit MSVC)
"%24d" across all 32-bit ints (4.5x/4.2x faster)
"%x" across all 32-bit ints (4.5x/3.8x faster)
"%08x" across all 32-bit ints (4.3x/3.8x faster)
"%f" across e-10 to e+10 floats (7.3x/6.0x faster)
"%e" across e-10 to e+10 floats (8.1x/6.0x faster)
"%g" across e-10 to e+10 floats (10.0x/7.1x faster)
"%f" for values near e-300 (7.9x/6.5x faster)
"%f" for values near e+300 (10.0x/9.1x faster)
"%e" for values near e-300 (10.1x/7.0x faster)
"%e" for values near e+300 (9.2x/6.0x faster)
"%.320f" for values near e-300 (12.6x/11.2x faster)
"%a" for random values (8.6x/4.3x faster)
"%I64d" for 64-bits with 32-bit values (4.8x/3.4x faster)
"%I64d" for 64-bits > 32-bit values (4.9x/5.5x faster)
"%s%s%s" for 64 char strings (7.1x/7.3x faster)
"...512 char string..." ( 35.0x/32.5x faster!)
*/

#if defined(__clang__)
#if defined(__has_feature) && defined(__has_attribute)
#if __has_feature(address_sanitizer)
#if __has_attribute(__no_sanitize__)
#define STBSP__ASAN __attribute__((__no_sanitize__("address")))
#elif __has_attribute(__no_sanitize_address__)
#define STBSP__ASAN __attribute__((__no_sanitize_address__))
#elif __has_attribute(__no_address_safety_analysis__)
#define STBSP__ASAN __attribute__((__no_address_safety_analysis__))
#endif
#endif
#endif
#elif defined(__GNUC__) && (__GNUC__ >= 5 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8))
#if defined(__SANITIZE_ADDRESS__) && __SANITIZE_ADDRESS__
#define STBSP__ASAN __attribute__((__no_sanitize_address__))
#endif
#endif

#ifndef STBSP__ASAN
#define STBSP__ASAN
#endif

#ifdef STB_SPRINTF_STATIC
#define STBSP__PUBLICDEC static
#define STBSP__PUBLICDEF static STBSP__ASAN
#else
#ifdef __cplusplus
#define STBSP__PUBLICDEC extern "C"
#define STBSP__PUBLICDEF extern "C" STBSP__ASAN
#else
#define STBSP__PUBLICDEC extern
#define STBSP__PUBLICDEF STBSP__ASAN
#endif
#endif

#if defined(__has_attribute)
#if __has_attribute(format)
#define STBSP__ATTRIBUTE_FORMAT(fmt,va) __attribute__((format(printf,fmt,va)))
#endif
#endif

#ifndef STBSP__ATTRIBUTE_FORMAT
#define STBSP__ATTRIBUTE_FORMAT(fmt,va)
#endif

#ifdef _MSC_VER
#define STBSP__NOTUSED(v)  (void)(v)
#else
#define STBSP__NOTUSED(v)  (void)sizeof(v)
#endif

#include <stdarg.h> // for va_arg(), va_list()
#include <stddef.h> // size_t, ptrdiff_t

#ifndef STB_SPRINTF_MIN
#define STB_SPRINTF_MIN 512 // how many characters per callback
#endif
typedef char *STBSP_SPRINTFCB(const char *buf, void *user, int len);

#ifndef STB_SPRINTF_DECORATE
#define STB_SPRINTF_DECORATE(name) stbsp_##name // define this before including if you want to change the names
#endif

STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(vsprintf)(char *buf, char const *fmt, va_list va);
STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(vsnprintf)(char *buf, int count, char const *fmt, va_list va);
STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(sprintf)(char *buf, char const *fmt, ...) STBSP__ATTRIBUTE_FORMAT(2,3);
STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(snprintf)(char *buf, int count, char const *fmt, ...) STBSP__ATTRIBUTE_FORMAT(3,4);

STBSP__PUBLICDEC int STB_SPRINTF_DECORATE(vsprintfcb)(STBSP_SPRINTFCB *callback, void *user, char *buf, char const *fmt, va_list va);
STBSP__PUBLICDEC void STB_SPRINTF_DECORATE(set_separators)(char comma, char period);

#endif /* MR4TH_BASE_DECLARATIONS */

/*
** TODO:
**
** [ ] CMDLN System
**     [x] basic implementation
**     [ ] fuzz
**     [ ] built in parser diagnostics
**     [ ] built in misuse feedback
**     [ ] help structurer
**
** [x] Come up with better names than 'small' and 'big' (just eliminated them)
** [x] Merge 'base' and 'os'
** [ ] Detect mr4th_base.dll version mismatch
**     [x] remove static configuration operations
** [ ] OS specific fatal error messages in base_before_main
** [x] Build the runtime linking system
**     [x] Log [x] Error [x] Scratch Memory
**     [x] Mechanism for Ensuring Maintenance Upkeep
** [x] stb snprintf
**     [x] add in stb sprintf directly
**     [x] option to control whether is present stdio.h
**     [x] switch all my code off of default *printf
**     [x] custom sprintf modifications
**         [x] String8
**         [x] indentation
**     [ ] put snprintf in the run-time linking system? reduce total binary sizes?
** [ ] unicode conversions should take string lists "Better unicode conversions here"
** [ ] math implementation replacements
** [ ] clib elimination option
**     [x] manually implement memset,memcmp,memmove
** [ ] Test out the usage process
**     [ ] new program
**     [ ] dynamic bridge (DLL)
**     [ ] profiling
** [ ] Write instructions
** [ ] Write examples
*/

/*
** NOTE(allen): sprintf modifications notes
**
** Legend for specifier allocation:
**  [ ] - specifier is free for use
**  [#] - used by stb_sprintf default
**  [$] - used by custom printing logic
**
**  Specifier allocation:
**   a[#] - hex float
**   A[#] - hex float
**   b[#] - binary
**   B[#] - binary
**   c[#] - char
**   C[ ]
**   d[#] - integer
**   D[ ]
**   e[#] - float
**   E[#] - float
**   f[#] - float
**   F[ ]
**   g[#] - float
**   G[#] - float
**   h[#] - half width
**   H[ ]
**   i[#] - integer
**   I[#] - size[Microsoft-Style]
**   j[#] - size[size_t]
**   J[ ]
**   k[ ]
**   K[ ]
**   l[#] - size[long]/size[long long]
**   L[ ]
**   m[ ]
**   M[ ]
**   n[#] - write-bytes
**   N[$] - indentation
**   o[#] - octal
**   O[ ]
**   p[ ]
**   P[ ]
**   q[ ]
**   Q[ ]
**   r[ ]
**   R[ ]
**   s[#] - cstr
**   S[$] - String8
**   t[#] - size[ptrdiff_t]
**   T[ ]
**   u[#] - unsigned integer
**   U[ ]
**   v[ ]
**   V[ ]
**   w[ ]
**   W[ ]
**   x[#] - hex
**   X[#] - hex
**   y[ ]
**   Y[ ]
**   z[#] - size[size_t]
**   Z[ ]
**   0[#] - leading zeroes
**   1[ ]
**   2[ ]
**   3[ ]
**   4[ ]
**   5[ ]
**   6[ ]
**   7[ ]
**   8[ ]
**   9[ ]
**   $[$] - memory size (modified from stb_sprintf.h)
**   ?[ ]
**
*/

