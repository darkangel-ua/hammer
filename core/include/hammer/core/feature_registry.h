#if !defined(h_2f2882b2_0d61_431a_ac19_94de8add9953)
#define h_2f2882b2_0d61_431a_ac19_94de8add9953

#include <map>
#include <hammer/core/feature_def.h>
#include <hammer/core/feature_value_ns_fwd.h>

namespace hammer
{
   class feature_set;
   class feature;
   class pool;

   class feature_registry
   {
      public:
         struct impl_t;
         feature_registry();

         feature_def&
         add_feature_def(const std::string& name,
                         const feature_def::legal_values_t& legal_values = feature_def::legal_values_t(),
                         feature_attributes attributes = feature_attributes());
         feature_def& get_def(const std::string& name);

         const feature_def*
         find_def(const std::string& name) const;

         feature_set* make_set();
         const feature_set& singleton() const;
         feature* create_feature(const std::string& name, const std::string& value);
         feature* create_feature(const feature& f, 
                                 const std::string& subfeature_name, 
                                 const std::string& subfeature_value);
         feature* clone_feature(const feature& f);

			const feature_value_ns_ptr&
			get_or_create_feature_value_ns(const std::string& ns);

         feature_set&
			add_defaults(feature_set& s);
         const feature_def* find_def(const char* feature_name) const;
         // find definition for complex names such as os-version where version is subfeature
         const feature_def* find_def_from_full_name(const char* feature_name) const;
         ~feature_registry();

      private:
         mutable impl_t* impl_;

         feature* simply_create_feature(const std::string& name, const std::string& value);
   };
}

#endif //h_2f2882b2_0d61_431a_ac19_94de8add9953
