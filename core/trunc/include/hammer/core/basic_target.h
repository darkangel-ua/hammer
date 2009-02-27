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
   class type;
   class feature_set;
   class main_target;

   class basic_target : public boost::noncopyable
   {
      public:
         typedef std::vector<boost::intrusive_ptr<build_node> > build_nodes_t;

         basic_target(const main_target* mt, const pstring& name, 
                      const type* t, const feature_set* f) 
                     : main_target_(mt), name_(name), 
                       type_(t), features_(f)
         {};

         const pstring& name() const { return name_; }
         const hammer::type& type() const { return *type_; }
         const feature_set& properties() const { return *features_; }
         void properties(const feature_set* p);
         const hammer::main_target* mtarget() const { return main_target_; }
         
         virtual build_nodes_t generate() = 0;
         const timestamp_info_t& timestamp_info(timestamp_info_t::getter_policy_t how_to_get = timestamp_info_t::just_get) const;

         virtual const location_t& location() const;

         const std::string& hash_string() const;
         static std::string hash_string(const feature_set& fs, const main_target& mt);

         virtual ~basic_target(){};
      
      protected:
         mutable timestamp_info_t timestamp_info_;

         virtual void timestamp_info_impl() const = 0;
         virtual void additional_hash_string_data(std::ostream& s) const {};

      private:
         const main_target* main_target_;
         const hammer::type* type_;
         pstring name_;
         const feature_set* features_;
         std::vector<basic_target*> dependencies_;
         mutable boost::optional<std::string> hash_;
   };
}

#endif //h_5c00dc03_f874_4599_a716_cf9f475192d0
