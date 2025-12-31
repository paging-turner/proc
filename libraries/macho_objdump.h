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


///////////////////////////////
// CPU type
#define Macho_Cpu_Type_Xlist\
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
#define X(value, name, _desc)\
  MachoCpuType_##name = value,
  Macho_Cpu_Type_Xlist
#undef X
} MachoCpuType;


///////////////////////////////
// CPU subtype ARM
#define Macho_Cpu_Subtype_Arm_Xlist\
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
  MachoCpuSubtype_##name = value,
  Macho_Cpu_Subtype_Xlist
#undef X
} MachoCpuSubtype;


///////////////////////////////
// CPU subtype x86
#define Macho_Cpu_Subtype_X86_Xlist\
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
  Macho_Cpu_Subtype_X86_Xlist
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

#define Macho_Load_Command_Xlist\
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
  Macho_Load_Command_Xlist
#undef X
} Macho_Load_Command;


// TODO: We should be able to use the right columns to determine the symbol-description.
// TODO: Add column names and/or help connect columns with members in `nlist_64`.
// TODO: Create enums for column-types like "address", "linenumber", and "nesting level".
// NOTE: NO_SECT == 0
#define Macho_Symbol_Table_Desc_Xlist\
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
#define X(name, ...)\
  Macho_##name,
  Macho_Symbol_Table_Desc_Xlist
#undef X
  Macho_Symbol_Table_Desc__Count
} Macho_Symbol_Table_Desc;


















static void macho_debug_dump_load_command_values(void) {
#define X(name, _value)\
  printf("%s %d\n", #name, name);
  Macho_Load_Command_Xlist;
#undef X
}


static char *macho_get_load_command_name(struct load_command *command) {
  switch(command->cmd)
  {
#define X(name, _value)\
    case name: return #name;
    Macho_Load_Command_Xlist;
#undef X
  }

  return "";
}


static B32 macho_load_command_has_valid_type(struct load_command *command) {
  B32 has_valid_type = 0;

  switch(command->cmd) {
#define X(name, _value)\
    case name: { has_valid_type = 1; } break;
    Macho_Load_Command_Xlist;
#undef X
  }

  return has_valid_type;
}


static void macho_debug_log_header(struct mach_header_64 *header) {
  printf("Image Header\n");
  printf("============\n");
  printf("      magic %x\n", header->magic);
  printf("    cputype %x\n", header->cputype);
  printf(" cpusubtype %x\n", header->cpusubtype);
  printf("   filetype %x\n", header->filetype);
  printf("      ncmds %x\n", header->ncmds);
  printf(" sizeofcmds %x\n", header->sizeofcmds);
  printf("      flags %x\n", header->flags);
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



static String8 macho_get_symbol_description(struct nlist_64 symbol) {
  switch(symbol.n_desc) {
#define X(name, ...)\
    case name: return str8_lit(#name);
    Macho_Symbol_Table_Desc_Xlist;
#undef X
  default: return str8_lit("0");
  }
}


// TODO: I originally though that these symbols were the ones I wanted, but it seems we may need LC_DYSYMTAB instead?
//       Reconsider how we name these things once we untangle what means what.
static char *macho_get_symbol_type_name(struct nlist_64 symbol) {
  U32 raw_type = symbol.n_type;
  B32 is_debug_entry      = (N_STAB & raw_type) != 0;
  B32 is_private_external = (N_PEXT & raw_type) != 0;
  B32 is_external         = (N_EXT  & raw_type) != 0;
  U32 type = raw_type & N_TYPE;

  printf("  %s%s%s  ", is_debug_entry?"d":" ", is_private_external?"p":" ", is_external?"e":" ");

  // these are symbol->n_value ?
  /* case NO_SECT: return "NO_SECT"; */
  /* case MAX_SECT: return "MAX_SECT"; */

  /* if (symbol.n_un.n_strx == 0) { */
  /*   return "<Unknown>"; */
  /* } */
  switch(type) {
  case N_UNDF: return "N_UNDF";
  case N_ABS: return "N_ABS";
  case N_SECT: return "N_SECT";
  case N_PBUD: return "N_PBUD";
  case N_INDR: return "N_INDR";
  default: return "<Unknown>";
  }
#if 0
  switch(raw_type) {
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
  case N_UNDF: return "N_UNDF";
  case N_ABS: return "N_ABS";
  case N_SECT: return "N_SECT";
  case N_PBUD: return "N_PBUD";
  case N_INDR: return "N_INDR";
  default: return "<Unknown>";
  }
#endif
}


static B32 macho_symbol_table_entry_is_zero(struct nlist_64 symbol) {
  return (symbol.n_un.n_strx == 0 &&
          symbol.n_type == 0 &&
          symbol.n_sect == 0 &&
          symbol.n_desc == 0 &&
          symbol.n_value == 0);
}

static B32 macho_symbol_table_entry_has_only_value(struct nlist_64 symbol) {
  return (symbol.n_un.n_strx == 0 &&
          symbol.n_type == 0 &&
          symbol.n_sect == 0 &&
          symbol.n_desc == 0);
}

static void macho_print_symbol_table(String8 object_file, struct symtab_command *table) {
  struct nlist_64 *symbol_table = (struct nlist_64 *)(object_file.str + table->symoff);

  for (U32 i = 0; i < table->nsyms; ++i) {
    struct nlist_64 *symbol = symbol_table + i;
    if (!macho_symbol_table_entry_is_zero(*symbol)) {
      char *symbol_type = macho_get_symbol_type_name(*symbol);
      String8 description = macho_get_symbol_description(*symbol);
      B32 is_unknown = symbol_type[0] == '<'; // @HACK macho_get_symbol_type_name

      if (macho_symbol_table_entry_has_only_value(*symbol)) {
        /* printf("\"%s\" ", object_file.str + (table->stroff + symbol->n_value)); */
      }

      printf("n_strx 0x%08x  ", symbol_table->n_un.n_strx);
      printf("n_type 0x%02x  ", symbol->n_type);
      printf("n_sect 0x%02x  ", symbol->n_sect);
      printf("n_desc 0x%04x  ", symbol->n_desc);
      printf("n_value 0x%016llx\n", symbol->n_value);
    }
  }
}

static void
macho_print_dynamic_symbol_table(String8 object_file, struct dysymtab_command *table) {
}



static void macho_dump_object_file(String8 object_file) {
  if (object_file.str) {
    struct mach_header_64 *header = (struct mach_header_64 *)object_file.str;
    U64 command_offset = sizeof(struct mach_header_64);

    macho_debug_log_header(header);

    // TODO: Check to make sure the size fits in the header's load_command_size
    // Loop through load-commands
    for (U32 count = 0; count < header->ncmds; ++count) {
      struct load_command *command = (struct load_command *)(object_file.str + command_offset);
      Assert(macho_load_command_has_valid_type(command));

      // dump 64-bit segment
      if (command->cmd == LC_SEGMENT_64) {
        struct segment_command_64 *segment = (struct segment_command_64 *)command;
        printf("%s with %d sections and size %u\n", segment->segname, segment->nsects, command->cmdsize);

        for (U32 i = 0; i < segment->nsects; ++i) {
          struct section_64 *section = ((struct section_64 *)(segment + 1)) + i;
          printf("  section %p '%s'  size=%llu", (void *)section->addr, section->sectname, section->size);
          printf("  align=%u\n", section->align);
        }

      } else if (command->cmd == LC_DYLD_EXPORTS_TRIE) {
        struct linkedit_data_command *data = (struct linkedit_data_command *)command;
        printf("LC_DYLD_EXPORTS_TRIE at foff=0x%x  size=0x%x\n", data->dataoff, data->datasize);

      } else if (command->cmd == LC_SYMTAB) {
        struct symtab_command *table = (struct symtab_command *)command;
        printf("LC_SYMTAB at symoff=0x%x  nsyms=0x%x\n", table->symoff, table->nsyms);
        printf("             stroff=0x%x strsize=0x%x\n", table->stroff, table->strsize);
        macho_print_symbol_table(object_file, table);

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
      Assert(macho_load_command_has_valid_type(command));

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
