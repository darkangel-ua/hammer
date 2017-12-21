#pragma once
#include <vector>
#include <boost/variant/variant.hpp>
#include <hammer/parscore/identifier.h>
#include <hammer/core/location.h>

namespace hammer{ namespace ast{
	class hamfile;
	class expression;
}}

namespace hammer {

class feature;
class feature_set;

typedef boost::variant<const feature*, const feature_set*> feature_or_feature_set_t;
typedef boost::variant<parscore::identifier, std::vector<parscore::identifier>> id_or_list_of_ids_t;
typedef boost::variant<location_t, std::vector<location_t>> path_or_list_of_paths_t;

class project;
class invocation_context;

void ast2objects(invocation_context& ctx,
                 const ast::hamfile& node);

std::unique_ptr<feature_or_feature_set_t>
ast2feature_or_feature_set(invocation_context& ctx,
                           const ast::expression& e);

}
