#include <boost/bind.hpp>
#include <hammer/core/htmpl/htmpl.h>
#include <hammer/core/engine.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/target_type.h>
#include <hammer/core/sources_decl.h>
#include "htmpl_generator.h"
#include "htmpl_meta_target.h"

using namespace std;

namespace hammer{ namespace types {
   const type_tag HTMPL("HTMPL");
}}

namespace hammer {

static
sources_decl htmpl_rule(project& p,
                        sources_decl& src)
{
   sources_decl result;

   for (const source_decl& sd : src) {
      const string target_name = "#unnamed::htmpl." + sd.target_path();
      auto_ptr<basic_meta_target> mt(new htmpl_meta_target(&p, target_name, sd));
      p.add_target(mt);

      result.push_back(source_decl(target_name, std::string(), nullptr, nullptr));
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

   e.call_resolver().insert("htmpl", boost::function<sources_decl(project&, sources_decl&)>(boost::bind(&htmpl_rule, _1, _2)));
}

}

