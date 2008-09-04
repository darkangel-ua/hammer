#include "stdafx.h"
#include "parser.h"
#include "hammer_parser_context.h"
#include <antlr3recognizersharedstate.h>
#include "non_buffered_token_stream.h"

using namespace std;


namespace hammer{

   static void displayRecognitionError(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames)
   {
      
      details::hammer_parser_context* ctx = static_cast<details::hammer_parser_context*>(static_cast<pANTLR3_PARSER>(recognizer->super)->super);
      ++ctx->error_count_;
      ctx->base_displayRecognitionError(recognizer, tokenNames);
   }

   parser::parser(engine* e) : engine_(e), input_(0), lexer_(0),
                               tstream_(0), parser_(0)
   {
      memset(&langAST_, 0, sizeof(langAST_));
   }
   
   parser::~parser()
   {
      reset();
   }

   bool parser::parse(const boost::filesystem::path& file_name)
   {
      if (!exists(file_name))
         throw std::runtime_error("Path does not exists '" + file_name.string() + "'");

      return parse(file_name.string().c_str());
   }

   bool parser::parse(const char* file_name)
   {
      if (!exists(boost::filesystem::path(file_name)))
         throw std::runtime_error("Path does not exists '" + string(file_name) + "'");

      reset();

      input_	= antlr3AsciiFileStreamNew((pANTLR3_UINT8)file_name);
      lexer_ = hammerLexerNew(input_);
//      tstream_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer_));
      tstream_ = non_buffered_token_stream::create(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer_));
      parser_ = hammerParserNew(tstream_);
      details::hammer_parser_context ctx;
      ctx.base_displayRecognitionError = parser_->pParser->rec->displayRecognitionError;
      ctx.token_stream_ = static_cast<non_buffered_token_stream*>(tstream_->super);
      ctx.token_stream_->ctx_.input_ = input_;
      ctx.token_stream_->ctx_.lexer_ = lexer_->pLexer;
      parser_->pParser->rec->displayRecognitionError = &displayRecognitionError;
      lexer_->pLexer->super = &static_cast<non_buffered_token_stream*>(tstream_->super)->ctx_;
      ctx.engine_ = engine_;
      parser_->pParser->super = &ctx;
      langAST_ = parser_->rules(parser_);
      pANTLR3_STRING s = langAST_.tree->toStringTree(langAST_.tree);
      return ctx.error_count_ == 0;
   }
   
   void parser::reset()
   {
      if (parser_)
         parser_->free(parser_);
      parser_ = 0;
      if (tstream_)
         tstream_->free(tstream_);
      tstream_ = 0;
      if (lexer_)
         lexer_->free(lexer_);
      lexer_ = 0;
      if (input_)
         input_->close(input_);
      input_ = 0;
   }                 

   void parser::walk(hammer_walker_context* ctx)
   {
      // if parsed empty file than tree will be null
      if (!langAST_.tree)
         return;

      pANTLR3_COMMON_TREE_NODE_STREAM nodes;
      phammer_walker          hammer_walker;

      nodes	= antlr3CommonTreeNodeStreamNewTree(langAST_.tree, ANTLR3_SIZE_HINT); // sIZE HINT WILL SOON BE DEPRECATED!!
      hammer_walker = hammer_walkerNew(nodes);
      hammer_walker->pTreeParser->super = ctx;
      hammer_walker->project(hammer_walker);
      
      hammer_walker->free(hammer_walker);
      nodes->free(nodes);
   }
}