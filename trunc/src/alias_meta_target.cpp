#include "stdafx.h"
#include "alias_meta_target.h"
#include "main_target.h"

namespace hammer{

alias_meta_target::alias_meta_target(hammer::project* p, const pstring& name, 
                                     const sources_decl& sources,
                                     const requirements_decl& req,
                                     const requirements_decl& usage_req) 
                                     : basic_meta_target(p, name, req, usage_req)
{
   this->sources(sources);
}
   
void alias_meta_target::instantiate(const main_target* owner, 
                                    const feature_set& build_request,
                                    std::vector<basic_target*>* result, 
                                    feature_set* usage_requirements) const
{
   this->usage_requirements().eval(owner->properties(), usage_requirements);
}

void alias_meta_target::transfer_sources(sources_decl* simple_targets, 
                                         meta_targets_t* meta_targets) const
{
   split_sources(simple_targets, meta_targets);
}

}