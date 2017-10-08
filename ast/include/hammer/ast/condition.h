#pragma once

#include <hammer/ast/expression.h>

namespace hammer{ namespace ast{

class binary_op : public expression
{
	public:
		binary_op(const expression* left,
					 const expression* right)
			: left_(left),
			  right_(right)
		{}

		parscore::source_location start_loc() const override { return left_->start_loc(); }
		const expression* left() const { return left_; }
		const expression* right() const { return right_; }

	private:
		const expression* left_;
		const expression* right_;
};

class logical_or : public binary_op
{
	public:
		logical_or(const expression* left,
					  const expression* right)
			: binary_op(left, right)
		{}
		bool accept(visitor& v) const override;
};

class logical_and : public binary_op
{
	public:
		logical_and(const expression* left,
						const expression* right)
			: binary_op(left, right)
		{}
		bool accept(visitor& v) const override;
};

class condition_expr : public expression
{
   public:
		condition_expr(const expression* condition,
							const expression* result)
			: condition_(condition),
			  result_(result)
		{}

		parscore::source_location start_loc() const override { return condition_->start_loc(); }
		const expression* condition() const { return condition_; }
		const expression* result() const { return result_; }
		bool accept(visitor& v) const override;

	private:
		const expression* condition_;
		const expression* result_;
};

}}
