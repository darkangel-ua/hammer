#pragma once
#include <functional>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional/optional.hpp>
#include <hammer/core/project.h>

namespace hammer {

class basic_target;
class type_registry;
class feature_registry;
class generator_registry;
class feature;
class project_requirements_decl;
class toolset_manager;
class scanner_manager;
class output_location_strategy;
class warehouse_manager;
class rule_manager;

class parsing_error : public std::runtime_error {
   public:
      using runtime_error::runtime_error;
};

class engine : public boost::noncopyable {
   public:
      engine();

      struct global_project_ref {
         explicit global_project_ref(const location_t& l) : value_(l) {}
         const location_t value_;
      };

      // FIXME: this should return const, because we shouldn't (FIXME: can't) change project after it was added to engine
      loaded_projects
      load_project(const global_project_ref& project_ref);

      // FIXME: this should return const, because we shouldn't (FIXME: can't) change project after it was added to engine
      project&
      load_project(location_t fs_project_path);

      loaded_projects
      try_load_project(location_t fs_project_path);

      void load_hammer_script(location_t filepath);
      void load_hammer_script(const std::string& script_body,
                              const std::string& script_name);

      // FIXME: this should return const, because we shouldn't (FIXME: can't) change project after it was added to engine
      project&
      insert(std::unique_ptr<project> p);

      type_registry& get_type_registry() { return *type_registry_; }
      generator_registry& generators() const { return *generators_; }
      hammer::feature_registry& feature_registry() { return *feature_registry_; }
      const hammer::feature_registry& feature_registry() const { return *feature_registry_; }
      rule_manager& get_rule_manager() { return *rule_manager_; }
      const rule_manager& get_rule_manager() const { return *rule_manager_; }
      hammer::toolset_manager& toolset_manager() { return *toolset_manager_; }
      hammer::scanner_manager& scanner_manager() { return *scanner_manager_; }
      const hammer::scanner_manager& scanner_manager() const { return *scanner_manager_; }
      hammer::warehouse_manager& warehouse_manager() { return *warehouse_manager_; }
      hammer::output_location_strategy& output_location_strategy() { return *output_location_strategy_; }
      void output_location_strategy(boost::shared_ptr<hammer::output_location_strategy>& strategy);
      void use_project(const project& p, const std::string& project_id_alias, const location_t& project_location);

      const project::aliases_t&
      global_aliases() const { return global_project_->aliases(); }

      ~engine();

      void add_alias(const location_t& alias_path,
                     const location_t& full_project_path,
                     feature_set* props);

      void enable_error_verbosity() { error_verbosity_ = true; }
      void disable_error_verbosity() { error_verbosity_ = false; }

   private:
      typedef boost::unordered_map<const location_t,
                                   boost::shared_ptr<project>,
                                   boost::hash<location_t>,
                                   location_equal_to> projects_t;
      projects_t projects_;
      std::unique_ptr<project> global_project_;

      boost::shared_ptr<type_registry> type_registry_;
      std::unique_ptr<hammer::feature_registry> feature_registry_;
      std::unique_ptr<rule_manager> rule_manager_;

      boost::shared_ptr<generator_registry> generators_;
      boost::shared_ptr<hammer::toolset_manager> toolset_manager_;
      boost::shared_ptr<hammer::scanner_manager> scanner_manager_;
      boost::shared_ptr<hammer::output_location_strategy> output_location_strategy_;

      std::unique_ptr<hammer::warehouse_manager> warehouse_manager_;

      bool error_verbosity_ = true;

      void load_hammer_script_v2(const project* parent,
                                 location_t filepath);
      void load_hammer_script_v2(const project* parent,
                                 const std::string& script_body,
                                 const std::string& script_name);

      std::unique_ptr<project>
      load_project_v2(const location_t& project_path,
                      const project* upper_project);

      project* get_upper_project(const location_t& project_path);
};

boost::optional<boost::filesystem::path>
find_root(boost::filesystem::path from_path);

bool has_project_file(const boost::filesystem::path& folder);

std::vector<std::reference_wrapper<const project>>
resolve_project_query(engine& e,
                      const std::string& query);

boost::regex
target_ref_mask_to_regex(const std::string& mask);

}

