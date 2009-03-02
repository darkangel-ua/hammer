#pragma once

#include "main_target.h"

namespace hammer
{
   class pch_main_target : public main_target
   {
      public:
         pch_main_target(const hammer::meta_target* mt,
                         const main_target& owner,
                         const pstring& name, 
                         const hammer::type* t, 
                         const feature_set* props,
                         pool& p);

         const basic_target& pch_header() const { return *pch_header_; }
         const basic_target& pch_source() const { return *pch_source_; }
         const basic_target& pch_product() const { return *pch_product_; }
         virtual std::vector<boost::intrusive_ptr<hammer::build_node> > generate();
      
      protected:
         virtual location_t intermediate_dir_impl() const;

      private:
         const main_target& owner_;
         const basic_target* pch_header_;
         const basic_target* pch_source_;
         const basic_target* pch_product_;
   };
}