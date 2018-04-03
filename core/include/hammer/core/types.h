#pragma once

namespace hammer {

class type_registry;
class feature_registry;
class type_tag;

}

namespace hammer { namespace types {

extern const type_tag CPP;
extern const type_tag C;
extern const type_tag H;
extern const type_tag RC;
extern const type_tag RES;
extern const type_tag OBJ;
extern const type_tag PCH;
extern const type_tag LIB;
extern const type_tag SHARED_LIB;
extern const type_tag PREBUILT_SHARED_LIB;
extern const type_tag IMPORT_LIB;
extern const type_tag STATIC_LIB;
extern const type_tag PREBUILT_STATIC_LIB;
extern const type_tag SEARCHED_LIB;
extern const type_tag SEARCHED_SHARED_LIB;
extern const type_tag SEARCHED_STATIC_LIB;
extern const type_tag HEADER_LIB;
extern const type_tag EXE;
extern const type_tag UNKNOWN;
extern const type_tag COPY;
extern const type_tag TESTING_OUTPUT;
extern const type_tag TESTING_RUN_PASSED;
extern const type_tag TESTING_COMPILE_FAIL;

void register_standart_types(type_registry& tr,
                             feature_registry& fr);

}}

