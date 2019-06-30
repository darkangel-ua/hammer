#pragma once
#include <memory>
#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <hammer/core/type_tag.h>

namespace hammer {

class target_type;
class feature_set;

class type_registry : public boost::noncopyable {
   public:
      type_registry();
      // FIXME: resolve_from_name should have other name
      const target_type* resolve_from_suffix(const char* first, const char* last) const;
      const target_type* resolve_from_suffix(const std::string& suffix) const;
      const target_type* resolve_from_target_name(const std::string& name) const;
      const target_type& hard_resolve_from_target_name(const std::string& name) const;
      const target_type& soft_resolve_from_target_name(const std::string& name) const;
      const target_type& get(const type_tag& tag) const;
      const target_type* find(const type_tag& tag) const;
      const target_type& insert(const target_type& t);
      ~type_registry();

   private:
      typedef std::map<type_tag, std::unique_ptr<target_type>> types_t;
      typedef std::multimap<std::string, target_type*> types_by_suffix_t;

      types_t types_;
      types_by_suffix_t types_by_suffix_;

};

}
