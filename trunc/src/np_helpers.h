#if !defined(h_bdbdc7f5_3827_4e2d_96e0_db906cb8b72f)
#define h_bdbdc7f5_3827_4e2d_96e0_db906cb8b72f

namespace hammer
{
   class type;   
   class pstring;
   class pool;
   class feature_set;

   pstring make_name(pool& p, 
                     const pstring& source_name, 
                     const type& source_type, 
                     const type& target_type,
                     const feature_set* target_properties);
   pstring make_name(pool& p, 
                     const pstring& source_name, 
                     const type& target_type,
                     const feature_set* target_properties);
   pstring cut_suffix(pool& p, const pstring& source_name,
                      const type& source_type);
}

#endif //h_bdbdc7f5_3827_4e2d_96e0_db906cb8b72f
