#pragma once

namespace hammer { namespace ast {

class visitor;

class node {
   public:
      virtual bool accept(visitor& v) const = 0;
      virtual ~node() {}
};

}}
