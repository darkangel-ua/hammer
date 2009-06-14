#include "stdafx.h"
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <boost/assign/list_of.hpp>
#include <vector>
#include <hammer/core/type_registry.h>
#include <hammer/core/feature_set.h>

using namespace std;
using namespace boost::assign;

namespace hammer{ namespace types{

const type_tag CPP("CPP");
const type_tag C("C");
const type_tag H("H");
const type_tag RC("RC");
const type_tag RES("RES");
const type_tag OBJ("OBJ");
const type_tag PCH("PCH");
const type_tag LIB("LIB");
const type_tag SHARED_LIB("SHARED_LIB");
const type_tag PREBUILT_SHARED_LIB("PREBUILT_SHARED_LIB");
const type_tag IMPORT_LIB("IMPORT_LIB");
const type_tag STATIC_LIB("STATIC_LIB");
const type_tag PREBUILT_STATIC_LIB("PREBUILT_STATIC_LIB");
const type_tag SEARCHED_LIB("SEARCHED_LIB");
const type_tag SEARCHED_SHARED_LIB("SEARCHED_SHARED_LIB");
const type_tag SEARCHED_STATIC_LIB("SEARCHED_STATIC_LIB");
const type_tag HEADER_LIB("HEADER_LIB");
const type_tag EXE("EXE");
const type_tag EXE_MANIFEST("EXE_MANIFEST");
const type_tag DLL_MANIFEST("DLL_MANIFEST");
const type_tag UNKNOWN("UNKNOWN");
const type_tag COPIED("COPIED");
const type_tag TESTING_OUTPUT("TESTING_OUTPUT");
const type_tag TESTING_RUN_PASSED("TESTING_RUN_PASSED");

void register_standart_types(type_registry& tr, feature_registry& fr)
{
   target_type::suffixes_t cpp_suffixes = list_of(".cpp")(".cc");
   const target_type& tCPP = tr.insert(target_type(CPP, cpp_suffixes));
   
   const target_type& tC = tr.insert(target_type(C, ".c"));
   
   target_type::suffixes_t h_suffixes = list_of(".h")(".hpp");
   const target_type& tH = tr.insert(target_type(H, h_suffixes));

   const target_type& tRC = tr.insert(target_type(RC, ".rc"));
   const target_type& tRES = tr.insert(target_type(RES, ".res"));
   
   target_type::suffixes_t obj_suffixes = list_of<target_type::suffix_def>(".obj", *parse_simple_set("<os>nt", fr))
                                                                          (".o", *parse_simple_set("<os>linux", fr));
   const target_type& tOBJ = tr.insert(target_type(OBJ, obj_suffixes));
   const target_type& tPCH = tr.insert(target_type(PCH, ".pch"));
   const target_type& tLIB = tr.insert(target_type(LIB, ""));
   target_type::suffixes_t shared_suffixes = list_of<target_type::suffix_def>(".dll", *parse_simple_set("<os>nt", fr))
                                                                             (".so", *parse_simple_set("<os>linux", fr));
   const target_type& tSHARED_LIB = tr.insert(target_type(SHARED_LIB, shared_suffixes, tLIB));
   const target_type& tPREBUILT_SHARED_LIB = tr.insert(target_type(PREBUILT_SHARED_LIB, "", tSHARED_LIB));
   const target_type& tSTATIC_LIB = tr.insert(target_type(STATIC_LIB, ".lib", tLIB));
   const target_type& tPREBUILT_STATIC_LIB = tr.insert(target_type(PREBUILT_STATIC_LIB, "", tSTATIC_LIB));

   target_type::suffixes_t static_lib_suffixes = list_of<target_type::suffix_def>(".lib", *parse_simple_set("<os>nt", fr))
                                                                                 (".a", *parse_simple_set("<os>linux", fr));
   const target_type& tIMPORT_LIB = tr.insert(target_type(IMPORT_LIB, static_lib_suffixes, tSTATIC_LIB));
   const target_type& tSEARCHED_LIB = tr.insert(target_type(SEARCHED_LIB, "", tLIB));
   const target_type& tSEARCHED_SHARED_LIB = tr.insert(target_type(SEARCHED_SHARED_LIB, "", tSEARCHED_LIB));
   const target_type& tSEARCHED_STATIC_LIB = tr.insert(target_type(SEARCHED_STATIC_LIB, "", tSEARCHED_LIB));
   const target_type& tHEADER_LIB = tr.insert(target_type(HEADER_LIB, target_type::suffixes_t(), tLIB));
   target_type::suffixes_t exe_suffixes = list_of<target_type::suffix_def>(".exe", *parse_simple_set("<os>nt", fr))
                                                                           ("", *parse_simple_set("<os>linux", fr));
   const target_type& tEXE = tr.insert(target_type(EXE, exe_suffixes));
   const target_type& tEXE_MANIFEST = tr.insert(target_type(EXE_MANIFEST, ".exe.manifest"));
   const target_type& tDLL_MANIFEST = tr.insert(target_type(DLL_MANIFEST, ".dll.manifest"));
   const target_type& tUNKNOWN = tr.insert(target_type(UNKNOWN, ""));
   const target_type& tCOPIED = tr.insert(target_type(COPIED, ""));
   const target_type& tTESTING_OUTPUT = tr.insert(target_type(TESTING_OUTPUT, ".test_output"));
   const target_type& tTESTING_RUN_PASSED = tr.insert(target_type(TESTING_RUN_PASSED, ".run_passed"));
}

}}
