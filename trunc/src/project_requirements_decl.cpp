#include "stdafx.h"
#include "project_requirements_decl.h"

namespace hammer
{

project_requirements_decl::project_requirements_decl(const std::string& name, 
                                                     const requirements_decl& r)
                                                     : name_(name),
                                                       requirements_(r)
{

}

}