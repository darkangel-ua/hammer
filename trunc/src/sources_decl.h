#if !defined(h_29741f53_09ff_4880_b28d_86010263122e)
#define h_29741f53_09ff_4880_b28d_86010263122e

#include "pstring.h"
#include <boost/iterator/iterator_facade.hpp>
#include "source_decl.h"

namespace hammer
{
   
	class feature_set;
	
   // @Semantic(Reference)
   class sources_decl
    {
      public:
         class const_iterator;
         friend class const_iterator;

         // ��� ������� �������� ��������� �� �����������. 
         // ��� ������ �� ��������� ��� � �� ���� �������� ��� ���������� �� ������������.
         // � ���������� �������� �������� ���� �����. �������� ���-������ ����� �������� �������.
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

         void push_back(const pstring& v, const type_registry& tr);
         void push_back(const source_decl& v);
         
         // ��������� � �����
         void insert(const std::vector<pstring>& v, const type_registry& tr);
         
         // ������������� ��� ��� ���� � s � ����� ������� ��������
         // s ����� ���������� ����
         void transfer_from(sources_decl& s);
         void add_to_source_properties(const feature_set& props);
         const_iterator begin() const { return const_iterator(*this, false); }
         const_iterator end() const { return const_iterator(*this, true); }
         void clear();
         void unique();
         bool empty() const;

      private:
         struct impl_t;
         impl_t* impl_;

         void clone_if_needed();
   };
}

#endif //h_29741f53_09ff_4880_b28d_86010263122e
