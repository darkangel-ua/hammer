#pragma once

namespace hammer{ namespace ast{

class hamfile;

}}

namespace hammer {

class project;
class invocation_context;

void ast2objects(invocation_context& ctx,
                 const ast::hamfile& node);

}
