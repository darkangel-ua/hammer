#ifndef HAMMER_CORE_HTMPL_HTMPL_H
#define HAMMER_CORE_HTMPL_HTMPL_H

#include <hammer/core/types.h>

namespace hammer {

class engine;
class type_tag;

void install_htmpl(engine& e);

}

namespace hammer{ namespace types {
   extern const type_tag HTMPL;
}}

#endif
