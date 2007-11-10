#pragma once

#include <boost/noncopyable.hpp>

extern "C"
{
#  include "build/hammerLexer.h"
#  include "build/hammerParser.h"
#  include "build/hammer_walker.h"
};

namespace hammer
{
   struct hammer_walker_context;
   class parser : public boost::noncopyable
   {
      public:
         parser();
         bool parse(const char* file_name);
         void walk(hammer_walker_context* ctx);
         ~parser() { reset(); }

      private:
         pANTLR3_INPUT_STREAM input_;
         phammerLexer lexer_;
         pANTLR3_COMMON_TOKEN_STREAM tstream_;
         phammerParser parser_;
         hammerParser_project_return langAST_;

         void reset();
   };
}