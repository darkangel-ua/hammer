#pragma once

#include <hammer/ast/statement.h>

namespace hammer{ namespace ast{

class rule_invocation;

class target_def : public statement
{
   public:
		target_def(const parscore::source_location explicit_tag,
					  const parscore::source_location local_tag,
					  const rule_invocation* ri)
			: explicit_tag_(explicit_tag),
			  local_tag_(local_tag),
			  ri_(ri)
		{}

		parscore::source_location start_loc() const override;
		bool accept(visitor& v) const override;

		const rule_invocation* body() const { return ri_; }
		parscore::source_location local_tag() const { return local_tag_; }
		parscore::source_location explicit_tag() const { return explicit_tag_; }

	private:
		const parscore::source_location explicit_tag_;
		const parscore::source_location local_tag_;
		const rule_invocation* ri_;
};

}}
