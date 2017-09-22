#if !defined(h_8babae10_1ba8_479f_ae25_e20b77eb41dc)
#define h_8babae10_1ba8_479f_ae25_e20b77eb41dc

#include <hammer/ast/statement.h>
#include <hammer/parscore/identifier.h>

namespace hammer{ namespace ast{

class expression : public statement
{
   public:
      virtual parscore::source_location start_loc() const = 0;

   private:
      parscore::source_location loc_;
};

class error_expression : public expression
{
   public:
      error_expression(const parscore::source_location& loc) 
         : loc_(loc) 
      {}

      error_expression(const expression* e) 
         : loc_(e->start_loc()) 
      {}

      parscore::source_location start_loc() const override { return loc_; }
      bool accept(visitor& v) const override;

   private:
      parscore::source_location loc_;
};

class empty_expr : public expression
{
   public:
      empty_expr(const parscore::identifier& next)
         : next_(next)
      {}

      parscore::source_location start_loc() const override { return next_.start_lok(); }
      bool accept(visitor& v) const override;
      const parscore::identifier& next_token() const { return next_; }

   private:
      parscore::identifier next_;
};

class id_expr : public expression
{
   public:
      id_expr(const parscore::identifier& id) 
         : id_(id) 
      {}

      const parscore::identifier& id() const { return id_; } 

      parscore::source_location start_loc() const override { return id_.start_lok(); }
      bool accept(visitor& v) const override;

   private:
      parscore::identifier id_;
};

class named_expr : public expression
{
   public:
      named_expr(const parscore::identifier& name,
                 const expression* value)
                 : name_(name),
                   value_(value)
      {
      }
   
      const parscore::identifier& name() const { return name_; }
      const expression* value() const { return value_; }
      parscore::source_location start_loc() const override { return name_.start_lok(); }
      bool accept(visitor& v) const override;

   private:
      parscore::identifier name_;
      const expression* value_;
};

class public_expr : public expression
{
   public:
	  public_expr(const parscore::identifier& tag,
				  const expression* value)
		 : tag_(tag),
		   value_(value)
	  {}

	  const parscore::identifier& tag() const { return tag_; }
	  const expression* value() const { return value_; }

	  parscore::source_location start_loc() const override { return tag_.start_lok(); }
	  bool accept(visitor& v) const override;

   private:
	  parscore::identifier tag_;
	  const expression* value_;
};

bool is_error_expr(const expression* e);

}}

#endif
