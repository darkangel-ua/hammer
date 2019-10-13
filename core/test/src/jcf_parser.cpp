#include "stdafx.h"

extern "C"
{
#  include "build/jcfLexer.h"
#  include "build/jcfParser.h"
#  include "build/jcf_walker.h"
};

#include "jcf_parser.h"
#include <boost/filesystem/convenience.hpp>

using namespace std;

struct jcf_parser::impl_t
{
   impl_t() : input_(0), lexer_(0),
              tstream_(0), parser_(0)
   {
      memset(&langAST_, 0, sizeof(langAST_));
   }

   pANTLR3_INPUT_STREAM input_;
   pjcfLexer lexer_;
   pANTLR3_COMMON_TOKEN_STREAM tstream_;
   pjcfParser parser_;
   jcfParser_jsf_file_return langAST_;
};

namespace 
{
   struct jcf_parser_context
   {
      jcf_parser_context() : error_count_(0) {}

      void (*base_displayRecognitionError)(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames);
      unsigned int error_count_;
   };
   
   void displayRecognitionError(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames)
   {

      jcf_parser_context* ctx = static_cast<jcf_parser_context*>(static_cast<pANTLR3_PARSER>(recognizer->super)->super);
      ++ctx->error_count_;
      ctx->base_displayRecognitionError(recognizer, tokenNames);
   }
}

jcf_parser::jcf_parser() : impl_(new impl_t)
{
}

bool jcf_parser::parse(const boost::filesystem::path& file_name)
{
   if (!exists(file_name))
      throw std::runtime_error("Path does not exists '" + file_name.string() + "'");

   return parse(file_name.string().c_str());
}

bool jcf_parser::parse(const char* file_name)
{
   if (!exists(boost::filesystem::path(file_name)))
      throw std::runtime_error("Path does not exists '" + string(file_name) + "'");

   reset();

   impl_->input_    = antlr3AsciiFileStreamNew((pANTLR3_UINT8)file_name);
   impl_->lexer_ = jcfLexerNew(impl_->input_);
   impl_->tstream_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(impl_->lexer_));
   impl_->parser_ = jcfParserNew(impl_->tstream_);
   impl_->langAST_ = impl_->parser_->jsf_file(impl_->parser_);

   jcf_parser_context ctx;
   ctx.base_displayRecognitionError = impl_->parser_->pParser->rec->displayRecognitionError;
   impl_->parser_->pParser->super = &ctx;
   
   return ctx.error_count_ == 0;
}

void jcf_parser::reset()
{
//    if (impl_->langAST_.tree)
//       impl_->langAST_.tree->free(impl_->langAST_.tree);
//    impl_->langAST_.tree = 0;

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

void jcf_parser::walk(const vector<hammer::basic_target*>& targets, hammer::engine* e)
{
   pANTLR3_COMMON_TREE_NODE_STREAM nodes;
   pjcf_walker          jcf_walker;

   nodes     = antlr3CommonTreeNodeStreamNewTree(impl_->langAST_.tree, ANTLR3_SIZE_HINT); // sIZE HINT WILL SOON BE DEPRECATED!!
   jcf_walker = jcf_walkerNew(nodes);
   jcf_walker->pTreeParser->super = e;
   jcf_walker->jcf_file(jcf_walker, const_cast<vector<hammer::basic_target*>*>(&targets));
   
   jcf_walker->free(jcf_walker);
   nodes->free(nodes);
}
