#if !defined(h_bfff9dfc_78a7_4dd6_94c5_9033e1fd6b33)
#define h_bfff9dfc_78a7_4dd6_94c5_9033e1fd6b33

#include <cassert>
#include <hammer/ast/expression.h>
#include <hammer/ast/types.h>

namespace hammer{ namespace ast{

class path : public expression
{
   public:
		path(const parscore::token root_name,
			  const expressions_t& elements)
			: root_name_(root_name),
			  elements_(elements)
		{
			assert(!elements_.empty());
		}

		const parscore::token& root_name() const { return root_name_; }
		const expressions_t& elements() const { return elements_; }
		parscore::source_location start_loc() const override { return elements_.front()->start_loc(); }
		bool accept(visitor& v) const override;
		std::string to_string() const;

   private:
	  const parscore::token root_name_;
	  // id_expr or list_of id_expr in case of wildcard element
	  // TODO: make wildcard node
	  const expressions_t elements_;
};

}}

#endif
