#if !defined(h_5c00dc03_f874_4599_a716_cf9f475192d0)
#define h_5c00dc03_f874_4599_a716_cf9f475192d0

#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include "build_node.h"
#include "timestamp_info.h"
#include "location.h"

namespace hammer
{
   class target_type;
   class feature_set;
   class main_target;
   class basic_meta_target;
   class engine;
   class project;
   class build_environment;

   class basic_target : public boost::noncopyable
   {
      public:
         typedef std::vector<boost::intrusive_ptr<build_node> > build_nodes_t;

			basic_target(const main_target* mt,
			             const std::string& name,
			             const target_type* t,
			             const feature_set* f)
			   : main_target_(mt),
			     type_(t),
			     name_(name),
			     features_(f)
			{}

         const std::string& name() const { return name_; }
         const target_type& type() const { return *type_; }
         const feature_set& properties() const { return *features_; }
         void properties(const feature_set* p);
         const main_target* get_main_target() const { return main_target_; }
         const basic_meta_target* get_meta_target() const;
         const project* get_project() const;
         engine& get_engine() const;
         
         virtual build_nodes_t generate() const = 0;
         virtual const location_t& location() const;
         location_t full_path() const;

         const std::string& hash_string() const;
         static std::string hash_string(const feature_set& fs, const main_target& mt);

         virtual ~basic_target(){}
      
      protected:
         virtual void additional_hash_string_data(std::ostream& s) const {}

      private:
         const main_target* main_target_;
         const target_type* type_;
         const std::string name_;
         const feature_set* features_;
         std::vector<basic_target*> dependencies_;
         mutable boost::optional<std::string> hash_;
   };
}

#endif //h_5c00dc03_f874_4599_a716_cf9f475192d0
