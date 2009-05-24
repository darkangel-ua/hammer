#include "stdafx.h"
#include <boost/filesystem/convenience.hpp>
#include <boost/assign/list_of.hpp>
#include <antlr3recognizersharedstate.h>
#include <hammer/core/parser.h>
#include "hammer_parser_context.h"
#include "non_buffered_token_stream.h"

#include "build/hammerLexer.h"
#include "build/hammerParser.h"
#include "build/hammer_walker.h"

using namespace std;


namespace hammer{

   struct parser::impl_t
   {
      impl_t(engine* e) 
         : engine_(e), input_(0), lexer_(0),
           tstream_(0), parser_(0)
      {
         memset(&langAST_, 0, sizeof(langAST_));
      }

      engine* engine_;
      pANTLR3_INPUT_STREAM input_;
      phammerLexer lexer_;
      pANTLR3_COMMON_TOKEN_STREAM tstream_;
      phammerParser parser_;
      hammerParser_rules_return langAST_;
   };

   static void displayRecognitionError(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames)
   {
      
      details::hammer_parser_context* ctx = static_cast<details::hammer_parser_context*>(static_cast<pANTLR3_PARSER>(recognizer->super)->super);
      ++ctx->error_count_;
      ctx->base_displayRecognitionError(recognizer, tokenNames);
   }

   parser::parser(engine* e) : impl_(new impl_t(e))
   {
   }
   
   parser::~parser()
   {
      reset();
      delete impl_;
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

      impl_->input_ = antlr3AsciiFileStreamNew((pANTLR3_UINT8)file_name);
      impl_->lexer_ = hammerLexerNew(impl_->input_);
//      tstream_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer_));
      impl_->tstream_ = non_buffered_token_stream::create(ANTLR3_SIZE_HINT, TOKENSOURCE(impl_->lexer_));
      impl_->parser_ = hammerParserNew(impl_->tstream_);
      details::hammer_parser_context ctx;
      ctx.base_displayRecognitionError = impl_->parser_->pParser->rec->displayRecognitionError;
      ctx.token_stream_ = static_cast<non_buffered_token_stream*>(impl_->tstream_->super);
      ctx.token_stream_->ctx_.input_ = impl_->input_;
      ctx.token_stream_->ctx_.lexer_ = impl_->lexer_->pLexer;
      impl_->parser_->pParser->rec->displayRecognitionError = &displayRecognitionError;
      impl_->lexer_->pLexer->super = &static_cast<non_buffered_token_stream*>(impl_->tstream_->super)->ctx_;
      ctx.engine_ = impl_->engine_;
      impl_->parser_->pParser->super = &ctx;
      impl_->langAST_ = impl_->parser_->rules(impl_->parser_);
//      pANTLR3_STRING s = langAST_.tree->toStringTree(langAST_.tree);
      return ctx.error_count_ == 0;
   }
   
   void parser::reset()
   {
      if (impl_->parser_)
         impl_->parser_->free(impl_->parser_);
      impl_->parser_ = 0;
      if (impl_->tstream_)
         impl_->tstream_->free(impl_->tstream_);
      impl_->tstream_ = 0;
      if (impl_->lexer_)
         impl_->lexer_->free(impl_->lexer_);
      impl_->lexer_ = 0;
      if (impl_->input_)
         impl_->input_->close(impl_->input_);
      impl_->input_ = 0;
   }                 

   void parser::walk(hammer_walker_context* ctx)
   {
      // if parsed empty file than tree will be null
      if (!impl_->langAST_.tree)
         return;

      pANTLR3_COMMON_TREE_NODE_STREAM nodes;
      phammer_walker          hammer_walker;

      nodes	= antlr3CommonTreeNodeStreamNewTree(impl_->langAST_.tree, ANTLR3_SIZE_HINT); // sIZE HINT WILL SOON BE DEPRECATED!!
      hammer_walker = hammer_walkerNew(nodes);
      hammer_walker->pTreeParser->super = ctx;
      hammer_walker->project(hammer_walker);
      
      hammer_walker->free(hammer_walker);
      nodes->free(nodes);
   }
}