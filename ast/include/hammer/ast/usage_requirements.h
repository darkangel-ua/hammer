#pragma once
#include <hammer/ast/expression.h>

namespace hammer { namespace ast {

class usage_requirements : public expression {
   public:
		usage_requirements(const expression* requirements);

		const expression* requirements() const { return requirements_; }
		parscore::source_location start_loc() const override;
		bool accept(visitor& v) const override;

	private:
		const expression* requirements_;
};

}}
