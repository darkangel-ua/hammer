#if !defined(h_218f171e_be70_4cbd_94fa_1c513df125d0)
#define h_218f171e_be70_4cbd_94fa_1c513df125d0

#include <hammer/ast/node.h>

namespace hammer{ namespace ast{

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

#endif
