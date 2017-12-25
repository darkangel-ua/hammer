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
typedef boost::variant<location_t, std::vector<location_t>> path_or_list_of_paths_t;

template<>
struct rule_argument_type_info<sources_decl> { static const rule_argument_type ast_type = rule_argument_type::sources; };

template<>
struct rule_argument_type_info<requirements_decl> { static const rule_argument_type ast_type = rule_argument_type::requirement_set; };

template<>
struct rule_argument_type_info<usage_requirements_decl> { static const rule_argument_type ast_type = rule_argument_type::usage_requirements; };

template<>
struct rule_argument_type_info<feature_set> { static const rule_argument_type ast_type = rule_argument_type::feature_set; };

template<>
struct rule_argument_type_info<location_t> { static const rule_argument_type ast_type = rule_argument_type::path; };

template<>
struct rule_argument_type_info<feature> { static const rule_argument_type ast_type = rule_argument_type::feature; };

template<>
struct rule_argument_type_info<feature_or_feature_set_t> { static const rule_argument_type ast_type = rule_argument_type::feature_or_feature_set; };

template<>
struct rule_argument_type_info<id_or_list_of_ids_t> { static const rule_argument_type ast_type = rule_argument_type::identifier_or_list_of_identifiers; };

template<>
struct rule_argument_type_info<path_or_list_of_paths_t> { static const rule_argument_type ast_type = rule_argument_type::path_or_list_of_paths; };

}

