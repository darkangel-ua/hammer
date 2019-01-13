#include <hammer/ast/visitor.h>
#include <hammer/ast/path.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/target_ref.h>

namespace hammer { namespace ast {

target_ref::target_ref(parscore::source_location public_tag,
                       parscore::source_location project_local_ref_tag,
                       const path* head,
                       const parscore::identifier& target_name,
                       const features_t& build_request)
   : public_tag_(public_tag),
     project_local_ref_tag_(project_local_ref_tag),
     target_path_(head),
     target_name_(target_name),
     build_request_(build_request)
{}

bool target_ref::accept(visitor& v) const
{
   return v.visit(*this);
}

parscore::source_location target_ref::start_loc() const
{
   return public_tag_.valid() ? public_tag_ : project_local_ref_tag_.valid() ? project_local_ref_tag_ : target_path_->start_loc();
}

bool target_ref::has_target_name() const
{
   return target_name_.valid();
}

template<>
bool is_a<target_ref>(const node& v) { return dynamic_cast<const target_ref*>(&v); }

template<>
const target_ref* as<target_ref>(const node* v) { return dynamic_cast<const target_ref*>(v); }

}}

