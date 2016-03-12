#include "stdafx.h"
#include <hammer/parser/parser.h>
#include <boost/filesystem/operations.hpp>
#include "hammerLexer.h"
#include "hammerParser.h"
#include "hammer_sema.h"
#include <hammer/ast/context.h>

namespace hammer{namespace parser{

namespace{
   struct parser_context : public ast::parser_context
   {
      parser_context() : input_(NULL) {}
      ~parser_context()
      {
         if (input_)
         {
//            langAST_.tree->free(langAST_.tree);
            parser_->free(parser_);
            tstream_->free(tstream_);
            lexer_->free(lexer_);
            input_->close(input_);
         }
      }

      pANTLR3_INPUT_STREAM input_;
      phammerLexer lexer_;
      pANTLR3_COMMON_TOKEN_STREAM tstream_;
      phammerParser parser_;
      hammerParser_hamfile_return langAST_;
   };
}
parser::parser(const boost::filesystem::path& hamfile,
               const sema::actions& actions)
   : hamfile_(hamfile),
     actions_(actions)
{

}

const ast::hamfile* 
parser::parse(const boost::filesystem::path& hamfile,
             const sema::actions& actions)
{
   parser p(hamfile, actions);
   return p.parse_impl();
}

const ast::hamfile* parser::parse_impl()
{
   if (!exists(hamfile_))
      throw std::runtime_error("Path does not exists '" + hamfile_.native_file_string() + "'");

   parser_context* ctx = new parser_context;
   actions_.on_begin_parse(ctx);

   ctx->input_ = antlr3AsciiFileStreamNew((pANTLR3_UINT8)hamfile_.native_file_string().c_str());
   ctx->lexer_ = hammerLexerNew(ctx->input_);
   ctx->tstream_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(ctx->lexer_));
   ctx->parser_ = hammerParserNew(ctx->tstream_);
   ctx->langAST_ = ctx->parser_->hamfile(ctx->parser_);
//   pANTLR3_STRING s = langAST_.tree->toStringTree(langAST_.tree);

   pANTLR3_COMMON_TREE_NODE_STREAM nodes;
   phammer_sema          hammer_sema;

   nodes	= antlr3CommonTreeNodeStreamNewTree(ctx->langAST_.tree, ANTLR3_SIZE_HINT);
   hammer_sema = hammer_semaNew(nodes);
   hammer_sema->pTreeParser->super = const_cast<sema::actions*>(&actions_);
   const ast::hamfile* result = hammer_sema->hamfile(hammer_sema);

   nodes->free(nodes);
   hammer_sema->free(hammer_sema);

   return result;
}

}}
