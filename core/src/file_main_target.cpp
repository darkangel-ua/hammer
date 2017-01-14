#include "stdafx.h"
#include <hammer/core/file_main_target.h>
#include <hammer/core/file_target.h>

namespace hammer{

file_main_target::file_main_target(const basic_meta_target* mt,
                                   const pstring& name, 
                                   const pstring& filename,
                                   const feature_set* props,
                                   const target_type& t,
                                   pool& p)
   : 
    main_target(mt, name, &t, props, p),
    type_(&t),
    filename_(filename)
{

}

std::vector<boost::intrusive_ptr<hammer::build_node> > 
file_main_target::generate()
{
   boost::intrusive_ptr<hammer::build_node> result(new hammer::build_node(*this, true));
   basic_target* t = new file_target(this, filename_, type_, &properties());
   result->products_.push_back(t);
   result->targeting_type_ = this->type_;
   generate_and_add_dependencies(*result);

   return std::vector<boost::intrusive_ptr<hammer::build_node> >(1, result);
}

}

