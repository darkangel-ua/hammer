#include <boost/regex.hpp>
#include <hammer/core/feature.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include "build_request.h"

using namespace std;
using namespace hammer;

namespace {

feature*
try_resolve_implicit_feature(feature_registry& fr,
                             feature_def& fd,
                             const std::string& value) {
   string::size_type p = value.find('-');
   if (p != string::npos) {
      string main_feature(value.substr(0, p));
      string subfeature(value.substr(p + 1, value.size() - p - 1));
      if (!fd.is_legal_value(main_feature))
         return nullptr;
      if (!fd.find_subfeature_for_value(main_feature, subfeature))
         return nullptr;
   } else {
      if (!fd.is_legal_value(value))
         return nullptr;
   }

   return fr.create_feature(fd.name(), value);
}

feature*
try_resolve_implicit_feature(feature_registry& fr,
                             const std::string& value) {
   feature* result = try_resolve_implicit_feature(fr, fr.get_def("toolset"), value);
   if (result)
      return result;
   else
      return try_resolve_implicit_feature(fr, fr.get_def("variant"), value);
}

bool is_looks_like_project(const string& s)
{
   return s.find('/') != string::npos;
}

void split_target_path(string& target_path,
                       string& target_name,
                       const string& to_split)
{
   boost::smatch match;
   boost::regex pattern("(.+?)//(.+)");
   if (boost::regex_match(to_split, match, pattern)) {
      target_path = match[1];
      target_name = match[2];
   } else
      target_path = to_split;
}

struct resolved_targets {
   vector<const basic_meta_target*> targets_;
   vector<string> unresolved_target_names_;
};

resolved_targets
resolve_target_names(hammer::engine& e,
                     const hammer::project* project,
                     const vector<string>& target_names,
                     const feature_set& build_request)
{
   using selected_targets_t = hammer::project::selected_targets_t;

   resolved_targets result;

   for (auto& target_name : target_names) {
      if (is_looks_like_project(target_name)) {
         string target_path, target_name;
         split_target_path(target_path, target_name, target_name);

         if (target_name[0] != '/' && !project) {
            result.unresolved_target_names_.push_back(target_name);
            continue;
         }

         const hammer::loaded_projects p =
            target_name[0] == '/' ? e.load_project(engine::global_project_ref{target_path})
                                  : project->load_project(target_path);
         selected_targets_t st =
            target_name.empty() ? p.select_best_alternative(build_request)
                                : selected_targets_t{1, p.select_best_alternative(target_name, build_request, false)};
         feature_set* usage_requirements = project->get_engine().feature_registry().make_set();
         for (selected_targets_t::const_iterator t = st.begin(), t_last = st.end(); t != t_last; ++t) {
            result.targets_.push_back(t->target_);
            usage_requirements->clear();
         }
      } else {
         if (project) {
            hammer::project::selected_target target = project->select_best_alternative(target_name, build_request);
            result.targets_.push_back(target.target_);
         } else
            result.unresolved_target_names_.push_back(target_name);
      }
   }

   return result;
}

}

build_request
resolve_build_request(hammer::engine& e,
                      const vector<string>& build_request_args,
                      const project* current_project) {
   feature_registry& fr = e.feature_registry();

   build_request result;
   result.build_request_ = e.feature_registry().make_set();
   for (const string& arg : build_request_args) {
      auto p = arg.find('=');
      if (p == string::npos) {
         feature* posible_implicit_feature = try_resolve_implicit_feature(fr, arg);
         if (posible_implicit_feature)
            result.build_request_->join(posible_implicit_feature);
         else
            result.target_names_.push_back(arg);
      } else {
         string feature_name(arg.begin(), arg.begin() + p);
         result.build_request_->join(feature_name.c_str(), arg.c_str() + p + 1);
      }
   }

   // lets handle 'toolset' feature in build request
#if defined(_WIN32)
   const string default_toolset_name = "msvc";
#else
   const string default_toolset_name = "gcc";
#endif
   auto i_toolset_in_build_request = result.build_request_->find("toolset");
   if (i_toolset_in_build_request == result.build_request_->end()) {
      const feature_def& toolset_definition = fr.get_def("toolset");
      if (!toolset_definition.is_legal_value(default_toolset_name))
         throw std::runtime_error("Default toolset is set to '"+ default_toolset_name + "', but either you didn't configure it in user-config.ham or it has failed to autoconfigure");

      const subfeature_def& toolset_version_def = toolset_definition.get_subfeature("version");
      // peek first configured as default
      const string& default_toolset_version = *toolset_version_def.legal_values(default_toolset_name).begin();
      result.build_request_->join("toolset", (default_toolset_name + "-" + default_toolset_version).c_str());
   } else {
      const feature& used_toolset = **i_toolset_in_build_request;
      if (!used_toolset.find_subfeature("version")) {
         const subfeature_def& toolset_version_def = used_toolset.definition().get_subfeature("version");
         const string& toolset = used_toolset.value();
         // peek first configured as default
         const string& default_toolset_version = *toolset_version_def.legal_values(toolset).begin();
         result.build_request_->erase_all("toolset");
         result.build_request_->join("toolset", (toolset + "-" + default_toolset_version).c_str());
      }
   }

   if (result.build_request_->find("variant") == result.build_request_->end())
      result.build_request_->join("variant", "debug");

   if (result.build_request_->find("host-os") == result.build_request_->end())
      result.build_request_->join("host-os", fr.get_def("host-os").get_defaults().front().value_.c_str());

   if (result.build_request_->find("target-os") == result.build_request_->end())
      result.build_request_->join("target-os", fr.get_def("target-os").get_defaults().front().value_.c_str());

   auto resolved_targets = resolve_target_names(e, current_project, result.target_names_, *result.build_request_);
   result.targets_ = move(resolved_targets.targets_);
   result.unresolved_target_names_ = move(resolved_targets.unresolved_target_names_);

   return result;
}
