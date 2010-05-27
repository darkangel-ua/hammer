#if !defined(h_19255f01_3818_46eb_9346_71f4150c468e)
#define h_19255f01_3818_46eb_9346_71f4150c468e

#include <string>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

namespace hammer{ 
   class project;
   class main_target;
   namespace project_generators{

   class eclipse_cdt_workspace;
   class eclipse_cdt_project
   {
      public:
         typedef boost::unordered_set<const main_target*> project_main_targets_t;

         eclipse_cdt_project(const eclipse_cdt_workspace& workspace,
                             const hammer::project& project);
         void construct(const project_main_targets_t& targets);
         void write() const;
      
      private:
         const eclipse_cdt_workspace& workspace_;
         const hammer::project& project_;
         
         std::string project_name_;
         std::string project_id_;
         std::string configuration_id_;
         std::string toolchain_id_;
         std::string platform_id_;
         std::string builder_id_;
         std::string holder_libs_id_;
         std::string assembler_settings_holder_id_;
         std::string assembler_holder_intype_id_;
         std::string cpp_settings_holder_id_;
         std::string cpp_holder_intype_id_;
         std::string c_settings_holder_id_;
         std::string c_holder_intype_id_;
         int cdt_id_;
         std::string links_;
         std::string includes_;

         void write_eclipse_project_file() const;
         void write_cdt_project_file() const;
         std::string make_cdt_id();
   };

}}

#endif //h_19255f01_3818_46eb_9346_71f4150c468e
