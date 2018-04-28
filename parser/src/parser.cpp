#include <hammer/parser/parser.h>
#include <boost/filesystem/operations.hpp>
#include "hammer_v2Lexer.h"
#include "hammer_v2Parser.h"
#include "hammer_sema_v2.h"
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
   phammer_v2Lexer lexer_;
   pANTLR3_COMMON_TOKEN_STREAM tstream_;
   phammer_v2Parser parser_;
   hammer_v2Parser_hamfile_return langAST_;
};

}

// this is our custom hackish version of createTokenFromToken method copied from antlr3 lib
// FIXME: I don't really know is this correct or not.
//        I need input & lineStart fields for source_location::line_content to work
static
pANTLR3_COMMON_TOKEN
createTokenFromToken(pANTLR3_BASE_TREE_ADAPTOR adaptor,
                     pANTLR3_COMMON_TOKEN fromToken)
{
    pANTLR3_COMMON_TOKEN    newToken;

    newToken	= adaptor->tokenFactory->newToken(adaptor->tokenFactory);

    if (newToken != NULL) {
		// Create the text using our own string factory to avoid complicating
		// commontoken.
		//
		pANTLR3_STRING	text;

		newToken->toString  = fromToken->toString;

		if	(fromToken->textState == ANTLR3_TEXT_CHARP) {
			newToken->textState		= ANTLR3_TEXT_CHARP;
			newToken->tokText.chars	= fromToken->tokText.chars;
		} else {
			text						= fromToken->getText(fromToken);
			newToken->textState			= ANTLR3_TEXT_STRING;
			newToken->tokText.text	    = adaptor->strFactory->newPtr(adaptor->strFactory, text->chars, text->len);
		}

		newToken->setLine				(newToken, fromToken->getLine(fromToken));
		newToken->setTokenIndex			(newToken, fromToken->getTokenIndex(fromToken));
		newToken->setCharPositionInLine	(newToken, fromToken->getCharPositionInLine(fromToken));
		newToken->setChannel			(newToken, fromToken->getChannel(fromToken));
		newToken->setType				(newToken, fromToken->getType(fromToken));
      newToken->input = fromToken->input;
      newToken->lineStart = fromToken->lineStart;
    }

    return  newToken;
}

static
ast_hamfile_ptr
parse(std::unique_ptr<parser_context> ctx,
      sema::actions& actions)
{
   ctx->lexer_ = hammer_v2LexerNew(ctx->input_);
   ctx->tstream_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(ctx->lexer_));
   ctx->parser_ = hammer_v2ParserNew(ctx->tstream_);
   ctx->parser_->adaptor->createTokenFromToken = createTokenFromToken;

   ctx->langAST_ = ctx->parser_->hamfile(ctx->parser_);
//   pANTLR3_STRING s = langAST_.tree->toStringTree(langAST_.tree);
   // FIXME: quick hack to be fixed later

   if (ctx->parser_->pParser->rec->state->errorCount)
      throw std::runtime_error("Parsing error");

   pANTLR3_COMMON_TREE_NODE_STREAM nodes;
   phammer_sema_v2          hammer_sema;

   nodes	= antlr3CommonTreeNodeStreamNewTree(ctx->langAST_.tree, ANTLR3_SIZE_HINT);
   hammer_sema = hammer_sema_v2New(nodes);
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
   ctx->input_ = antlr3NewAsciiStringCopyStream((pANTLR3_UINT8)content.c_str(), content.size(), (pANTLR3_UINT8)content_name.c_str());

   return parse(std::move(ctx), actions);
}

}
