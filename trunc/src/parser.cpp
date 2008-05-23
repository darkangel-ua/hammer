#include "stdafx.h"
#include "parser.h"

using namespace std;

namespace hammer{

   parser::parser() : input_(0), lexer_(0),
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
      tstream_ = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, lexer_->pLexer->tokSource);
      parser_ = hammerParserNew(tstream_);
      langAST_ = parser_->rules(parser_);
      return parser_->pParser->rec->errorCount == 0 && 
		       lexer_->pLexer->rec->error == 0;
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