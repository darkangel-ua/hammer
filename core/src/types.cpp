#include <vector>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/feature_set.h>

using namespace std;

namespace hammer{ namespace types{

const type_tag CPP("CPP");
const type_tag C("C");
const type_tag C_AS_CPP("C_AS_CPP");
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
const type_tag UNKNOWN("UNKNOWN");
const type_tag COPY("COPIED");
const type_tag TESTING_OUTPUT("TESTING_OUTPUT");
const type_tag TESTING_RUN("TESTING_RUN_PASSED");
const type_tag TESTING_PASSED("TESTING_PASSED");
const type_tag TESTING_RUN_OUTPUT("TESTING_RUN_OUTPUT");
const type_tag TESTING_COMPILE_FAIL("TESTING_COMPILE_FAIL");
const type_tag TESTING_COMPILE_SUCCESSFUL("TESTING_COMPILE_SUCCESSFUL");
const type_tag TESTING_LINK_FAIL("TESTING_LINK_FAIL");
const type_tag TESTING_LINK_SUCCESSFUL("TESTING_LINK_SUCCESSFUL");
const type_tag TESTING_SUITE("TESTING_SUITE");

void register_standart_types(type_registry& tr,
                             feature_registry& fr)
{
   target_type::suffixes_t cpp_suffixes = {".cpp", ".cc"};
   const target_type& cpp_type = tr.insert(target_type(CPP, cpp_suffixes));

   tr.insert(target_type(C, ".c"));
   tr.insert(target_type(C_AS_CPP, ".c", cpp_type));

   target_type::suffixes_t h_suffixes = {".h", ".hpp", ".H"};
   tr.insert(target_type(H, h_suffixes));

   tr.insert(target_type(RC, ".rc"));
   tr.insert(target_type(RES, ".res"));

   target_type::suffixes_t obj_suffixes = {
      { ".o", *parse_simple_set("<target-os>linux", fr)},
      {".obj", *parse_simple_set("<target-os>windows", fr)}
   };

   tr.insert(target_type(OBJ, obj_suffixes));
   tr.insert(target_type(PCH, ".pch"));
   const target_type& tLIB = tr.insert(target_type(LIB, ""));

   target_type::suffixes_t shared_suffixes = {
      {".dll", *parse_simple_set("<target-os>windows", fr)},
      {".so", *parse_simple_set("<target-os>linux", fr)}
   };

   const target_type& tSHARED_LIB = tr.insert(target_type(SHARED_LIB, shared_suffixes, tLIB));
   tr.insert(target_type(PREBUILT_SHARED_LIB, "", tSHARED_LIB));
   target_type::suffixes_t static_lib_suffixes = {
      {".lib", *parse_simple_set("<target-os>windows", fr)},
      {".a", *parse_simple_set("<target-os>linux", fr)}
   };

   const target_type& tSTATIC_LIB = tr.insert(target_type(STATIC_LIB, static_lib_suffixes, tLIB));
   tr.insert(target_type(PREBUILT_STATIC_LIB, "", tSTATIC_LIB));

   tr.insert(target_type(IMPORT_LIB, static_lib_suffixes, tSTATIC_LIB));
   const target_type& tSEARCHED_LIB = tr.insert(target_type(SEARCHED_LIB, "", tLIB));
   tr.insert(target_type(SEARCHED_SHARED_LIB, "", tSEARCHED_LIB));
   tr.insert(target_type(SEARCHED_STATIC_LIB, "", tSEARCHED_LIB));
   tr.insert(target_type(HEADER_LIB, target_type::suffixes_t(), tLIB));


   target_type::suffixes_t exe_suffixes = {
      {".exe", *parse_simple_set("<target-os>windows", fr)},
      {"", *parse_simple_set("<target-os>linux", fr)}
   };

   tr.insert(target_type(EXE, exe_suffixes));
   tr.insert(target_type(UNKNOWN, ""));
   tr.insert(target_type(COPY, ""));
   tr.insert(target_type(TESTING_PASSED, ".passed"));
   tr.insert(target_type(TESTING_SUITE, string{}));

   const target_type& testing_output_base = tr.insert(target_type(TESTING_OUTPUT, ".output"));
   tr.insert(target_type(TESTING_RUN, ".output", testing_output_base));
   tr.insert(target_type(TESTING_COMPILE_FAIL, ".output", testing_output_base));
   tr.insert(target_type(TESTING_COMPILE_SUCCESSFUL, ".output", testing_output_base));
   tr.insert(target_type(TESTING_LINK_FAIL, ".output", testing_output_base));
   tr.insert(target_type(TESTING_LINK_SUCCESSFUL, ".output", testing_output_base));
}

}}
