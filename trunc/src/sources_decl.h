#pragma once
#include "pstring.h"
#include <boost/iterator/iterator_facade.hpp>

namespace hammer
{
   
	class feature_set;
	class sources_decl
    {
      public:
         class const_iterator;
         friend class const_iterator;

		 class source_decl
		 {
			public:
            source_decl() : properties_(NULL) {};
            source_decl(const pstring& target_path,
					         const pstring& target_name,
				            const feature_set* props) 
                       :
				            target_path_(target_path),
							   target_name_(target_name),
                        properties_(props)
                {}
				
				bool operator < (const source_decl& rhs) const
				{
					if (target_path_ < rhs.target_path_)
						return true;
					else
						if (target_path_ == rhs.target_path_)
							return false;
						else
							if (target_name_ < rhs.target_name_)
								return true;
							else
								return false;
				}

				bool operator == (const source_decl& rhs) const
				{
					return target_path_ == rhs.target_path_ && 
						   target_name_ == rhs.target_name_;
				}

				pstring target_path_;
				pstring target_name_;
				const feature_set* properties_;
		 };

         // это попытка спрятать контейнер за интерфейсом. 
         // Мне сильно не нравиться что я не могу спрятать тип контейрера от пользователя.
         // А реализация прятания выглядит явно криво. Возможно что-нибудь позже придумаю получше.
         class const_iterator : public boost::iterator_facade<const_iterator, source_decl const, 
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
               const source_decl& dereference() const { return *i_; }

               source_decl* i_;
         };

         sources_decl();
         sources_decl(const sources_decl& rhs);
         sources_decl& operator = (const sources_decl& rhs);

         void push_back(const pstring& v);
         void push_back(const source_decl& v);
         
         // добавляет в конец
         void insert(const std::vector<pstring>& v);
         
         // перебрасывает все что есть в s в конец данного инстанса
         // s после переброски пуст
         void transfer_from(sources_decl& s);
         
         const_iterator begin() const { return const_iterator(*this, false); }
         const_iterator end() const { return const_iterator(*this, true); }
         void clear();
         void unique();

      private:
         struct impl_t;
         impl_t* impl_;

         void clone_if_needed();
   };
}