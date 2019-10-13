#include <fstream>
#include <boost/filesystem/operations.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <hammer/core/feature_set.h>
#include <hammer/core/main_target.h>
#include <hammer/core/engine.h>
#include <hammer/core/type_registry.h>
#include <hammer/core/types.h>
#include "signature_build_target.h"

namespace hammer {

static
std::string make_content(const build_node& node) {
   std::ostringstream s;
   s << "build request:\n\n" << dump_for_hash(node.build_request(), true) << "\n\n"
     << "sources:\n\n";

   // stabilize sources
   std::vector<std::string> sources;
   for (auto& source: node.sources_)
      sources.push_back((source.source_target_->location() / source.source_target_->name()).string());

   sort(sources.begin(), sources.end());

   for (auto& source : sources)
      s << source << '\n';

   return s.str();
}

static
std::string make_content(const feature_set& build_request) {
   return "build request:\n\n" + dump_for_hash(build_request, true);
}

signature_build_target::signature_build_target(const main_target* mt,
                                               const build_node& node)
   : generated_build_target(mt,
                            node.products_.front()->name() + ".sig",
                            node.products_.front()->hash(),
                            &mt->get_engine().get_type_registry().get(types::UNKNOWN),
                            &node.build_request(),
                            &node.products_.front()->location()),
     content_(make_content(node))
{

}

signature_build_target::signature_build_target(const main_target* mt,
                                               const feature_set& build_request,
                                               const location_t& l)
   : generated_build_target(mt,
                            "properties.sig",
                            {},
                            &mt->get_engine().get_type_registry().get(types::UNKNOWN),
                            &build_request,
                            &l),
     content_(make_content(build_request))
{

}

void signature_build_target::timestamp_info_impl() const
{
   timestamp_info_.is_unknown_ = false;

   location_t l = location() / name();
   l.normalize();
   
   std::ifstream f(l.string().c_str());
   
   if (!f)
      timestamp_info_.timestamp_ = boost::date_time::neg_infin;
   else {
      std::string saved_signature;
      std::copy(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>(), back_inserter(saved_signature));
      if (!f)
         timestamp_info_.timestamp_ = boost::date_time::neg_infin;
      else
         if (content_ != saved_signature)
            timestamp_info_.timestamp_ = boost::date_time::neg_infin;
         else
            timestamp_info_.timestamp_ = boost::posix_time::from_time_t(last_write_time(l));
   }
}

}
