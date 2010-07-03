#include "stdafx.h"
#include <hammer/parser/parser.h>

namespace hammer{namespace parser{

parser::parser(const boost::filesystem::path& hamfile,
               const sema::actions& actions)
   : hamfile_(hamfile),
     actions_(actions)
{

}

void parser::parse(const boost::filesystem::path& hamfile,
                   const sema::actions& actions)
{
   parser p(hamfile, actions);
}

void parser::parse_impl()
{

}

}}
