#pragma once
#include <boost/variant/variant.hpp>
#include <hammer/core/rule_manager.h>
#include <hammer/core/location.h>

namespace hammer {

class sources_decl;
class requirements_decl;
class usage_requirements_decl;
class feature_set;
class feature;

typedef boost::variant<const feature*, const feature_set*> feature_or_feature_set_t;
typedef std::vector<parscore::identifier> id_or_list_of_ids_t;
typedef std::vector<location_t> path_or_list_of_paths_t;
typedef std::vector<wcpath> wcpath_or_list_of_wcpaths_t;

HAMMER_RULE_MANAGER_SIMPLE_TYPE(sources_decl, sources);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(requirements_decl, requirement_set);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(usage_requirements_decl, usage_requirements);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(feature_set, feature_set);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(location_t, path);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(path_or_list_of_paths_t, path_or_list_of_paths);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(wcpath, wcpath);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(wcpath_or_list_of_wcpaths_t, wcpath_or_list_of_wcpaths);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(feature, feature);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(feature_or_feature_set_t, feature_or_feature_set);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(id_or_list_of_ids_t, identifier_or_list_of_identifiers);

}

