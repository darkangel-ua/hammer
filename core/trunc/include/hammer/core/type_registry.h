#if !defined(h_4e29de28_5a4c_4974_bd81_60d334da12f7)
#define h_4e29de28_5a4c_4974_bd81_60d334da12f7

#include <boost/ptr_container/ptr_map.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include <string>
#include "pstring.h"
#include "type_tag.h"

namespace hammer
{
   class type;
   class feature_set;

   class type_registry : public boost::noncopyable
   {
      public:
         type_registry();
         // FIXME: resolve_from_name should have other name
         const type* resolve_from_suffix(const char* first, const char* last) const;
         const type* resolve_from_suffix(const std::string& suffix) const;
         const type* resolve_from_target_name(const pstring& name) const;
         const type& hard_resolve_from_target_name(const pstring& name) const;
         const type& get(const type_tag& tag) const;
         const type* find(const type_tag& tag) const;
         void insert(const type& t);
         ~type_registry();

      private:
         typedef boost::ptr_map<type_tag, type> types_t;
         typedef std::multimap<std::string, type*> types_by_suffix_t;

         types_t types_;
         types_by_suffix_t types_by_suffix_;

   };
}

#endif //h_4e29de28_5a4c_4974_bd81_60d334da12f7
