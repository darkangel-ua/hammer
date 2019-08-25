#include <sstream>
#include <assert.h>
#include <boost/algorithm/string/join.hpp>
#include <hammer/core/build_request.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>

namespace hammer {

build_request::build_request(const feature_registry& fr)
   : resolved_features_(fr.make_set())
{
}

build_request::build_request(const feature_set& resolved_features)
   : resolved_features_(resolved_features.clone())
{
}

build_request
build_request::resolve(const feature_registry& context) const
{
   assert(!resolved_completely());

   build_request result{resolved_features_->owner()};

   for (auto& f: unresolved_features_) {
      if (context.find_def(f.name_))
         result.join(context.create_feature(f.name_, f.value_));
      else
         result.join_unresolved(f.name_, f.value_);
   }

   return result;
}

void build_request::join(const feature_set& props) {
   resolved_features_->join(props);
}

void build_request::join(feature_ref f) {
   resolved_features_->join(f);
}

void build_request::join_unresolved(std::string name,
                                    std::string value) {
   unresolved_features_.push_back({std::move(name), std::move(value)});
}

std::string
build_request::string(const bool full) const {
   std::stringstream s;
   if (resolved_completely())
      dump_for_hash(s, *resolved_features_, full);
   else {
      std::vector<std::string> unresolved_features;
      for (auto& f : unresolved_features_)
         unresolved_features.push_back("<" + f.name_ + ">" + f.value_);

      s << "unresolved : " << boost::join(unresolved_features, ", ") << ", resolved : ";
      dump_for_hash(s, *resolved_features_, full);
   }
   return s.str();
}

build_request::~build_request() = default;

}
