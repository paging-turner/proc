MR4TH_SYM_COMPTIME U64 SymbolCount(SYMBOL_SET_DEFINE) = 0;
MR4TH_SYM_COMPTIME SYMBOL__TYPE(SYMBOL_SET_DEFINE) * SymbolBasePtr(SYMBOL_SET_DEFINE) = 0;
MR4TH_SYM_COMPTIME SYMBOL__TYPE(SYMBOL_SET_DEFINE) SYMBOL__SYM(SYMBOL_SET_DEFINE,0);
#if COMPILER_CL
# pragma SYMBOL__CL_PRAGMA
#endif
MR4TH_BEFORE_MAIN(SYMBOL__BEFORE_MAIN_NAME){
#if OS_MAC
  String8 lib = os_this_image();
  String8 segment_name = str8_lit("__DATA");
  String8 section_name = str8_lit(Glue(SYMBOL_SET_DEFINE, _section));
  RangeAddr range = macho_get_section(lib, segment_name, section_name);
#else
  RangeAddr range = selfimg_get_section_by_name(os_this_image(), str8_lit(Glue(SYMBOL_SET_DEFINE,_section)));
#endif
  SymbolCount(SYMBOL_SET_DEFINE) = (range.opl - range.first)/sizeof(SYMBOL__TYPE(SYMBOL_SET_DEFINE));
  SymbolBasePtr(SYMBOL_SET_DEFINE) = (SYMBOL__TYPE(SYMBOL_SET_DEFINE)*)(range.first);
}

#undef SYMBOL_SET_DEFINE
