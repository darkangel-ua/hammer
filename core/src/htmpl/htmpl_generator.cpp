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
      product_stream->write((**i).value().begin(), (**i).value().size());
      content_p = &*(*first)[0].first + (*first)[0].length();
   }

   product_stream->write(content_p, distance(content_p, &content.back()));

   return true;
}

}

htmpl_generator::htmpl_generator(engine& e,
                                 const string& name,
                                 const type_tag& product_type)
   : generator(e, name, make_consume_types(e, {HTMPL}), make_product_types(e, {product_type}), true)
{
   action(std::unique_ptr<build_action>(new htmpl_action(*consumable_types().front().type_)));
}

}
