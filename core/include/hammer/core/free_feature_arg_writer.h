#if !defined(h_63891f74_7e83_4fee_82e2_ecef013ff922)
#define h_63891f74_7e83_4fee_82e2_ecef013ff922

#include "argument_writer.h"

namespace hammer
{
   class feature_set;
   class feature_def;

   class free_feature_arg_writer : public argument_writer
   {
      public:
         free_feature_arg_writer(const std::string& name, 
                                 const feature_def& def,
                                 const std::string& prefix = std::string(), 
                                 const std::string& suffix = std::string(),
                                 const std::string& delimiter = std::string(" "),
                                 const std::string& global_prefix = std::string(),
                                 const std::string& global_suffix = std::string());
         virtual free_feature_arg_writer* clone() const;

      private:
         const feature_def& feature_def_;
         std::string prefix_;
         std::string suffix_;
         std::string delimiter_;
         std::string global_prefix_;
         std::string global_suffix_;

         virtual void write_impl(std::ostream& output, const build_node& node, const build_environment& environment) const;
   };
}

#endif //h_63891f74_7e83_4fee_82e2_ecef013ff922
