#include "stdafx.h"
#include "engine.h"
#include "hammer_walker_context.h"
#include "type_registry.h"
#include "types.h"
#include "basic_target.h"

extern "C"
{
#  include "build/hammerLexer.h"
#  include "build/hammerParser.h"
#  include "build/hammer_walker.h"
};

using namespace std;

#undef LIB
#undef EXE

namespace hammer{

   engine::engine(const boost::filesystem::path& root_path) 
      : root_path_(root_path)
   {
      type_registry_.reset(new type_registry);
      auto_ptr<type> cpp(new type(types::CPP));
      type_registry_->insert(cpp);
      auto_ptr<type> h(new type(types::H));
      type_registry_->insert(h);
      auto_ptr<type> lib(new type(types::LIB));
      type_registry_->insert(lib);
      auto_ptr<type> shared_lib(new type(types::SHARED_LIB));
      type_registry_->insert(shared_lib);
      auto_ptr<type> exe(new type(types::EXE));
      type_registry_->insert(exe);
   }

   static void parse_file(const char* file_name, hammer_walker_context* ctx)
   {
      pANTLR3_INPUT_STREAM    input;
      phammerLexer		      lexer;
      pANTLR3_COMMON_TOKEN_STREAM	    tstream;
      phammerParser				parser;
      hammerParser_project_return	    langAST;
      pANTLR3_COMMON_TREE_NODE_STREAM	nodes;
      phammer_walker          hammer_walker;
      input	= antlr3AsciiFileStreamNew((pANTLR3_UINT8)file_name);
      lexer = hammerLexerNew(input);
      tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, lexer->pLexer->tokSource);
      parser = hammerParserNew(tstream);
      langAST = parser->project(parser);
      if (parser->pParser->rec->errorCount > 0)
      {
         fprintf(stderr, "The parser returned %d errors, tree walking aborted.\n", parser->pParser->rec->errorCount);
         throw std::runtime_error("parser error");
      }

      nodes	= antlr3CommonTreeNodeStreamNewTree(langAST.tree, ANTLR3_SIZE_HINT); // sIZE HINT WILL SOON BE DEPRECATED!!
      hammer_walker = hammer_walkerNew(nodes);
      hammer_walker->pTreeParser->super = ctx;
      hammer_walker->project(hammer_walker);

      nodes   ->free  (nodes);	    nodes	= NULL;
      parser	    ->free  (parser); parser = NULL;
      tstream ->free  (tstream);	    tstream = NULL;
      lexer	    ->free  (lexer);	    lexer = NULL;
      input   ->close (input);	    input = NULL;
   }

   const project& engine::load_project(const location_t& project_path)
   {
      hammer_walker_context ctx;
      ctx.engine_ = this;
      ctx.location_ = project_path;

      parse_file((root_path_ / project_path / "jamfile").native_file_string().c_str(), &ctx);

      return *ctx.project_;
   }

   void engine::insert(project* p)
   {
      projects_.insert(p->location(), p);
   }
   
   engine::~engine()
   {

   }
}