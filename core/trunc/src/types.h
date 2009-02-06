#if !defined(h_883d3846_ffd7_4a31_840e_13148216c79b)
#define h_883d3846_ffd7_4a31_840e_13148216c79b

namespace hammer
{
   class type_registry;
   class type_tag;

   namespace types
   {
      extern const type_tag CPP;
      extern const type_tag C;
      extern const type_tag H;
      extern const type_tag OBJ;
      extern const type_tag PCH;
      extern const type_tag SHARED_LIB;
      extern const type_tag IMPORT_LIB;
      extern const type_tag STATIC_LIB;
      extern const type_tag SEARCHED_LIB;
      extern const type_tag HEADER_LIB;
      extern const type_tag EXE;
      extern const type_tag EXE_MANIFEST;
      extern const type_tag DLL_MANIFEST;
      extern const type_tag UNKNOWN;
      extern const type_tag COPIED;
      extern const type_tag COPIED_TAG;
      extern const type_tag TESTING_OUTPUT;
      extern const type_tag TESTING_RUN_PASSED;

      void register_standart_types(type_registry& tr);
   }
}

#endif //h_883d3846_ffd7_4a31_840e_13148216c79b
