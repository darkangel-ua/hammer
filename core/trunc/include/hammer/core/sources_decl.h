#if !defined(h_29741f53_09ff_4880_b28d_86010263122e)
#define h_29741f53_09ff_4880_b28d_86010263122e

#include "pstring.h"
#include "source_decl.h"

namespace hammer
{
   
	class feature_set;
	
   // @Semantic(Reference)
   class sources_decl
    {
      public:
         typedef std::vector<source_decl>::iterator iterator;
         typedef std::vector<source_decl>::const_iterator const_iterator;

         sources_decl();
         sources_decl(const sources_decl& rhs);
         sources_decl& operator = (const sources_decl& rhs);

         void push_back(const pstring& v, const type_registry& tr);
         void push_back(const source_decl& v);
         
         // добавляет в конец
         void insert(const std::vector<pstring>& v, const type_registry& tr);
         void insert(const sources_decl& s);
         
         // перебрасывает все что есть в s в конец данного инстанса
         // s после переброски пуст
         void transfer_from(sources_decl& s);
         void add_to_source_properties(const feature_set& props);
         const_iterator begin() const;
         const_iterator end() const;
         iterator begin();
         iterator end();
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
