#include <algorithm>
#include <boost/unordered_set.hpp>
#include <boost/bind.hpp>
#include <boost/make_unique.hpp>
#include <hammer/core/warehouse.h>
#include <hammer/core/warehouse_target.h>
#include <hammer/core/warehouse_meta_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/rule_argument_types.h>
#include <hammer/core/warehouse_manager.h>

using namespace std;
using boost::unordered_set;

namespace hammer {

const std::string warehouse::any_version;

warehouse::warehouse(const std::string& id,
                     const boost::filesystem::path& storage_dir)
   : id_(id),
     storage_dir_(storage_dir)
{}

static
void walk_over_targets(vector<const warehouse_target*>& result,
                       unordered_set<basic_target*>& visited,
                       const vector<basic_target*>& targets)
{
   for(vector<basic_target*>::const_iterator i = targets.begin(), last = targets.end(); i != last; ++i) {
      if (warehouse_target* t = dynamic_cast<warehouse_target*>(*i))
         result.push_back(t);
      else if (main_target* m = dynamic_cast<main_target*>(*i)) {
         if (visited.find(m) == visited.end()) {
            visited.insert(m);
            walk_over_targets(result, visited, m->sources());
            walk_over_targets(result, visited, m->dependencies());
         }
      }
   }
}

vector<const warehouse_target*>
find_all_warehouse_unresolved_targets(const vector<basic_target*>& targets)
{
   vector<const warehouse_target*> result;
   unordered_set<basic_target*> visited;

   walk_over_targets(result, visited, targets);

   return result;
}

void add_traps(warehouse& wh,
               project& p,
               const std::string& public_id,
               const std::string& internal_path) {
   warehouse::versions_t all_versions = wh.get_package_versions(public_id);
   vector<string> installed_versions = wh.get_installed_versions(public_id);

   warehouse::versions_t not_installed_versions;

   sort(all_versions.begin(), all_versions.end(),
        [] (const warehouse::version_info& lhs, const warehouse::version_info& rhs)
      {
         return lhs.version_ < rhs.version_;
      });

   struct pred {
      bool operator()(const warehouse::version_info& lhs, const string& rhs) const { return lhs.version_ < rhs; }
      bool operator()(const string& lhs, const warehouse::version_info& rhs) const { return lhs < rhs.version_; }
      // These two need by msvc debug implementation
      bool operator()(const warehouse::version_info& lhs, const warehouse::version_info& rhs) const { return lhs.version_ < rhs.version_; }
      bool operator()(const string& lhs, const string& rhs) const { return lhs < rhs; }
   };

   set_difference(all_versions.begin(), all_versions.end(),
                  installed_versions.begin(), installed_versions.end(),
                  back_inserter(not_installed_versions), pred());

   for (const auto& v : not_installed_versions) {
      for (const string& full_target_name : v.targets_) {
         auto target_name_offset = [&] {
            std::string::size_type offset = full_target_name.front() == '@' ? 1 : 0;
            if (full_target_name.length() - offset < internal_path.length())
               return std::string::npos;

            if (!internal_path.empty()) {
               if (!std::equal(internal_path.begin(), internal_path.end(), full_target_name.begin() + offset))
                  return std::string::npos;
            }

            offset += internal_path.length();
            // non-empty internal path means we have '/' after it
            if (!internal_path.empty())
               ++offset;

            if (full_target_name.find('/', offset) != std::string::npos)
               return std::string::npos;

            return offset;
         } ();

         if (target_name_offset != std::string::npos) {
            auto target = boost::make_unique<warehouse_meta_target>(p, full_target_name.substr(target_name_offset), v.version_);
            if (full_target_name.front() != '@')
               target->set_explicit(true);
            p.add_target(std::move(target));
         }
      }
   }
}

static
void warehouse_trap_rule(invocation_context& ctx,
                         const parscore::identifier& public_id)
{
   auto& whm = ctx.current_project_.get_engine().warehouse_manager();
   auto iwh = whm.find(ctx.current_project_);
   assert(iwh != whm.end() && "warehouse-trap used outside of warehouse tree");
   warehouse& wh = *iwh->second;
   if (!wh.has_project(location_t(public_id.to_string()), warehouse::any_version))
      throw std::runtime_error("Can't find '" + public_id.to_string() + "' in warehouse");

   add_traps(wh, ctx.current_project_, public_id.to_string(), {});
}

void install_warehouse_rules(engine& engine)
{
   engine.get_rule_manager().add_rule("warehouse-trap", &warehouse_trap_rule, { "public-id" });
}

}
