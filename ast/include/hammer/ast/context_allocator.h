#pragma once
#include <limits>
#include <utility>
#include <hammer/ast/context.h>

namespace hammer { namespace ast {

template<typename T>
class context_allocator {
      template<typename U>
      friend class context_allocator;

   public:
      typedef T value_type;
      typedef T* pointer;
      typedef const T* const_pointer;
      typedef T& reference;
      typedef const T& const_reference;
      typedef std::size_t size_type;
      typedef std::ptrdiff_t difference_type;

      template< class U > struct rebind { typedef context_allocator<U> other; };

      context_allocator(context& ctx) : ctx_(&ctx) {}
      template<typename U>
      context_allocator(const context_allocator<U>& v) : ctx_(v.ctx_) {}

      pointer allocate(size_type n) { return static_cast<pointer>(ctx_->allocate(n * sizeof(value_type))); }
      void deallocate(T* p, std::size_t n) {}
      size_type max_size() const { return std::numeric_limits<size_type>::max() / sizeof(value_type); }
      void construct(pointer p, const_reference val) { new((void*)p) T(val); }
      template<class U> void destroy(U* p) { p->~U(); }

   private:
      context* ctx_;
};

}}
