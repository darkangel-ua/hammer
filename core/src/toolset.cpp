#include <hammer/core/toolset.h>

namespace hammer {

toolset::toolset(const std::string& name,
                 const rule_declaration& use_rule)
   : name_(name),
     use_rule_(use_rule)
{

}

toolset::~toolset()
{

}

}
