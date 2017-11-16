#include <hammer/parser/parser.h>
#include <boost/filesystem/operations.hpp>
#include "hammerLexer.h"
#include "hammerParser.h"
#include "hammer_sema.h"
#include <hammer/ast/context.h>
#include <hammer/ast/hamfile.h>

namespace hammer{

namespace {

struct parser_context
{
   ~parser_context()
   {
      if (input_) {
         parser_->free(parser_);
         tstream_->free(tstream_);
         lexer_->free(lexer_);
         input_->close(input_);
      }
   }

   pANTLR3_INPUT_STREAM input_ = nullptr;
   phammerLexer lexer_;
   pANTLR3_COMMON_TOKEN_STREAM tstream_;
   phammerParser parser_;
   hammerParser_hamfile_return langAST_;
};

}

static
ast_hamfile_ptr
parse(std::unique_ptr<parser_context> ctx,
      sema::actions& actions)
{
   ctx->lexer_ = hammerLexerNew(ctx->input_);
   ctx->tstream_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(ctx->lexer_));
   ctx->parser_ = hammerParserNew(ctx->tstream_);
   ctx->langAST_ = ctx->parser_->hamfile(ctx->parser_);
//   pANTLR3_STRING s = langAST_.tree->toStringTree(langAST_.tree);

   pANTLR3_COMMON_TREE_NODE_STREAM nodes;
   phammer_sema          hammer_sema;

   nodes	= antlr3CommonTreeNodeStreamNewTree(ctx->langAST_.tree, ANTLR3_SIZE_HINT);
   hammer_sema = hammer_semaNew(nodes);
   hammer_sema->pTreeParser->super = &actions;
   const ast::hamfile* result = hammer_sema->hamfile(hammer_sema);

   nodes->free(nodes);
   hammer_sema->free(hammer_sema);

   parser_context* p_ctx = ctx.release();
   return { result, [=](const ast::hamfile*) { delete p_ctx; } };
}

ast_hamfile_ptr
parse_hammer_script(const boost::filesystem::path& hamfile,
                    sema::actions& actions)
{
   if (!exists(hamfile))
      throw std::runtime_error("Path does not exists '" + hamfile.string() + "'");

   std::unique_ptr<parser_context> ctx(new parser_context);
   ctx->input_ = antlr3AsciiFileStreamNew((pANTLR3_UINT8)hamfile.string().c_str());

   return parse(std::move(ctx), actions);
}

ast_hamfile_ptr
parse_hammer_script(const std::string content,
                    const std::string content_name,
                    sema::actions& actions)
{
   std::unique_ptr<parser_context> ctx(new parser_context);
   ctx->input_ = antlr3NewAsciiStringInPlaceStream((pANTLR3_UINT8)content.c_str(), content.size(), (pANTLR3_UINT8)content_name.c_str());

   return parse(std::move(ctx), actions);
}

}
