#pragma once
#include <hammer/ast/expression.h>

namespace hammer { namespace ast {

class statement : public expression {
};

class expression_statement : public statement {
	public:
		expression_statement(const expression* e) : content_(e) {}
		const expression* content() const { return content_; }
		parscore::source_location start_loc() const override { return content_->start_loc(); }
		bool accept(visitor& v) const override;

	private:
		const expression* content_;
};

}}
