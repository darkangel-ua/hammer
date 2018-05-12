#pragma once
#include <hammer/ast/node.h>

namespace hammer { namespace ast {

template<typename T>
bool is_a(const node& n);

template<typename T>
bool is_a(const node* n)
{
   return is_a<T>(*n);
}

template<typename T>
const T& as(const node&);

// return NULL if types are unrelated 
template<typename T>
const T* as(const node* n);

}}
