#pragma once
#include <vector>
#include <string>
#include <boost/noncopyable.hpp>
#include <hammer/core/feature_value_ns_fwd.h>

namespace hammer {

class feature;
class feature_registry;
class feature_def;
class basic_meta_target;
class sources_decl;

class feature_set : public boost::noncopyable {
   public:
      typedef std::vector<feature*> features_t;
      typedef features_t::const_iterator const_iterator;
      typedef features_t::iterator iterator;

      feature_set(feature_registry* fr);
      const_iterator begin() const { return features_.begin(); }
      const_iterator end() const { return features_.end(); }
      iterator begin(){ return features_.begin(); }
      iterator end() { return features_.end(); }
      const feature& get(const char* name) const; // throw if not found
      const feature* find(const char* name, const char* value) const;
      const_iterator find(const char* name) const; // find first occurrence
      const_iterator find(const std::string& name) const { return find(name.c_str()); } // find first occurrence
      const_iterator find(const feature& f) const;
      // differs from find in terms of equality operator. It uses feature::contains(), so
      // gcc and gcc-6 will match gcc-6, while find will return only gcc-6
      const_iterator contains(const feature& f) const;
      iterator find(const std::string& name) { return find(name.c_str()); }
      iterator find(const std::string& name,
                    const feature_value_ns_ptr& ns);
      iterator find(const char* name); // find first occurrence
      const_iterator find(const_iterator from, const char* name) const; // find next occurrence
      const_iterator find(const_iterator from, const std::string& name) const { return find(from, name.c_str()); }
      iterator find(iterator from, const char* name); // find next occurrence
      feature_set* join(const feature_set& rhs) const;
      feature_set& join(feature* f);
      feature_set& join(const char* name, const char* value);
      feature_set& join(const feature_set& v);
      void replace(iterator where,
                   feature* new_value);
      feature_set* clone() const;
      void copy_propagated(const feature_set& rhs);
      void erase_all(const std::string& feature_name);
      size_t size() const { return features_.size(); }
      bool empty() const { return features_.empty(); }
      void clear();
      void swap(feature_set& fs);

      bool operator == (const feature_set& rhs) const;
      bool compatible_with(const feature_set& rhs) const;
      bool operator != (const feature_set& rhs) const { return !(*this == rhs); }
      bool contains(const feature_set& rhs) const;

      feature_registry& owner() { return *fr_; }
      feature_registry& owner() const { return *fr_; }

   private:
      feature_registry* fr_;
      features_t features_;

      void join_impl(feature_set* lhs, const feature_set& rhs) const;
      iterator deconstify(const_iterator i);
      void add_composites(const feature_def& def);
};

// relative_to_target - target relative to which we adjust sources_decl path
void extract_sources(sources_decl& result,
                     const feature_set& fs,
                     const basic_meta_target& relative_to_target);
void extract_uses(sources_decl& result,
                  const feature_set& fs,
                  const basic_meta_target& relative_to_target);
void extract_dependencies(sources_decl& result,
                          const feature_set& fs,
                          const basic_meta_target& relative_to_target);
feature_set* parse_simple_set(const std::string& s, feature_registry& r);
void dump_for_hash(std::ostream& s, const feature_set& fs, bool dump_all = false);
std::string dump_for_hash(const feature_set& fs, bool dump_all = false);
std::string md5(const feature_set& fs, bool use_all);

// when construct usage requirements we need to add to all <use> build request
// been used in target constuction
void apply_build_request(feature_set& dest, const feature_set& build_request);

void append_valuable_feature(std::vector<const feature*>& result,
                             const feature& f,
                             feature_registry& f_owner);
void append_valuable_features(std::vector<const feature*>& result,
                              const feature_set& fs);
std::vector<const feature*>
make_valuable_features(const feature_set& fs);

void merge(std::vector<const feature*>& result,
           const std::vector<const feature*>& features);
}
