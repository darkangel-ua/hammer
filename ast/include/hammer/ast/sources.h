#pragma once

#include <hammer/ast/expression.h>

namespace hammer{ namespace ast{

class sources : public expression
{
   public:
		sources(const expression* content)
			: content_(content)
      {}
      
		const expression* content() const { return content_; }
		parscore::source_location start_loc() const override { return content_->start_loc(); }
      bool accept(visitor& v) const override;

   private:
		const expression* content_;
};

}}
