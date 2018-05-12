#include <sstream>
#include <cassert>
#include <hammer/ast/visitor.h>
#include <hammer/ast/casts.h>
#include <hammer/ast/list_of.h>
#include <hammer/ast/path.h>

namespace hammer { namespace ast {

bool path::accept(visitor& v) const
{
   return v.visit(*this);
}

std::string path::to_string() const
{
   std::stringstream s;
   if (root_name_.valid()) {
      auto r = root_name_.to_string();
      if (r == "/")
         s << '/';
      else
         s << r << ":/";
   }

   bool first = true;
   for (auto e : elements_) {
      if (first)
         first = false;
      else
         s << '/';

      if (is_a<id_expr>(e))
         s << as<id_expr>(e)->id().to_string();
      else {
         assert(as<list_of>(e));
         for (auto we : as<list_of>(e)->values())
            s << as<id_expr>(we)->id().to_string();
      }
   }

   if (trailing_slash_.valid())
      s << '/';

   return s.str();
}

bool path::has_wildcard() const
{
   for (const ast::expression* pe : elements()) {
      if (!ast::as<ast::id_expr>(pe))
         return true;
   }

   return false;
}

template<>
const path* as<path>(const node* v) { return dynamic_cast<const path*>(v); }

template<>
bool is_a<path>(const node& v) { return dynamic_cast<const path*>(&v); }

}}
