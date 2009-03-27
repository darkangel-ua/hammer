#if !defined(h_bdbdc7f5_3827_4e2d_96e0_db906cb8b72f)
#define h_bdbdc7f5_3827_4e2d_96e0_db906cb8b72f

namespace hammer
{
   class target_type;   
   class pstring;
   class pool;
   class feature_set;
   class main_target;

   pstring make_name(pool& p, 
                     const pstring& source_name, 
                     const target_type& source_type, 
                     const target_type& target_type,
                     const feature_set* target_properties,
                     const main_target* owner);
   pstring make_name(pool& p, 
                     const pstring& source_name, 
                     const target_type& target_type,
                     const feature_set* target_properties,
                     const main_target* owner);
   pstring cut_suffix(pool& p, const pstring& source_name,
                      const target_type& source_type);
}

#endif //h_bdbdc7f5_3827_4e2d_96e0_db906cb8b72f
