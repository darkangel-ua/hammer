#include "htmpl_generator.h"
#include <hammer/core/types.h>
#include <hammer/core/type_tag.h>
#include <hammer/core/htmpl/htmpl.h>
#include <hammer/core/build_action.h>
#include <hammer/core/source_argument_writer.h>
#include <hammer/core/product_argument_writer.h>
#include <hammer/core/build_environment.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/feature_def.h>
#include <hammer/core/engine.h>
#include <hammer/core/generated_build_target.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <regex>

namespace fs = boost::filesystem;
using namespace std;

namespace hammer {

using namespace types;

namespace {

class htmpl_action : public build_action
{
   public:
      htmpl_action(const target_type& source_target_type)
         : build_action("htmpl"),
           source_target_type_(source_target_type)
      { }

      string target_tag(const build_node& node,
                        const build_environment& environment) const override;
      std::vector<const feature*> valuable_features() const override { return {}; }

   protected:
      bool execute_impl(const build_node& node,
                        const build_environment& environment) const override;
   private:
      const target_type& source_target_type_;
};

string
htmpl_action::target_tag(const build_node& node,
                         const build_environment& environment) const
{
   return "test";
}

bool
htmpl_action::execute_impl(const build_node& node,
                           const build_environment& environment) const
{
   const fs::path source_file = [&] {
      source_argument_writer saw({}, source_target_type_, true, source_argument_writer::FULL_PATH, "");
      stringstream s;
      saw.write(s, node, environment);

      return fs::path(s.str());
   }();

   const fs::path product_file = [&] {
      product_argument_writer paw({}, *node.targeting_type_, product_argument_writer::output_strategy::FULL_PATH);
      stringstream s;
      paw.write(s, node, environment);

      return fs::path(s.str());
   }();

   const regex htmpl_pattern("(%%<(.+?)>%%)");
   fs::ifstream source_stream(source_file);
   unique_ptr<std::ostream> product_stream = environment.create_output_file(product_file.string().c_str(), ios_base::trunc);

   const vector<char> content{istreambuf_iterator<char>(source_stream), istreambuf_iterator<char>()};

   if (content.empty())
      return true;

   const feature_set& props = node.products_owner().properties();
   const char* content_p = &content[0];
   for (regex_iterator<vector<char>::const_iterator> first(content.cbegin(), content.cend(), htmpl_pattern), last = {}; first != last; ++first) {
      const string feature_name = (*first)[2];
      auto i = props.find(feature_name);
      if (i == props.end()) {
         environment.error_stream() << "[htmpl] Can't find feature '" << feature_name
                                    << "' for instantiated meta target located at '" << node.products_owner().get_meta_target()->location().string() << "'.\n"
                                    << "Source: '" << source_file.string() << "'";
         return false;
      }

      const feature_def& def = (**i).definition();
      if (def.attributes().free) {
         environment.error_stream() << "[htmpl] Free feature not supported. Feature '"
                                    << "' for instantiated meta target located at '" << node.products_owner().get_meta_target()->location().string() << "'.\n"
                                    << "Source: '" << source_file.string() << "'";
         return false;
      }

      product_stream->write(content_p, distance(content_p, &*(*first)[0].first));
      product_stream->write((**i).value().c_str(), (**i).value().size());
      content_p = &*(*first)[0].first + (*first)[0].length();
   }

   product_stream->write(content_p, distance(content_p, &content.back()));

   return true;
}

}

htmpl_generator::htmpl_generator(engine& e,
                                 const string& name,
                                 const type_tag& product_type)
   : generator(e,
               name,
               make_consume_types(e, {HTMPL}),
               make_product_types(e, {product_type}),
               true,
               std::make_shared<htmpl_action>(e.get_type_registry().get(HTMPL)))
{
}

basic_build_target*
htmpl_generator::create_target(const main_target* mt,
                               const build_node::sources_t& sources,
                               const std::string& target_name,
                               const target_type* t,
                               const feature_set* f) const
{
   // because we use name with extension for main target generator::generate will add extra suffix for output type that we need to remove
   const std::string new_target_name = fs::path(target_name).stem().string();
   return new generated_build_target(mt, new_target_name, t, f);
}

}
