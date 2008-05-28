#pragma once

#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>

extern "C"
{
#  include "build/hammerLexer.h"
#  include "build/hammerParser.h"
#  include "build/hammer_walker.h"
};

namespace hammer
{
   struct hammer_walker_context;
   class engine;
   class parser : public boost::noncopyable
   {
      public:
         parser(engine* e);
         bool parse(const char* file_name);
         bool parse(const boost::filesystem::path& file_name);
         void walk(hammer_walker_context* ctx);
         ~parser();

      private:
         engine* engine_;
         pANTLR3_INPUT_STREAM input_;
         phammerLexer lexer_;
         pANTLR3_COMMON_TOKEN_STREAM tstream_;
         phammerParser parser_;
         hammerParser_rules_return langAST_;

         void reset();
   };
}
