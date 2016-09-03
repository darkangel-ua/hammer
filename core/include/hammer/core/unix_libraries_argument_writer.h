#if !defined(h_815c3d5f_47e3_4a08_9f67_7d96b899d1b3)
#define h_815c3d5f_47e3_4a08_9f67_7d96b899d1b3

#include <hammer/core/argument_writer.h>
#include <hammer/core/linker_type.h>

namespace hammer
{
   class target_type;
   class unix_libraries_argument_writer : public argument_writer
   {
      public:
         unix_libraries_argument_writer(const std::string& name,
                                        linker_type::value linker,
                                        engine& e);
         virtual unix_libraries_argument_writer* clone() const;

      private:
         linker_type::value linker_type_;
         const target_type* shared_lib_type_;
         const target_type* static_lib_type_;
         const target_type* searched_shared_lib_type_;
         const target_type* searched_static_lib_type_;
         const target_type* import_lib_type_;

         virtual void write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const;
   };
}

#endif //h_815c3d5f_47e3_4a08_9f67_7d96b899d1b3