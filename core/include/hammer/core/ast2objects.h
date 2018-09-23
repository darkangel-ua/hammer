#pragma once
#include <vector>
#include <hammer/parscore/identifier.h>
#include <hammer/core/location.h>
#include <hammer/core/rule_argument_types.h>

namespace hammer { namespace ast {

class hamfile;
class expression;

}}

namespace hammer {

class project;
class invocation_context;

class ast2objects_semantic_error : public std::exception {
	public:
		ast2objects_semantic_error() {}
};

void ast2objects(invocation_context& ctx,
                 const ast::hamfile& node);

std::unique_ptr<feature_or_feature_set_t>
ast2feature_or_feature_set(invocation_context& ctx,
                           const ast::expression& e);

}
