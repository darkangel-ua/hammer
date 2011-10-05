#if !defined(h_5c00dc03_f874_4599_a716_cf9f475192d0)
#define h_5c00dc03_f874_4599_a716_cf9f475192d0

#include <boost/noncopyable.hpp>
#include "pool.h"
#include "pstring.h"
#include "build_node.h"
#include "timestamp_info.h"
#include "location.h"
#include <boost/optional.hpp>

namespace hammer
{
   class target_type;
   class feature_set;
   class main_target;
   class meta_target;
   class engine;
   class project;
   class build_environment;

   class basic_target : public boost::noncopyable
   {
      public:
         typedef std::vector<boost::intrusive_ptr<build_node> > build_nodes_t;

         basic_target(const main_target* mt, const pstring& name, 
                      const target_type* t, const feature_set* f) 
                     : main_target_(mt),  
                       type_(t), 
                       name_(name),
                       features_(f)
         {};

         const pstring& name() const { return name_; }
         const target_type& type() const { return *type_; }
         const feature_set& properties() const { return *features_; }
         void properties(const feature_set* p);
         const main_target* get_main_target() const { return main_target_; }
         const meta_target* get_meta_target() const;
         const project* get_project() const;
         engine* get_engine() const;
         
         virtual build_nodes_t generate() = 0;
         // maybe this is wrong? The reason to making it const was cleaner implementation
         virtual void clean(const build_environment& environment) const {};

         const timestamp_info_t& timestamp_info() const;

         virtual const location_t& location() const;
         location_t full_path() const;

         const std::string& hash_string() const;
         static std::string hash_string(const feature_set& fs, const main_target& mt);

         virtual ~basic_target(){};
      
      protected:
         mutable timestamp_info_t timestamp_info_;

         virtual void timestamp_info_impl() const = 0;
         virtual void additional_hash_string_data(std::ostream& s) const {};

      private:
         const main_target* main_target_;
         const target_type* type_;
         pstring name_;
         const feature_set* features_;
         std::vector<basic_target*> dependencies_;
         mutable boost::optional<std::string> hash_;
   };
}

#endif //h_5c00dc03_f874_4599_a716_cf9f475192d0
