#pragma once
#include <vector>
#include <hammer/ast/context_allocator.h>

namespace hammer { namespace parscore {
   class identifier;
}}

namespace hammer { namespace ast {

class expression;
class statement;
class feature;

typedef std::vector<const expression*, context_allocator<const expression*> > expressions_t;
typedef std::vector<parscore::identifier, context_allocator<parscore::identifier>> identifiers_t;
typedef std::vector<const statement*, context_allocator<const statement*> > statements_t;
typedef std::vector<const feature*, context_allocator<const feature*> > features_t;
   
}}
