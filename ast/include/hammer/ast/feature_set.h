#pragma once
#include <hammer/ast/expression.h>

namespace hammer { namespace ast {

class feature_set : public expression {
   public:
		feature_set(const expression* values) : values_(values) {}
		parscore::source_location start_loc() const override { return values_->start_loc(); }
		bool accept(visitor& v) const override;
		const expression* values() const { return values_; }

	private:
		const expression* values_;
};

}}
