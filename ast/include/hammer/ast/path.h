#pragma once
#include <cassert>
#include <hammer/ast/expression.h>
#include <hammer/ast/types.h>

namespace hammer { namespace ast {

class path : public expression {
   public:
		path(const parscore::token root_name,
			  const expressions_t& elements,
           const parscore::token& trailing_slash)
			: root_name_(root_name),
			  elements_(elements),
           trailing_slash_(trailing_slash)
		{
			assert(!elements_.empty());
		}

		const parscore::token& root_name() const { return root_name_; }
		const expressions_t& elements() const { return elements_; }
		parscore::source_location start_loc() const override { return root_name_.valid() ? root_name_.start_loc() : elements_.front()->start_loc(); }
		bool accept(visitor& v) const override;
		std::string to_string() const;
      bool has_wildcard() const;

   private:
	  const parscore::token root_name_;
	  // id_expr or list_of id_expr in case of wildcard element
	  // TODO: make wildcard node
	  const expressions_t elements_;
     const parscore::token trailing_slash_;
};

}}
