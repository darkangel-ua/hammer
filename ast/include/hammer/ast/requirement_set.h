#pragma once
#include <hammer/ast/types.h>
#include <hammer/ast/expression.h>

namespace hammer { namespace ast {

class requirement_set : public expression {
	public:
		requirement_set(const expression* requirements);

		const expression* requirements() const { return requirements_; }
		parscore::source_location start_loc() const override;
		bool accept(visitor& v) const override;

	private:
		const expression* requirements_;
};

}}
