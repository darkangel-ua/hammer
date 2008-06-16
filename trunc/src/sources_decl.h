#pragma once
#include "pstring.h"
#include <boost/iterator/iterator_facade.hpp>

namespace hammer
{
   class sources_decl
   {
      public:
         class const_iterator;
         friend class const_iterator;

         // это попытка спрятать контейнер за интерфейсом. 
         // Мне сильно не нравиться что я не могу спрятать тип контейрера от пользователя.
         // А реализация прятания выглядит явно криво. Возможно что-нибудь позже придумаю получше.
         class const_iterator : public boost::iterator_facade<const_iterator, pstring const, 
                                                              boost::forward_traversal_tag>
         {
            public:
               const_iterator(const sources_decl& s, bool last);

            private:
               friend class boost::iterator_core_access;
               void increment() { ++i_; }
               bool equal(const const_iterator& other) const
               {
                  return this->i_ == other.i_;
               }
               const pstring& dereference() const { return *i_; }

               pstring* i_;
         };

         sources_decl();
         sources_decl(const sources_decl& rhs);
         sources_decl& operator = (const sources_decl& rhs);

         void push_back(const pstring& v);
         // добавляет в конец
         void insert(const std::vector<pstring>& v);
         const_iterator begin() const { return const_iterator(*this, false); }
         const_iterator end() const { return const_iterator(*this, true); }

      private:
         struct impl_t;
         impl_t* impl_;
   };
}