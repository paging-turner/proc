#ifndef MACHO_INCLUDE_H
# define MACHO_INCLUDE_H

/*
  [ ] Copy over data-structures from mach-o h-files  (this should help in creating a document that is meant to help somebody learn about the mach-o format.
*/

#ifndef MACHO_TEST_AGAINST_MAC
# define MACHO_TEST_AGAINST_MAC 1
#endif

#if MACHO_TEST_AGAINST_MAC
# include <mach-o/ldsyms.h> // for LC_* values
# include <mach-o/loader.h> // header and load-commands
# include <mach-o/nlist.h> // symbol-table
# include <mach-o/stab.h> // debugger symbols
# include <mach/machine.h> // for things like MC88000
#endif

// NOTE: Some tables are copied from https://en.wikipedia.org/wiki/Mach-O




#define Macho_Magic_Number_32  0xfeedface
#define Macho_Magic_Number_64  0xfeedfacf




typedef struct Macho_Section_Buffer {
  String8 *sections;
  U64 count;
} Macho_Section_Buffer;




///////////////////////////////
// CPU type
#define Macho_Cpu_Type_Xlist(X)\
  /* Value       Name,         Description                                */\
  X( 0x00000001, VAX,          "VAX"                                       )\
  X( 0x00000002, ROMP,         "ROMP"                                      )\
  X( 0x00000004, NS32032,      "NS32032"                                   )\
  X( 0x00000005, NS32332,      "NS32332"                                   )\
  X( 0x00000006, MC680x0,      "MC680x0"                                   )\
  X( 0x00000007, x86,          "x86"                                       )\
  X( 0x00000008, MIPS,         "MIPS"                                      )\
  X( 0x00000009, NS32352,      "NS32352"                                   )\
  X( 0x0000000B, HP,           "HP-PA"                                     )\
  X( 0x0000000C, ARM,          "ARM"                                       )\
  X( 0x0000000D, MC88000,      "MC88000"                                   )\
  X( 0x0000000E, SPARC,        "SPARC"                                     )\
  X( 0x0000000F, i860_Big,     "i860 (big-endian)"                         )\
  X( 0x00000010, i860_Little,  "i860 (little-endian) or maybe DEC Alpha[7]")\
  X( 0x00000011, RS,           "RS/6000"                                   )\
  X( 0x00000012, PowerPC,      "PowerPC / MC98000"                         )

// TODO: If the file is for the 64-bit version of the instruction set architecture, the CPU type value has the 0x01000000 bit set.
//       This means we may need to check both verions of the enum values when using them??
typedef enum {
  MachoCpuType__Null,
#define X(value, name, _desc)\
  MachoCpuType_##name = value,
  Macho_Cpu_Type_Xlist(X)
#undef X
} MachoCpuType;


///////////////////////////////
// CPU subtype ARM
#define Macho_Cpu_Subtype_Arm_Xlist(X)\
  /* Value       Name           Description                                  */\
  X( 0x00000000, arm,           "All ARM processors."                         )\
  X( 0x00000001, arm_a500_arch, "Optimized for ARM-A500 ARCH or newer."       )\
  X( 0x00000002, arm_a500,      "Optimized for ARM-A500 or newer."            )\
  X( 0x00000003, arm_a440,      "Optimized for ARM-A440 or newer."            )\
  X( 0x00000004, arm_m4,        "Optimized for ARM-M4 or newer."              )\
  X( 0x00000005, arm_v4t,       "Optimized for ARM-V4T or newer."             )\
  X( 0x00000006, arm_v6,        "Optimized for ARM-V6 or newer."              )\
  X( 0x00000007, arm_v5tej,     "Optimized for ARM-V5TEJ or newer."           )\
  X( 0x00000008, arm_xscale,    "Optimized for ARM-XSCALE or newer."          )\
  X( 0x00000009, arm_v7,        "Optimized for ARM-V7 or newer."              )\
  X( 0x0000000A, arm_v7f,       "Optimized for ARM-V7F (Cortex A9) or newer." )\
  X( 0x0000000B, arm_v7s,       "Optimized for ARM-V7S (Swift) or newer."     )\
  X( 0x0000000C, arm_v7k,       "Optimized for ARM-V7K (Kirkwood40) or newer.")\
  X( 0x0000000D, arm_v8,        "Optimized for ARM-V8 or newer."              )\
  X( 0x0000000E, arm_v6m,       "Optimized for ARM-V6M or newer."             )\
  X( 0x0000000F, arm_v7m,       "Optimized for ARM-V7M or newer."             )\
  X( 0x00000010, arm_v7em,      "Optimized for ARM-V7EM or newer."            )

typedef enum {
#define X(value, name, _desc)\
  MachoCpuSubtypeArm_##name = value,
  Macho_Cpu_Subtype_Arm_Xlist(X)
#undef X
} MachoCpuSubtypeArm;


///////////////////////////////
// CPU subtype x86
#define Macho_Cpu_Subtype_X86_Xlist(X)\
  /* Value       Name             CPU version                            */\
  X( 0x00000003, all,             "All x86 processors."                   )\
  X( 0x00000004, 486,             "Optimized for 486 or newer."           )\
  X( 0x00000084, 486sx,           "Optimized for 486SX or newer."         )\
  X( 0x00000056, pentium_m5,      "Optimized for Pentium M5 or newer."    )\
  X( 0x00000067, celeron,         "Optimized for Celeron or newer."       )\
  X( 0x00000077, celeeron_mobile, "Optimized for Celeron Mobile."         )\
  X( 0x00000008, pentium_3,       "Optimized for Pentium 3 or newer."     )\
  X( 0x00000018, pentium_3_m,     "Optimized for Pentium 3-M or newer."   )\
  X( 0x00000028, pentium_3_xeon,  "Optimized for Pentium 3-XEON or newer.")\
  X( 0x0000000A, pentium_4,       "Optimized for Pentium-4 or newer."     )\
  X( 0x0000000B, itanium,         "Optimized for Itanium or newer."       )\
  X( 0x0000001B, itanium_2,       "Optimized for Itanium-2 or newer."     )\
  X( 0x0000000C, xeon,            "Optimized for XEON or newer."          )\
  X( 0x0000001C, xeon_mp,         "Optimized for XEON-MP or newer."       )

typedef enum {
#define X(value, name, _desc)\
  MachoCpuSubtypeX86_##name = value,
  Macho_Cpu_Subtype_X86_Xlist(X)
#undef X
} MachoCpuSubtypeX86;


///////////////////////////////
// File type
#define Macho_File_Type_Xlist\
  /* Value       Description                                                                                 */\
  X( 0x00000001, "Relocatable object file."                                                                   )\
  X( 0x00000002, "Demand paged executable file."                                                              )\
  X( 0x00000003, "Fixed VM shared library file."                                                              )\
  X( 0x00000004, "Core file."                                                                                 )\
  X( 0x00000005, "Preloaded executable file."                                                                 )\
  X( 0x00000006, "Dynamically bound shared library file."                                                     )\
  X( 0x00000007, "Dynamic link editor."                                                                       )\
  X( 0x00000008, "Dynamically bound bundle file."                                                             )\
  X( 0x00000009, "Shared library stub for static linking only, no section contents."                          )\
  X( 0x0000000A, "Companion file with only debug sections."                                                   )\
  X( 0x0000000B, "x86_64 kexts."                                                                              )\
  X( 0x0000000C, "a file composed of other Mach-Os to be run in the same userspace sharing a single linkedit.")\


///////////////////////////////
// Flag Settings
/* NOTE: 0xxx_0000_0000_0000_0000_0000_0000_0000
         The digits marked with "x" have no use, and are reserved for future use. */
#define Macho_Flag_Settings_Xlist\
  /* Flag    Description */\
  X( 1<<0,   "The object vile has no undefined references.")\
  X( 1<<1,   "The object file is the output of an incremental link against a base file and can't be link edited again.")\
  X( 1<<2,   "The object file is input for the dynamic linker and can't be statically link edited again.")\
  X( 1<<3,   "The object file's undefined references are bound by the dynamic linker when loaded.")\
  X( 1<<4,   "The file has its dynamic undefined references prebound.")\
  X( 1<<5,   "The file has its read-only and read-write segments split.")\
  X( 1<<6,   "The shared library init routine is to be run lazily via catching memory faults to its writeable segments (obsolete).")\
  X( 1<<7,   "The image is using two-level name space bindings.")\
  X( 1<<8,   "The executable is forcing all images to use flat name space bindings.")\
  X( 1<<9,   "This umbrella guarantees no multiple definitions of symbols in its sub-images so the two-level namespace hints can always be used.")\
  X( 1<<10,  "Do not have dyld notify the prebinding agent about this executable.")\
  X( 1<<11,  "The binary is not prebound but can have its prebinding redone. only used when MH_PREBOUND is not set.")\
  X( 1<<12,  "Indicates that this binary binds to all two-level namespace modules of its dependent libraries.")\
  X( 1<<13,  "Safe to divide up the sections into sub-sections via symbols for dead code stripping.")\
  X( 1<<14,  "The binary has been canonicalized via the un-prebind operation.")\
  X( 1<<15,  "The final linked image contains external weak symbols.")\
  X( 1<<16,  "The final linked image uses weak symbols.")\
  X( 1<<17,  "When this bit is set, all stacks in the task will be given stack execution privilege.")\
  X( 1<<18,  "When this bit is set, the binary declares it is safe for use in processes with uid zero.")\
  X( 1<<19,  "When this bit is set, the binary declares it is safe for use in processes when UGID is true.")\
  X( 1<<20,  "When this bit is set on a dylib, the static linker does not need to examine dependent dylibs to see if any are re-exported.")\
  X( 1<<21,  "When this bit is set, the OS will load the main executable at a random address.")\
  X( 1<<22,  "Only for use on dylibs. When linking against a dylib that has this bit set, the static linker will automatically not create a load command to the dylib if no symbols are being referenced from the dylib.")\
  X( 1<<23,  "Contains a section of type S_THREAD_LOCAL_VARIABLES.")\
  X( 1<<24,  "When this bit is set, the OS will run the main executable with a non-executable heap even on platforms (e.g. i386) that don't require it.")\
  X( 1<<25,  "The code was linked for use in an application.")\
  X( 1<<26,  "The external symbols listed in the nlist symbol table do not include all the symbols listed in the dyld info.")\
  X( 1<<27,  "Allow LC_MIN_VERSION_MACOS and LC_BUILD_VERSION load commands with the platforms macOS, macCatalyst, iOSSimulator, tvOSSimulator and watchOSSimulator.")\
  X( 1<<31,  "Only for use on dylibs. When this bit is set, the dylib is part of the dyld shared cache, rather than loose in the filesystem.")










//////////////////
// Load Commands (from mach-o/ldsyms.h)
//////////////////

#define Macho_LC_REQ_DYLD 0x80000000

#define Macho_Load_Command_Xlist(X)\
  X( LC_SEGMENT                  ,  0x1) /* segment of this file to be mapped */\
  X( LC_SYMTAB                   ,  0x2) /* link-edit stab symbol table info */\
  X( LC_SYMSEG                   ,  0x3) /* link-edit gdb symbol table info (obsolete) */\
  X( LC_THREAD                   ,  0x4) /* thread */\
  X( LC_UNIXTHREAD               ,  0x5) /* unix thread (includes a stack) */\
  X( LC_LOADFVMLIB               ,  0x6) /* load a specified fixed VM shared library */\
  X( LC_IDFVMLIB                 ,  0x7) /* fixed VM shared library identification */\
  X( LC_IDENT                    ,  0x8) /* object identification info (obsolete) */\
  X( LC_FVMFILE                  ,  0x9) /* fixed VM file inclusion (internal use) */\
  X( LC_PREPAGE                  ,  0xa)     /* prepage command (internal use) */\
  X( LC_DYSYMTAB                 ,  0xb) /* dynamic link-edit symbol table info */\
  X( LC_LOAD_DYLIB               ,  0xc) /* load a dynamically linked shared library */\
  X( LC_ID_DYLIB                 ,  0xd) /* dynamically linked shared lib ident */\
  X( LC_LOAD_DYLINKER            ,  0xe) /* load a dynamic linker */\
  X( LC_ID_DYLINKER              ,  0xf) /* dynamic linker identification */\
  X( LC_PREBOUND_DYLIB           ,  0x10) /* modules prebound for a dynamically linked shared library */\
  X( LC_ROUTINES                 ,  0x11) /* image routines */\
  X( LC_SUB_FRAMEWORK            ,  0x12) /* sub framework */\
  X( LC_SUB_UMBRELLA             ,  0x13) /* sub umbrella */\
  X( LC_SUB_CLIENT               ,  0x14) /* sub client */\
  X( LC_SUB_LIBRARY              ,  0x15) /* sub library */\
  X( LC_TWOLEVEL_HINTS           ,  0x16) /* two-level namespace lookup hints */\
  X( LC_PREBIND_CKSUM            ,  0x17) /* prebind checksum */\
  X( LC_LOAD_WEAK_DYLIB          , (0x18 | Macho_LC_REQ_DYLD)) /* load a dynamically linked shared library that is allowed to be missing (all symbols are weak imported). */\
  X( LC_SEGMENT_64               ,  0x19) /* 64-bit segment of this file to be mapped */\
  X( LC_ROUTINES_64              ,  0x1a) /* 64-bit image routines */\
  X( LC_UUID                     ,  0x1b) /* the uuid */\
  X( LC_RPATH                    , (0x1c | Macho_LC_REQ_DYLD)   ) /* runpath additions */\
  X( LC_CODE_SIGNATURE           ,  0x1d) /* local of code signature */\
  X( LC_SEGMENT_SPLIT_INFO       ,  0x1e) /* local of info to split segments */\
  X( LC_REEXPORT_DYLIB           , (0x1f | Macho_LC_REQ_DYLD)) /* load and re-export dylib */\
  X( LC_LAZY_LOAD_DYLIB          ,  0x20) /* delay load of dylib until first use */\
  X( LC_ENCRYPTION_INFO          ,  0x21) /* encrypted segment information */\
  X( LC_DYLD_INFO                ,  0x22) /* compressed dyld information */\
  X( LC_DYLD_INFO_ONLY           , (0x22|Macho_LC_REQ_DYLD)) /* compressed dyld information only */\
  X( LC_LOAD_UPWARD_DYLIB        , (0x23 | Macho_LC_REQ_DYLD)) /* load upward dylib */\
  X( LC_VERSION_MIN_MACOSX       ,  0x24) /* build for MacOSX min OS version */\
  X( LC_VERSION_MIN_IPHONEOS     ,  0x25) /* build for iPhoneOS min OS version */\
  X( LC_FUNCTION_STARTS          ,  0x26) /* compressed table of function start addresses */\
  X( LC_DYLD_ENVIRONMENT         ,  0x27) /* string for dyld to treat like environment variable */\
  X( LC_MAIN                     , (0x28|Macho_LC_REQ_DYLD)) /* replacement for LC_UNIXTHREAD */\
  X( LC_DATA_IN_CODE             ,  0x29) /* table of non-instructions in __text */\
  X( LC_SOURCE_VERSION           ,  0x2A) /* source version used to build binary */\
  X( LC_DYLIB_CODE_SIGN_DRS      ,  0x2B) /* Code signing DRs copied from linked dylibs */\
  X( LC_ENCRYPTION_INFO_64       ,  0x2C) /* 64-bit encrypted segment information */\
  X( LC_LINKER_OPTION            ,  0x2D) /* linker options in MH_OBJECT files */\
  X( LC_LINKER_OPTIMIZATION_HINT ,  0x2E) /* optimization hints in MH_OBJECT files */\
  X( LC_VERSION_MIN_TVOS         ,  0x2F) /* build for AppleTV min OS version */\
  X( LC_VERSION_MIN_WATCHOS      ,  0x30) /* build for Watch min OS version */\
  X( LC_NOTE                     ,  0x31) /* arbitrary data included within a Mach-O file */\
  X( LC_BUILD_VERSION            ,  0x32) /* build for platform min OS version */\
  X( LC_DYLD_EXPORTS_TRIE        , (0x33 | Macho_LC_REQ_DYLD)) /* used with linkedit_data_command, payload is trie */\
  X( LC_DYLD_CHAINED_FIXUPS      , (0x34 | Macho_LC_REQ_DYLD)) /* used with linkedit_data_command */\
  X( LC_FILESET_ENTRY            , (0x35 | Macho_LC_REQ_DYLD)) /* used with fileset_entry_command */\
  X( LC_ATOM_INFO                ,  0x36) /* used with linkedit_data_command */

typedef enum {
#define X(name, value, ...)\
  Macho_##name,
  Macho_Load_Command_Xlist(X)
#undef X
} Macho_Load_Command;


typedef enum Macho_Symbol_Flag {
  Macho_Symbol_Flag_DebugEntry      = (1<<0),
  Macho_Symbol_Flag_PrivateExternal = (1<<1),
  Macho_Symbol_Flag_External        = (1<<2),
} Macho_Symbol_Flag;

typedef struct Macho_Symbol {
  Macho_Symbol_Flag flags;
  U32 stab;
  U32 pext;
  U32 type;
  U32 ext;
} Macho_Symbol;



#define Macho_Symbol_Table_Desc_Xlist(X)\
  /* Ident.    Lit.  Comment   */\
  X(N_UNDF  , 0x0  , "undefined, n_sect == NO_SECT")\
  X(N_ABS   , 0x2  , "absolute, n_sect == NO_SECT")\
  X(N_SECT  , 0xe  , "defined in section number n_sect")\
  X(N_PBUD  , 0xc  , "prebound undefined (defined in a dylib)")\
  X(N_INDR  , 0xa  , "indirect")




// TODO: We should be able to use the right columns to determine the symbol-description.
// TODO: Add column names and/or help connect columns with members in `nlist_64`.
// TODO: Create enums for column-types like "address", "linenumber", and "nesting level".
// NOTE: NO_SECT == 0
#define Macho_Debug_Symbol_Table_Desc_Xlist(X)\
  /* Ident.     Lit.   n_strx   n_type  n_sect      n_desc          n_value    comment            */\
  X( N_GSYM   , 0x20,  name,    0,      NO_SECT,          type,             0, "global symbol               ")\
  X( N_FNAME  , 0x22,  name,    0,      NO_SECT,             0,             0, "procedure name (f77 kludge) ")\
  X( N_FUN    , 0x24,  name,    0,       n_sect,    linenumber,       address, "procedure                   ")\
  X( N_STSYM  , 0x26,  name,    0,       n_sect,          type,       address, "static symbol               ")\
  X( N_LCSYM  , 0x28,  name,    0,       n_sect,          type,       address, ".lcomm symbol               ")\
  X( N_BNSYM  , 0x2e,     0,    0,       n_sect,             0,       address, "begin nsect sym             ")\
  X( N_AST    , 0x32,  name,    0,      NO_SECT,             0,             0, "AST file path               ")\
  X( N_OPT    , 0x3c,     0,    0,            0,             0,              , "emitted with gcc2_compiled and in gcc source")\
  X( N_RSYM   , 0x40,  name,    0,      NO_SECT,          type,      register, "register sym                ")\
  X( N_SLINE  , 0x44,     0,    0,       n_sect,    linenumber,       address, "src line                    ")\
  X( N_ENSYM  , 0x4e,     0,    0,       n_sect,             0,       address, "end nsect sym               ")\
  X( N_SSYM   , 0x60,  name,    0,      NO_SECT,          type, struct_offset, "structure elt               ")\
  X( N_SO     , 0x64,  name,    0,       n_sect,             0,       address, "source file name            ")\
  X( N_OSO    , 0x66,  name,    0,      0/* see below */,    1,      st_mtime, "object file name            ")\
  /* Historically N_OSO set n_sect to 0. The N_OSO n_sect may instead hold the low byte of the cpusubtype value from the Mach-O header. */\
  X( N_LIB    , 0x68,  name,    0,      NO_SECT,             0,             0, "dynamic library file name   ")\
  X( N_LSYM   , 0x80,  name,    0,      NO_SECT,          type,        offset, "local sym                   ")\
  X( N_BINCL  , 0x82,  name,    0,      NO_SECT,             0,           sum, "include file beginning      ")\
  X( N_SOL    , 0x84,  name,    0,       n_sect,             0,       address, "#included file name         ")\
  X( N_PARAMS , 0x86,  name,    0,      NO_SECT,             0,             0, "compiler parameters         ")\
  X( N_VERSION, 0x88,  name,    0,      NO_SECT,             0,             0, "compiler version            ")\
  X( N_OLEVEL , 0x8A,  name,    0,      NO_SECT,             0,             0, "compiler -O level           ")\
  X( N_PSYM   , 0xa0,  name,    0,      NO_SECT,          type,        offset, "parameter                   ")\
  X( N_EINCL  , 0xa2,  name,    0,      NO_SECT,             0,             0, "include file end            ")\
  X( N_ENTRY  , 0xa4,  name,    0,       n_sect,    linenumber,       address, "alternate entry             ")\
  X( N_LBRAC  , 0xc0,     0,    0,      NO_SECT, nesting level,       address, "left bracket                ")\
  X( N_EXCL   , 0xc2,  name,    0,      NO_SECT,             0,           sum, "deleted include file        ")\
  X( N_RBRAC  , 0xe0,     0,    0,      NO_SECT, nesting level,       address, "right bracket               ")\
  X( N_BCOMM  , 0xe2,  name,    0,      NO_SECT,             0,             0, "begin common                ")\
  X( N_ECOMM  , 0xe4,  name,    0,       n_sect,             0,             0, "end common                  ")\
  X( N_ECOML  , 0xe8,     0,    0,       n_sect,             0,       address, "end common (local name)     ")\
  X( N_LENG   , 0xfe,     0,    0,            0,             0,              , "second stab entry with length information")\
  X( N_PC     , 0x30,  name,    0,      NO_SECT,       subtype,          line, "global pascal symbol        ")


typedef enum {
#define X(name, l, ...)\
  Macho_##name = (l),
  Macho_Debug_Symbol_Table_Desc_Xlist(X)
#undef X
  Macho_Debug_Symbol_Table_Desc__Count
} Macho_Debug_Symbol_Table_Desc;




#define Macho_Filetype_Desc_Xlist(X)\
  X( 0x1, MH_OBJECT      , "relocatable object file")\
  X( 0x2, MH_EXECUTE     , "demand paged executable file")\
  X( 0x3, MH_FVMLIB      , "fixed VM shared library file")\
  X( 0x4, MH_CORE        , "core file")\
  X( 0x5, MH_PRELOAD     , "preloaded executable file")\
  X( 0x6, MH_DYLIB       , "dynamically bound shared library")\
  X( 0x7, MH_DYLINKER    , "dynamic link editor")\
  X( 0x8, MH_BUNDLE      , "dynamically bound bundle file")\
  X( 0x9, MH_DYLIB_STUB  , "shared library stub for static linking only, no section contents")\
  X( 0xa, MH_DSYM        , "companion file with only debug sections")\
  X( 0xb, MH_KEXT_BUNDLE , "x86_64 kexts")\
  X( 0xc, MH_FILESET     , "a file composed of other Mach-Os to be run in the same userspace sharing a single linkedit.")\
  X( 0xd, MH_GPU_EXECUTE , "gpu program")\
  X( 0xe, MH_GPU_DYLIB   , "gpu support functions")

#define Macho_Flags_Desc_Xlist(X)\
  X(        0x1, MH_NOUNDEFS                      , "the object file has no undefined references")\
  X(        0x2, MH_INCRLINK                      , "the object file is the output of an incremental link against a base file and can't be link edited again")\
  X(        0x4, MH_DYLDLINK                      , "the object file is input for the dynamic linker and can't be staticly link edited again")\
  X(        0x8, MH_BINDATLOAD                    , "the object file's undefined references are bound by the dynamic linker when loaded.")\
  X(       0x10, MH_PREBOUND                      , "the file has its dynamic undefined references prebound.")\
  X(       0x20, MH_SPLIT_SEGS                    , "the file has its read-only and read-write segments split")\
  X(       0x40, MH_LAZY_INIT                     , "the shared library init routine is to be run lazily via catching memory faults to its writeable segments (obsolete)")\
  X(       0x80, MH_TWOLEVEL                      , "the image is using two-level name space bindings")\
  X(      0x100, MH_FORCE_FLAT                    , "the executable is forcing all images to use flat name space bindings")\
  X(      0x200, MH_NOMULTIDEFS                   , "this umbrella guarantees no multiple defintions of symbols in its sub-images so the two-level namespace hints can always be used.")\
  X(      0x400, MH_NOFIXPREBINDING               , "do not have dyld notify the prebinding agent about this executable")\
  X(      0x800, MH_PREBINDABLE                   , "the binary is not prebound but can have its prebinding redone. only used when MH_PREBOUND is not set.")\
  X(     0x1000, MH_ALLMODSBOUND                  , "indicates that this binary binds to all two-level namespace modules of its dependent libraries. only used when MH_PREBINDABLE and MH_TWOLEVEL are both set.")\
  X(     0x2000, MH_SUBSECTIONS_VIA_SYMBOLS       , "safe to divide up the sections into sub-sections via symbols for dead code stripping")\
  X(     0x4000, MH_CANONICAL                     , "the binary has been canonicalized via the unprebind operation")\
  X(     0x8000, MH_WEAK_DEFINES                  , "the final linked image contains external weak symbols")\
  X(    0x10000, MH_BINDS_TO_WEAK                 , "the final linked image uses weak symbols")\
  X(    0x20000, MH_ALLOW_STACK_EXECUTION         , "When this bit is set, all stacks in the task will be given stack execution privilege. Only used in MH_EXECUTE filetypes.")\
  X(    0x40000, MH_ROOT_SAFE                     , "When this bit is set, the binary declares it is safe for use in processes with uid zero")\
  X(    0x80000, MH_SETUID_SAFE                   , "When this bit is set, the binary declares it is safe for use in processes when issetugid() is true")\
  X(   0x100000, MH_NO_REEXPORTED_DYLIBS          , "When this bit is set on a dylib, the static linker does not need to examine dependent dylibs to see if any are re-exported")\
  X(   0x200000, MH_PIE                           , "When this bit is set, the OS will load the main executable at a random address. Only used in MH_EXECUTE filetypes.")\
  X(   0x400000, MH_DEAD_STRIPPABLE_DYLIB         , "Only for use on dylibs. When linking against a dylib that has this bit set, the static linker will automatically not create a LC_LOAD_DYLIB load command to the dylib if no symbols are being referenced from the dylib.")\
  X(   0x800000, MH_HAS_TLV_DESCRIPTORS           , "Contains a section of type S_THREAD_LOCAL_VARIABLES")\
  X(  0x1000000, MH_NO_HEAP_EXECUTION             , "When this bit is set, the OS will run the main executable with a non-executable heap even on platforms (e.g. i386) that don't require it. Only used in MH_EXECUTE filetypes.")\
  X( 0x02000000, MH_APP_EXTENSION_SAFE            , "The code was linked for use in an application extension.")\
  X( 0x04000000, MH_NLIST_OUTOFSYNC_WITH_DYLDINFO , "The external symbols listed in the nlist symbol table do not include all the symbols listed in the dyld info.")\
  X( 0x08000000, MH_SIM_SUPPORT                   , "Allow LC_MIN_VERSION_MACOS and LC_BUILD_VERSION load commands with the platforms macOS, macCatalyst, iOSSimulator, tvOSSimulator and watchOSSimulator.")\
  X( 0x10000000, MH_IMPLICIT_PAGEZERO             , "main executable has no __PAGEZERO segment. Instead, loader (xnu) will load program high and block out all memory below it.")\
  X( 0x80000000, MH_DYLIB_IN_CACHE                , "Only for use on dylibs. When this bit is set, the dylib is part of the dyld shared cache, rather than loose in the filesystem.")

typedef enum Macho_Flag_Index {
#define X(v, n, d, ...)\
  Macho_Flag_Index_##n,
  Macho_Flags_Desc_Xlist(X)
#undef X
  Macho_Flag_Index__Count,
} Macho_Flag_Index;

typedef enum Macho_Filetype {
#define X(v, n, d, ...)\
  Macho_Filetype_##n = (v),
  Macho_Filetype_Desc_Xlist(X)
#undef X
} Macho_Filetype;







static void macho_dump_n_desc_flags(struct nlist_64 s) {
  B32 printed = 0;
  printf("{");
  if (N_NO_DEAD_STRIP & s.n_desc) printf("N_NO_DEAD_STRIP ");
  if (N_DESC_DISCARDED & s.n_desc) printf("N_DESC_DISCARDED ");
  if (N_WEAK_REF & s.n_desc) printf("N_WEAK_REF ");
  if (N_WEAK_DEF & s.n_desc) printf("N_WEAK_DEF ");
  if (N_REF_TO_WEAK & s.n_desc) printf("N_REF_TO_WEAK ");
  if (N_ARM_THUMB_DEF & s.n_desc) printf("N_ARM_THUMB_DEF ");
  if (N_SYMBOL_RESOLVER & s.n_desc) printf("N_SYMBOL_RESOLVER ");
  if (N_ALT_ENTRY & s.n_desc) printf("N_ALT_ENTRY ");
  if (N_COLD_FUNC & s.n_desc) printf("N_COLD_FUNC ");
  printf("}");
}













static U32 macho_flag_lookup[Macho_Flag_Index__Count] = {
#define X(v, n, d, ...)\
  [Macho_Flag_Index_##n] = (v),
  Macho_Flags_Desc_Xlist(X)
#undef X
};

static char *macho_flag_name_lookup[Macho_Flag_Index__Count] = {
#define X(v, n, d, ...)\
  [Macho_Flag_Index_##n] = #n,
  Macho_Flags_Desc_Xlist(X)
#undef X
};

static void macho_debug_dump_flag_string_from_flags(U32 flags) {
  B32 has_printed_a_flag = 0;
  for (U32 i = 0; i < Macho_Flag_Index__Count; ++i) {
    U32 test_flag_value = macho_flag_lookup[i];
    if (flags & test_flag_value) {
      char *flag_name = macho_flag_name_lookup[i];
      if (flag_name) {
        if (has_printed_a_flag) {
          printf(" | ");
        }
        has_printed_a_flag = 1;
        printf("%s", flag_name);
      }
    }
  }
}






// NOTE: Hacker's Delight figure 5-11
static U32 bit_number_of_leading_zeros(U32 x) {
  U32 n = 32;

  if (x != 0) {
    n = 1;
    if ((x >> 16) == 0) { n = n + 16; x = x << 16; }
    if ((x >> 24) == 0) { n = n +  8; x = x <<  8; }
    if ((x >> 28) == 0) { n = n +  4; x = x <<  4; }
    if ((x >> 30) == 0) { n = n +  2; x = x <<  2; }
    n = n - (x >> 31);
  }

  return n;
}


// NOTE: Hacker's Delight figure 5-26
static U32 bit_number_of_trailing_zeros(U32 x) {
  static U8 table[32] = {
     0, 1, 2,24, 3,19, 6,25,  22, 4,20,10,16, 7,12,26,
    31,23,18, 5,21, 9,15,11,  30,17, 8,14,29,13,28,27
  };

  if (x == 0) return 32;
  x = (x & -x)*0x04d7651f;
  return table[x >> 27];
}









static char *macho_get_load_command_name(struct load_command *command) {
  switch(command->cmd)
  {
#define X(name, _value)\
    case name: return #name;
    Macho_Load_Command_Xlist(X);
#undef X
  }

  return "";
}








static MachoCpuType macho_debug_get_cputype_from_raw_cputype(cpu_type_t raw_cputype) {
   MachoCpuType cpu_type = raw_cputype & (~CPU_ARCH_MASK);
   return cpu_type;
}

static String8 macho_debug_get_cputype_string(cpu_type_t raw_cpu_type) {
  MachoCpuType cpu_type = macho_debug_get_cputype_from_raw_cputype(raw_cpu_type);
  switch(cpu_type) {
#define X(v, n, d, ...)\
    case MachoCpuType_##n: return str8_lit(#n);
    Macho_Cpu_Type_Xlist(X)
#undef X
    default: return str8_lit("<Unknown Cpu Type>");
  }
}

static U32 macho_debug_get_cpusubtype_from_raw_cpusubtype(
  cpu_type_t raw_cpu_type,
  cpu_subtype_t raw_cpu_subtype
  ) {
  U32 cpu_subtype = cpu_subtype = raw_cpu_subtype & (~CPU_SUBTYPE_MASK);
  return cpu_subtype;
}

static String8 macho_debug_get_cpusubtype_string(
  cpu_type_t raw_cpu_type,
  cpu_subtype_t raw_cpu_subtype
  ) {
  String8 result = (String8){0};
  MachoCpuType cpu_type = macho_debug_get_cputype_from_raw_cputype(raw_cpu_type);
  U32 cpu_subtype = macho_debug_get_cpusubtype_from_raw_cpusubtype(raw_cpu_type, raw_cpu_subtype);

  if (cpu_type == CPU_TYPE_X86) {
    switch(cpu_subtype) {
#define X(v, n, d, ...)\
      case MachoCpuSubtypeX86_##n: return str8_lit(#n);
      Macho_Cpu_Subtype_X86_Xlist(X)
#undef X
    default: return str8_lit("<Unknown Cpu Subtype>");
    }
  }
  else if (cpu_type == CPU_TYPE_ARM) {
    switch(cpu_subtype) {
#define X(v, n, d, ...)\
      case MachoCpuSubtypeArm_##n: return str8_lit(#n);
      Macho_Cpu_Subtype_Arm_Xlist(X)
#undef X
    default: return str8_lit("<Unknown Cpu Subtype>");
    }
  }

  return result;
}

static String8 macho_debug_get_filetype_string(U32 filetype) {
  switch(filetype) {
#define X(v, n, d, ...)\
    case n: return str8_lit(#n);
    Macho_Filetype_Desc_Xlist(X)
#undef X
  default: return str8_lit("<Unknown filetype>");
  }
}


static void macho_debug_log_header(struct mach_header_64 *header) {
  printf("Image Header\n");
  printf("============\n");
  printf("      magic 0x%x\n", header->magic);
  String8 cputype_string = macho_debug_get_cputype_string(header->cputype);
  printf("    cputype %s\n", cputype_string.str);
  String8 cpu_subtype_string = macho_debug_get_cpusubtype_string(header->cputype, header->cpusubtype);
  printf(" cpusubtype %s\n", cpu_subtype_string.str);
  String8 filetype_string = macho_debug_get_filetype_string(header->filetype);
  printf("   filetype %s\n", filetype_string.str);
  printf("      ncmds %u\n", header->ncmds);
  printf(" sizeofcmds %u\n", header->sizeofcmds);
  {
    printf("      flags 0x%x  ", header->flags);
    macho_debug_dump_flag_string_from_flags(header->flags);
    printf("\n");
  }
  printf("   reserved %x\n", header->reserved);

  printf("\n\n");
}


#define macho_print_uuid(uuid) Assert(sizeof(uuid)==16); macho_print_uuid_(uuid)


static void macho_print_uuid_(U8 *uuid) {
  U32 chunk_sizes[] = {4, 2, 2, 2, 6};
  U32 offset = 0;

  for (U32 c = 0; c < ArrayCount(chunk_sizes); ++c) {
    U32 chunk_size = chunk_sizes[c];
    for (U32 i = 0; i < chunk_size; ++i) {
      printf("%x", uuid[offset+i]);
    }
    if (c+1 < ArrayCount(chunk_sizes)) {
      printf("-");
    }
    offset += chunk_size;
  }
}


static void macho_print_version(U32 version) {
  U32 chunk_sizes[] = {4,2,2};
  U32 offset = 0;
  for (U32 c = 0; c < ArrayCount(chunk_sizes); ++c) {
    U32 chunk_size = chunk_sizes[c];
    for (U32 i = 0; i < chunk_size; ++i) {
      printf("%d", ((version>>(offset+i))&0xf));
    }
    if (c+1 < ArrayCount(chunk_sizes)) {
      printf(".");
    }
  }
}


static void macho_print_source_version(U64 version) {
  /* A.B.C.D.E packed as a24.b10.c10.d10.e10 */
  U32 A = ((version >>  0) & 0xffffff);
  U32 B = ((version >> 24) & 0x3ff);
  U32 C = ((version >> 34) & 0x3ff);
  U32 D = ((version >> 44) & 0x3ff);
  U32 E = ((version >> 54) & 0x3ff);
  printf("%d.%d.%d.%d.%d", A, B, C, D, E);
}



static String8 macho_get_debug_symbol_description(struct nlist_64 symbol) {
  switch(symbol.n_desc) {
#define X(name, ...)\
    case name: return str8_lit(#name);
    Macho_Debug_Symbol_Table_Desc_Xlist(X);
#undef X
  default: return str8_lit("0");
  }
}



#define Get_Masked_Value(m, v)\
  (((U32)(m) & (v)) >> bit_number_of_trailing_zeros((U32)(m)))

static Macho_Symbol macho_get_symbol_data(U8 raw_n_type) {
  Macho_Symbol symbol = (Macho_Symbol){0};

  symbol.stab = Get_Masked_Value(N_STAB, raw_n_type);
  symbol.pext = Get_Masked_Value(N_PEXT, raw_n_type);
  symbol.type = Get_Masked_Value(N_TYPE, raw_n_type);
  symbol.ext  = Get_Masked_Value(N_EXT , raw_n_type);

  return symbol;
}


static char global_symbol_type_name_buffer[64] = {0};

static char *macho_get_symbol_type_name(U8 raw_n_type) {
  U32 masked_value = N_TYPE & raw_n_type;
  switch(masked_value) {
#define X(n, v, ...)\
    case n: return #n;
    Macho_Symbol_Table_Desc_Xlist(X)
#undef X
    default: {
      sprintf(global_symbol_type_name_buffer, "(%x)??", masked_value);
      return global_symbol_type_name_buffer;
    } break;
  }
}






static char *macho_get_debug_symbol_type_name(struct nlist_64 debug_symbol) {
    switch(debug_symbol.n_type) {
    case N_GSYM: return "N_GSYM";
    case N_FNAME: return "N_FNAME";
    case N_FUN: return "N_FUN";
    case N_STSYM: return "N_STSYM";
    case N_LCSYM: return "N_LCSYM";
    case N_BNSYM: return "N_BNSYM";
    case N_AST: return "N_AST";
    case N_OPT: return "N_OPT";
    case N_RSYM: return "N_RSYM";
    case N_SLINE: return "N_SLINE";
    case N_ENSYM: return "N_ENSYM";
    case N_SSYM: return "N_SSYM";
    case N_SO: return "N_SO";
    case N_OSO: return "N_OSO";
    case N_LIB: return "N_LIB";
    case N_LSYM: return "N_LSYM";
    case N_BINCL: return "N_BINCL";
    case N_SOL: return "N_SOL";
    case N_PARAMS: return "N_PARAMS";
    case N_VERSION: return "N_VERSION";
    case N_OLEVEL: return "N_OLEVEL";
    case N_PSYM: return "N_PSYM";
    case N_EINCL: return "N_EINCL";
    case N_ENTRY: return "N_ENTRY";
    case N_LBRAC: return "N_LBRAC";
    case N_EXCL: return "N_EXCL";
    case N_RBRAC: return "N_RBRAC";
    case N_BCOMM: return "N_BCOMM";
    case N_ECOMM: return "N_ECOMM";
    case N_ECOML: return "N_ECOML";
    case N_LENG: return "N_LENG";
    default: return "??????";
    }

  return "<NotStab>";
}



static B32 macho_symbol_table_entry_is_zero(struct nlist_64 symbol) {
  return (symbol.n_un.n_strx == 0 &&
          symbol.n_type == 0 &&
          symbol.n_sect == 0 &&
          symbol.n_desc == 0 &&
          symbol.n_value == 0);
}



static char *macho_get_n_desc_reference_type_name(struct nlist_64 s) {
  U16 ref_type = Get_Masked_Value(REFERENCE_TYPE, s.n_desc);
  switch(ref_type) {
  case REFERENCE_FLAG_UNDEFINED_NON_LAZY: return "UNDEFINED_NON_LAZY";
  case REFERENCE_FLAG_UNDEFINED_LAZY: return "UNDEFINED_LAZY";
  case REFERENCE_FLAG_DEFINED: return "DEFINED";
  case REFERENCE_FLAG_PRIVATE_DEFINED: return "PRIVATE_DEFINED";
  case REFERENCE_FLAG_PRIVATE_UNDEFINED_NON_LAZY: return "PRIVATE_UNDEFINED_NON_LAZY";
  case REFERENCE_FLAG_PRIVATE_UNDEFINED_LAZY: return "PRIVATE_UNDEFINED_LAZY";
  default: return 0;
  }
}




static String8 *macho_get_section_string_from_buffer_index(
  Macho_Section_Buffer buffer,
  U32 i
  ) {
  String8 *section_string = 0;

  if (i >= 0 && i < buffer.count) {
    section_string = buffer.sections + i;
  }

  return section_string;
}





static B32 macho_seems_like_an_n_sect_symbol(
  String8 object_file,
  Macho_Section_Buffer section_buffer,
  struct nlist_64 s
  ) {
  // n_type should be N_SECT
  if ((s.n_type & N_TYPE) != N_SECT) {
    return 0;
  }
  else {
    B32 we_are_here = 1;
  }

  // n_sect should be in range
  if (s.n_sect < 0 || s.n_sect >= section_buffer.count) {
    return 0;
  }

  // n_desc should be 0
  if (0 && s.n_desc != 0) {
    return 0;
  }

#if 0
  // n_value should be somewhat in pointer range
  if (((U8 *)s.n_value < object_file.str) ||
      ((U8 *)s.n_value >= (object_file.str + object_file.size))) {
    return 0;
  }
#endif

  // n_strx should be in range of the section
  /* if (TODO)  */{
    String8 *section_string = macho_get_section_string_from_buffer_index(section_buffer, s.n_sect);
    if (section_string == 0 || s.n_un.n_strx < 0 || s.n_un.n_strx >= section_string->size) {
      return 0;
    }
  }

  return 1;
}





static void macho_print_symbol_table(
  String8 object_file,
  struct mach_header_64 *header,
  Macho_Section_Buffer buffer,
  struct symtab_command *table
  ) {
  struct nlist_64 *symbol_table = (struct nlist_64 *)(object_file.str + table->symoff);
  U8 *start_of_string_table = object_file.str + table->stroff;
  /* printf("start of string table %p \"%s\"\n", start_of_string_table, start_of_string_table); */
  /* printf("start of string table %p   '", start_of_string_table); */
  /* for (U32 i = 0; i < 100; ++i) { */
  /*   printf("%c", start_of_string_table[i]); */
  /* } */
  /* printf("'\n"); */
  /* Assert(0); */

  for (U32 i = 0; i < table->nsyms; ++i) {
    struct nlist_64 *symbol = symbol_table + i;
    B32 is_undefined = symbol->n_type == 0; // TODO: is this really what makes the symbol undefined?
#if 1
    Macho_Symbol macho_symbol = macho_get_symbol_data(symbol->n_type);

    if(0){ // TODO: delete this
      if (macho_symbol.stab)continue;
      if (is_undefined)continue;
      if (macho_symbol.type != (N_SECT>>1))continue;
      if (symbol->n_sect != 5)continue;
      if (macho_seems_like_an_n_sect_symbol(object_file, buffer, *symbol)) {
        if (symbol->n_un.n_strx < table->strsize) {
          U8 *symbol_string = start_of_string_table + symbol->n_un.n_strx;
          /* printf("symbol? (%d)   \"%s\"\n", symbol->n_sect, (U8 *)symbol->n_value + symbol->n_un.n_strx); */
          /* printf("symbol? sect=%d   %s\n", symbol->n_sect, symbol_string); */
          // TODO: try to print the string
          /* continue; */
        }
      }
      else {
        continue;
      }
    }

    /* if (!(REFERENCE_TYPE & symbol->n_desc))continue; */
    printf("stab %3x  ", macho_symbol.stab);
    printf("pext %1x  ", macho_symbol.pext);

    if (symbol->n_un.n_strx == 0) {
      printf("\"\"");
    }
    else {
      if (macho_symbol.stab) {
        char *debug_type_name = macho_get_debug_symbol_type_name(*symbol);
        printf("type %10s  ", debug_type_name);
      }
      else {
        char *type_name = macho_get_symbol_type_name(symbol->n_type);
        /* printf("type %10x  ", macho_symbol.type); */
        printf("type %10s  ", type_name);
      }
    }
    printf(" ext %1x  ", macho_symbol.ext );

    printf("n_type 0x%08x  ", symbol->n_type);
    printf("n_strx 0x%08x  ", symbol->n_un.n_strx);
    printf("n_sect %02d  ", symbol->n_sect);

    printf("n_desc 0x%04x", symbol->n_desc);
    if (0) {
      char *type_name = macho_get_n_desc_reference_type_name(*symbol);
      if (type_name) {
        printf("(%s)", type_name);
      }
      if (Get_Masked_Value(N_NO_DEAD_STRIP, symbol->n_desc)) {
        printf("[no_dead_strip__or__discarded]");
      }
      macho_dump_n_desc_flags(*symbol);
      printf("  ");
    }

    if (symbol->n_sect >= 0 && symbol->n_sect < buffer.count) {
      String8 *section_string = macho_get_section_string_from_buffer_index(buffer, symbol->n_sect);
      if (symbol->n_un.n_strx < section_string->size) {
        /* printf(" \"%s\"  ", section_string->str + symbol->n_un.n_strx); */
      }
    }
    printf("  ");
    printf("n_value 0x%016llx\n", symbol->n_value);
#else
    printf("n_strx 0x%08x  ", symbol->n_un.n_strx);
    /* printf("n_type 0x%02x  ", symbol->n_type); */
    printf("n_type 0x%04x  ", macho_symbol.type);
    printf("n_sect 0x%02x  ", symbol->n_sect);
    printf("n_desc 0x%04x  ", symbol->n_desc);
    printf("n_value 0x%016llx\n", symbol->n_value);
#endif
  }
}




static void
macho_print_dynamic_symbol_table(String8 object_file, struct dysymtab_command *table) {
}




static Macho_Section_Buffer macho_create_section_buffer(
  Arena *arena,
  String8 object_file,
  struct mach_header_64 header
  ) {
  Macho_Section_Buffer buffer = (Macho_Section_Buffer){0};
  U64 section_count = 1; // NOTE: Section indexes start at 1
  U64 command_offset = sizeof(struct mach_header_64);

  // get total section count
  for (U32 i = 0; i < header.ncmds; ++i) {
    struct load_command *command = (struct load_command *)(object_file.str + command_offset);

    if (command->cmd == LC_SEGMENT_64) {
      struct segment_command_64 *segment = (struct segment_command_64 *)command;
      section_count += segment->nsects;
    }

    command_offset += command->cmdsize;
  }

  // fill out section pointers
  if (section_count) {
    U32 section_index = 1;
    buffer.sections = push_array(arena, String8, section_count);
    if (buffer.sections) {
      buffer.count = section_count;
      command_offset = sizeof(struct mach_header_64);
      for (U32 i = 0; i < header.ncmds; ++i) {
        struct load_command *command = (struct load_command *)(object_file.str + command_offset);
        if (command->cmd == LC_SEGMENT_64) {
          struct segment_command_64 *segment = (struct segment_command_64 *)command;
          for (U32 i = 0; i < segment->nsects; ++i) {
            struct section_64 *section = ((struct section_64 *)(segment + 1)) + i;
            Assert(section_index >= 0 && section_index < section_count);
            buffer.sections[section_index].str = object_file.str + section->offset;
            buffer.sections[section_index].size = section->size;
            section_index += 1;
          }
        }
        command_offset += command->cmdsize;
      }
    }
  }

  /* Assert(0); */
  return buffer;
}



static void macho_dump_object_file(Arena *arena, String8 object_file) {
  if (object_file.str) {
    struct mach_header_64 *header = (struct mach_header_64 *)object_file.str;
    U64 command_offset = sizeof(struct mach_header_64);
    Macho_Section_Buffer section_buffer = macho_create_section_buffer(arena, object_file, *header);
    for (U32 i = 0; i < section_buffer.count; ++i) {
      String8 *section = section_buffer.sections + i;
      printf("section[%3d]    ", i);
      {
        for (U64 j = 0; j < Min(100, section->size); ++j) {
          char c = section->str[j];
          B32 is_printable = c >= 32 && c < 127;
          printf("%c", is_printable ? c : '-');
        }
      }
      printf("\n");
    }
    macho_debug_log_header(header);

    // TODO: Check to make sure the size fits in the header's load_command_size
    // Loop through load-commands
    for (U32 count = 0; count < header->ncmds; ++count) {
      struct load_command *command = (struct load_command *)(object_file.str + command_offset);

      // dump 64-bit segment
      if (command->cmd == LC_SEGMENT_64) {
        struct segment_command_64 *segment = (struct segment_command_64 *)command;
        printf("%s with %d sections and size %u\n", segment->segname, segment->nsects, command->cmdsize);

        for (U32 i = 0; i < segment->nsects; ++i) {
          struct section_64 *section = ((struct section_64 *)(segment + 1)) + i;
          printf("  section %p '%s'  size=%llu   foff=%u", (void *)section->addr, section->sectname, section->size, section->offset);
          printf("  align=%u\n", section->align);
        }

      } else if (command->cmd == LC_DYLD_EXPORTS_TRIE) {
        struct linkedit_data_command *data = (struct linkedit_data_command *)command;
        printf("LC_DYLD_EXPORTS_TRIE at foff=0x%x  size=0x%x\n", data->dataoff, data->datasize);

      } else if (command->cmd == LC_SYMTAB) {
        struct symtab_command *table = (struct symtab_command *)command;
        printf("LC_SYMTAB at symoff=0x%x  nsyms=0x%x\n", table->symoff, table->nsyms);
        printf("             stroff=0x%x strsize=0x%x\n", table->stroff, table->strsize);
        macho_print_symbol_table(object_file, header, section_buffer, table);

      } else if (command->cmd == LC_DYSYMTAB) {
        struct dysymtab_command *table = (struct dysymtab_command *)command;
        printf("LC_DYSYMTAB       local_index=%d      local_count=%d\n", table->ilocalsym, table->nlocalsym);
        printf("               external_index=%d   external_count=%d\n", table->iextdefsym, table->nextdefsym);
        printf("              undefined_index=%d  undefined_count=%d\n", table->iundefsym, table->nundefsym);
        macho_print_dynamic_symbol_table(object_file, table);

      } else if (command->cmd == LC_ID_DYLINKER ||
                 command->cmd == LC_LOAD_DYLINKER ||
                 command->cmd == LC_DYLD_ENVIRONMENT) {
        struct dylinker_command *table = (struct dylinker_command *)command;
        char *command_name = "";
        switch(command->cmd){
        case LC_ID_DYLINKER      :command_name="LC_ID_DYLINKER";break;
        case LC_LOAD_DYLINKER    :command_name="LC_LOAD_DYLINKER";break;
        case LC_DYLD_ENVIRONMENT :command_name="LC_DYLD_ENVIRONMENT";break;
        }
        printf("%s at path '%s'\n", command_name, ((U8 *)command)+table->name.offset);

      } else if (command->cmd == LC_UUID) {
        printf("LC_UUID ");
        struct uuid_command *uuid = (struct uuid_command *)command;
        macho_print_uuid(uuid->uuid);
        printf("\n");

      } else if (command->cmd == LC_BUILD_VERSION) {
        struct build_version_command *version = (struct build_version_command *)command;
        printf("LC_BUILD_VERSION platform=%d\n", version->platform);
        printf("                   min-os=");
        macho_print_version(version->minos);
        printf("\n                      sdk=");
        macho_print_version(version->sdk);
        printf("\n");

      } else if (command->cmd == LC_SOURCE_VERSION) {
        struct source_version_command *version = (struct source_version_command *)command;
        printf("LC_SOURCE_VERSION version=");
        macho_print_source_version(version->version);
        printf("\n");

      } else if (command->cmd == LC_MAIN) {
        struct entry_point_command *entry_point = (struct entry_point_command *)command;
        printf("LC_MAIN  foff=%llu   stack-size=%llu\n", entry_point->entryoff, entry_point->stacksize);

      } else if (command->cmd == LC_LOAD_DYLIB ||
                 command->cmd == LC_ID_DYLIB ||
                 command->cmd == LC_LOAD_WEAK_DYLIB ||
                 command->cmd == LC_REEXPORT_DYLIB) {
        char *command_name = 0;
        switch(command->cmd){
        case LC_LOAD_DYLIB: command_name="LC_LOAD_DYLIB";break;
        case LC_ID_DYLIB: command_name="LC_ID_DYLIB";break;
        case LC_LOAD_WEAK_DYLIB: command_name="LC_LOAD_WEAK_DYLIB";break;
        case LC_REEXPORT_DYLIB: command_name="LC_REEXPORT_DYLIB";break;
        }
        struct dylib_command *dylib_command = (struct dylib_command *)command;
        printf("%s  '%s'\n", command_name, ((U8 *)dylib_command) + dylib_command->dylib.name.offset);

      } else if (command->cmd == LC_FUNCTION_STARTS ||
                 command->cmd == LC_CODE_SIGNATURE ||
                 command->cmd == LC_SEGMENT_SPLIT_INFO ||
                 command->cmd == LC_FUNCTION_STARTS ||
                 command->cmd == LC_DATA_IN_CODE ||
                 command->cmd == LC_DYLIB_CODE_SIGN_DRS ||
                 command->cmd == LC_ATOM_INFO ||
                 command->cmd == LC_LINKER_OPTIMIZATION_HINT ||
                 command->cmd == LC_DYLD_EXPORTS_TRIE ||
                 command->cmd == LC_DYLD_CHAINED_FIXUPS) {
        char *command_name = "";
        switch(command->cmd){
        case LC_CODE_SIGNATURE: command_name="LC_CODE_SIGNATURE";break;
        case LC_SEGMENT_SPLIT_INFO: command_name="LC_SEGMENT_SPLIT_INFO";break;
        case LC_FUNCTION_STARTS: command_name="LC_FUNCTION_STARTS";break;
        case LC_DATA_IN_CODE: command_name="LC_DATA_IN_CODE";break;
        case LC_DYLIB_CODE_SIGN_DRS: command_name="LC_DYLIB_CODE_SIGN_DRS";break;
        case LC_ATOM_INFO: command_name="LC_ATOM_INFO";break;
        case LC_LINKER_OPTIMIZATION_HINT: command_name="LC_LINKER_OPTIMIZATION_HINT";break;
        case LC_DYLD_EXPORTS_TRIE: command_name="LC_DYLD_EXPORTS_TRIE";break;
        case LC_DYLD_CHAINED_FIXUPS: command_name="LC_DYLD_CHAINED_FIXUPS)";break;
        }
        struct linkedit_data_command *linkedit = (struct linkedit_data_command *)command;
        printf("%s  foff=%d   size=%d\n", command_name, linkedit->dataoff, linkedit->datasize);

      } else {
        printf("### Unhandled command type 0x%08x '%s'\n", command->cmd, macho_get_load_command_name(command));
      }

      command_offset += command->cmdsize;
    }
    printf("\nObject file size %llu\n", object_file.size);
  }
}



static void resize_string8(String8 *string, U32 max_size) {
  U64 actual_size = 0;

  for (; actual_size < max_size && string->str[actual_size]; ++actual_size);
  string->size = actual_size;
}



static RangeAddr
macho_get_section(String8 object_file, String8 segment_name, String8 section_name) {
  RangeAddr range = {};

  if (object_file.str) {
    struct mach_header_64 *header = (struct mach_header_64 *)object_file.str;
    U64 command_offset = sizeof(struct mach_header_64);

    // Loop through load-commands
    for (U32 count = 0; count < header->ncmds; ++count) {
      struct load_command *command = (struct load_command *)(object_file.str + command_offset);

      // 64-bit segment
      if (command->cmd == LC_SEGMENT_64) {
        struct segment_command_64 *segment = (struct segment_command_64 *)command;

        String8 test_segment_name = str8_lit(segment->segname);
        resize_string8(&test_segment_name, 16);

        if (str8_match(segment_name, test_segment_name, 0)) {
          for (U32 i = 0; i < segment->nsects; ++i) {
            struct section_64 *section = ((struct section_64 *)(segment + 1)) + i;

            String8 test_section_name = str8_lit(section->sectname);
            resize_string8(&test_section_name, 16);

            if (str8_match(section_name, test_section_name, 0)) {
              range.first = (U8 *)section->addr;
              range.opl = range.first + section->size;
            }
          }
        }
      }

      command_offset += command->cmdsize;
    }
  }

  return range;
}


#endif // MACHO_INCLUDE_H
