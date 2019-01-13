#pragma once
#include <vector>
#include <hammer/core/source_decl.h>

namespace hammer {
   
class feature_set;

class sources_decl {
   public:
      using iterator = std::vector<source_decl>::iterator;
      using const_iterator = std::vector<source_decl>::const_iterator;

      sources_decl();
      ~sources_decl();
      sources_decl(const sources_decl& rhs);
      sources_decl& operator = (const sources_decl& rhs);

      void push_back(const project& related_project,
                     const std::string& v,
                     const type_registry& tr);
      void push_back(const source_decl& v);

      void insert(const project& related_project,
                  const std::vector<std::string>& v,
                  const type_registry& tr);
      void insert(const sources_decl& s);

      void transfer_from(sources_decl& s);
      void add_to_source_properties(const feature_set& props);
      const_iterator begin() const;
      const_iterator end() const;
      iterator begin();
      iterator end();
      void clear();
      void unique();
      bool empty() const;
      std::size_t size() const;

      void dump_for_hash(std::ostream& s) const;

   private:
      struct impl_t;
      impl_t* impl_;

      void clone_if_needed();
};

}
