#if !defined(h_ab2aca5e_e63f_423c_b6da_5ac29570cf00)
#define h_ab2aca5e_e63f_423c_b6da_5ac29570cf00

#include "targets_argument_writer.h"

namespace hammer
{
   class basic_target;

   class source_argument_writer : public targets_argument_writer
   {
      public:
         source_argument_writer(const std::string& name, 
                                const target_type& t,
                                bool exact_type = true,
                                bool write_full_path = false,
                                const std::string& quoting_string = "\"",
                                const std::string& prefix = std::string());
         virtual argument_writer* clone() const;

      protected:
         virtual void write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const;
         
         // return true if source type is accepted for output
         virtual bool accept(const basic_target& source) const;

      private:
         bool exact_type_;
         bool write_full_path_;
         std::string quoting_string_;
         std::string prefix_;
   };
}

#endif //h_ab2aca5e_e63f_423c_b6da_5ac29570cf00
