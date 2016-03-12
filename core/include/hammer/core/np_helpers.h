#if !defined(h_bdbdc7f5_3827_4e2d_96e0_db906cb8b72f)
#define h_bdbdc7f5_3827_4e2d_96e0_db906cb8b72f

namespace hammer
{
   class target_type;   
   class pstring;
   class pool;
   class feature_set;
   class main_target;
   class basic_target;

   pstring make_product_name(pool& p, 
                             const basic_target& source_target,
                             const target_type& product_type,
                             const feature_set& product_properties,
                             const main_target* owner);
   pstring make_product_name(pool& p, 
                             const pstring& composite_target_name, 
                             const target_type& product_type,
                             const feature_set& product_properties,
                             const main_target* owner,
                             bool primary_target);
}

#endif //h_bdbdc7f5_3827_4e2d_96e0_db906cb8b72f
