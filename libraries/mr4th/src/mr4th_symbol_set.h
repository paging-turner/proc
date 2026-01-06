/*
  DISCLAIMER: This is a non-standard, experimental version of "mr4th_symbol_set.h".

  This version is NOT portable with the standard version from the mr4th codebase.

  If you want to use symbol-sets, then it would be best to get the symbol-set code from mr4th.com.
*/




#ifndef MR4TH_SYMBOL_SET_H
#define MR4TH_SYMBOL_SET_H

////////////////////////////////
// Copy-Pastable
#if 0

// To setup the Symbol Set system: (once per translation unit)
#include "mr4th_symbol_set.h"

// To define a Symbol Set
#define SYMBOL_SET_DEFINE example_name
#define example_name_Type    ExampleType
#define example_name_section ".exmpl"
#include "mr4th_symbol_set.define.h"

// Common wrappers to put around a Symbol Set (after defining it)
#define EXAMPLE_ID(N)   SymbolID(example_name, N)
#define EXAMPLE_RAW(N)  SymbolRaw(example_name, N)
#define EXAMPLE_DECL(N) SymbolDeclare(example_name, N)

#endif


////////////////////////////////
// Symbol Set System

// Symbol Set key terms:
// "count"    - the total number of symbols in this set
// "baseptr"  - the pointer to the base of the array of symbol data
// "oplptr"   - the pointer to the spot one past the last element of the array
// "id"       - the 1-based index of the symbol
//              id=0 stands for the special 'nil' case
//              id values in the range [1,count] stand for a symbol in the set
// "metadata" - a pointer to the slot in the array for this symbol
// "raw"      - address of the metadata as an integer

// user interface

#define SymbolDeclare(E,N) MR4TH_SECNAME(E##_section) SYMBOL__TYPE(E) SYMBOL__SYM(E,N)

#define SymbolDefine(E,N) MR4TH_DO_NOT_ELIMINATE(SYMBOL__SYM(E,N)) SymbolDeclare(E,N)
#define SymbolDefineNameless(E) SymbolDefine(E,Glue(auto,__COUNTER__))

#define SymbolCount(E)   Glue(E, _count)
#define SymbolBasePtr(E) Glue(E, _baseptr)
#define SymbolOplPtr(E)  (SymbolBasePtr(E) + SymbolCount(E))

#define SymbolMetadata(E,N) (&SYMBOL__SYM(E,N))
#define SymbolID(E,N)       SymbolIDFromMetadata(E, SymbolMetadata(E,N))

#define SymbolIDFromMetadata(E,ptr) \
((SymbolBasePtr(E) <= (ptr) && (ptr) < SymbolOplPtr(E))? \
(U32)(1 + ((ptr) - SymbolBasePtr(E))):0)

#define SymbolMetadataFromID(E,id) \
((1 <= (id) && (id) <= SymbolCount(E))?\
(SymbolBasePtr(E) + (id) - 1):(&SYMBOL__SYM(E,0)))

#define SymbolRaw(E,N)         IntFromPtr(SymbolMetadata(E,N))
#define SymbolIDFromRaw(E,raw) SymbolIDFromMetadata(E,(SYMBOL__TYPE(E)*)(PtrFromInt(raw)))
#define SymbolRawFromID(E,id)  IntFromPtr(SymbolMetadataFromID(E,id))

// internal

#define SYMBOL__TYPE(E)    Glue(E,_Type)
#define SYMBOL__SYM(E,N)   Glue(E, Glue(__, N))

#define SYMBOL__RAW_BASE(E) IntFromPtr(SymbolBasePtr(E))
#define SYMBOL__RAW_OPL(E)  (SYMBOL__RAW_BASE(E) + SymbolCount(E)*sizeof(SYMBOL__TYPE(E)))

#define SYMBOL__BEFORE_MAIN_NAME Glue(SYMBOL_SET_DEFINE,__init)
#define SYMBOL__CL_PRAGMA section(Glue(SYMBOL_SET_DEFINE,_section),read,write)

#endif //MR4TH_SYMBOL_SET_H
