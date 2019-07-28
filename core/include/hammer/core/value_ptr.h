#pragma once
#include <memory>

namespace hammer {

template <typename T>
class value_ptr {
   public:
      value_ptr() noexcept = default;
      explicit value_ptr(const T& v) : value_(new T{v}) {}
      value_ptr(const value_ptr& v) : value_(v.get() ? new T{*v.get()} : nullptr) {}
      value_ptr(value_ptr&& v) noexcept = default;
      ~value_ptr() noexcept = default;

      void reset() noexcept { value_.reset(); }

      value_ptr& operator = (const T& v) { value_.reset(new T{v}); return *this; }
      value_ptr& operator = (T&& v) noexcept { value_.reset(new T{std::move(v)}); return *this; }

      value_ptr&
      operator = (const value_ptr& v) {
         if (v.get())
            *this = *v.get();
         else
            value_.reset();

         return *this;
      }
      value_ptr& operator = (value_ptr&& v) noexcept = default;

      const T* get() const noexcept { return value_.get(); }
      T* get() noexcept { return value_.get(); }

   private:
      std::unique_ptr<T> value_;
};

}
