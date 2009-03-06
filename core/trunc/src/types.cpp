#include "stdafx.h"
#include <hammer/core/types.h>
#include <hammer/core/type.h>
#include <boost/assign/list_of.hpp>
#include <vector>
#include <hammer/core/type_registry.h>

using namespace std;
using namespace boost::assign;

namespace hammer{ namespace types{

const type_tag CPP("CPP");
const type_tag C("C");
const type_tag H("H");
const type_tag OBJ("OBJ");
const type_tag PCH("PCH");
const type_tag LIB("LIB");
const type_tag SHARED_LIB("SHARED_LIB");
const type_tag IMPORT_LIB("IMPORT_LIB");
const type_tag STATIC_LIB("STATIC_LIB");
const type_tag SEARCHED_LIB("SEARCHED_LIB");
const type_tag HEADER_LIB("HEADER_LIB");
const type_tag EXE("EXE");
const type_tag EXE_MANIFEST("EXE_MANIFEST");
const type_tag DLL_MANIFEST("DLL_MANIFEST");
const type_tag UNKNOWN("UNKNOWN");
const type_tag COPIED("COPIED");
const type_tag TESTING_OUTPUT("TESTING_OUTPUT");
const type_tag TESTING_RUN_PASSED("TESTING_RUN_PASSED");

void register_standart_types(type_registry& tr)
{
   const type& tCPP = tr.insert(type(CPP, type::suffixes_t(list_of(".cpp")(".cc"))));
   const type& tC = tr.insert(type(C, ".c"));
   const type& tH = tr.insert(type(H, type::suffixes_t(list_of(".h")(".hpp"))));
   const type& tOBJ = tr.insert(type(OBJ, ".obj"));
   const type& tPCH = tr.insert(type(PCH, ".pch"));
   const type& tLIB = tr.insert(type(LIB, ""));
   const type& tSHARED_LIB = tr.insert(type(SHARED_LIB, ".dll", tLIB));
   const type& tSTATIC_LIB = tr.insert(type(STATIC_LIB, ".lib", tLIB));
   const type& tIMPORT_LIB = tr.insert(type(IMPORT_LIB, ".lib", tSTATIC_LIB));
   const type& tSEARCHED_LIB = tr.insert(type(SEARCHED_LIB, "", tLIB));
   const type& tHEADER_LIB = tr.insert(type(HEADER_LIB, type::suffixes_t()));
   const type& tEXE = tr.insert(type(EXE, ".exe"));
   const type& tEXE_MANIFEST = tr.insert(type(EXE_MANIFEST, ".exe.manifest"));
   const type& tDLL_MANIFEST = tr.insert(type(DLL_MANIFEST, ".dll.manifest"));
   const type& tUNKNOWN = tr.insert(type(UNKNOWN, ""));
   const type& tCOPIED = tr.insert(type(COPIED, ""));
   const type& tTESTING_OUTPUT = tr.insert(type(TESTING_OUTPUT, ".test_output"));
   const type& tTESTING_RUN_PASSED = tr.insert(type(TESTING_RUN_PASSED, ".run_passed"));
}

}}
