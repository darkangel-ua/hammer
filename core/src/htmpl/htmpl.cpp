#include <boost/bind.hpp>
#include <boost/make_unique.hpp>
#include <hammer/core/htmpl/htmpl.h>
#include <hammer/core/engine.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/target_type.h>
#include <hammer/core/sources_decl.h>
#include <hammer/core/rule_argument_types.h>
#include "htmpl_generator.h"
#include "htmpl_meta_target.h"

using namespace std;

namespace hammer{ namespace types {
   const type_tag HTMPL("HTMPL");
}}

namespace hammer {

static
unique_ptr<sources_decl>
htmpl_rule(invocation_context& ctx,
           const sources_decl& sources)
{
   auto result = boost::make_unique<sources_decl>();

   for (const source_decl& sd : sources) {
      const string target_name = "#unnamed::htmpl." + sd.target_path();
      ctx.current_project_.add_target(boost::make_unique<htmpl_meta_target>(&ctx.current_project_, target_name, sd));

      result->push_back(source_decl{ctx.current_project_, "./", target_name, nullptr, nullptr});
   }

   return result;
}

void install_htmpl(engine& e)
{
   e.get_type_registry().insert(target_type(types::HTMPL, "htmpl"));

   unique_ptr<generator> g_h(new htmpl_generator(e, "htmpl.h", types::H));
   e.generators().insert(move(g_h));

   unique_ptr<generator> g_cpp(new htmpl_generator(e, "htmpl.cpp", types::CPP));
   e.generators().insert(move(g_cpp));

   e.get_rule_manager().add_rule("htmpl", htmpl_rule, {"sources"});
}

}

