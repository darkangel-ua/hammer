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
      {
      }
      virtual parscore::source_location start_loc() const { return loc_; }

      virtual bool accept(visitor& v) const;
   
   private:
      parscore::source_location loc_;
};

class empty_expr : public expression
{
   public:
      empty_expr(const parscore::source_location& loc)
         : loc_(loc)
      {
      }

      virtual parscore::source_location start_loc() const { return loc_; }
      virtual bool accept(visitor& v) const;

   private:
      parscore::source_location loc_;
};

class id_expr : public expression
{
   public:
      id_expr(const parscore::identifier& id) 
         : id_(id) 
      {}

      const parscore::identifier& id() const { return id_; } 

      virtual parscore::source_location start_loc() const { return id_.start_lok(); }
      virtual bool accept(visitor& v) const;
   
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
      virtual parscore::source_location start_loc() const { return name_.start_lok(); }
      virtual bool accept(visitor& v) const;

   private:
      parscore::identifier name_;
      const expression* value_;
};

bool is_error_expr(const expression* e);

}}

#endif //h_8babae10_1ba8_479f_ae25_e20b77eb41dc
