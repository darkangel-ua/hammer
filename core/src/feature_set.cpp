#include "stdafx.h"
#include <hammer/core/feature_set.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature.h>
#include <hammer/core/location.h>
#include <hammer/core/basic_meta_target.h>
#include <hammer/core/fs_helpers.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/engine.h>
#include <iterator>
#include <stdexcept>
#include <sstream>
#include <hammer/core/sources_decl.h>
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_lists.hpp>
#include <hammer/core/subfeature.h>
#include <boost/crypto/md5.hpp>

using namespace std;

namespace hammer{

feature_set::feature_set(feature_registry* fr) : fr_(fr)
{
}

feature_set& feature_set::join(const char* name, const char* value)
{
   return join(fr_->create_feature(name, value));
}

feature_set&
feature_set::join(feature* f)
{
   if (!f->attributes().free) {
      iterator i = find(f->name(), f->get_value_ns());
      if (i != end()) {
         if ((**i).value() != f->value()) {
            *i = f;
            if (f->attributes().composite)
               f->definition().expand_composites(f->value(), this);
         }
      } else {
         features_.push_back(f);
         if (f->attributes().composite)
            f->definition().expand_composites(f->value(), this);
      }
   } else {
      if (find(*f) == end())
         features_.push_back(f);
   }
   
   return *this;
}

const feature& feature_set::get(const char* name_) const
{
   const_iterator f = find(name_);
   if (f == features_.end())
      throw runtime_error("feature '" + string(name_) + "' not founded");
   
   return **f;
}

feature_set::iterator feature_set::deconstify(const_iterator i)
{
   iterator result = features_.begin();
   std::advance(result, std::distance<feature_set::const_iterator>(features_.begin(), i));
   return result;
}

feature_set::iterator feature_set::find(const char* name)
{
   return deconstify(static_cast<const feature_set*>(this)->find(name));
}

feature_set::const_iterator feature_set::find(const char* name) const
{
   return find(features_.begin(), name);
}

feature_set::const_iterator feature_set::find(const_iterator from, const char* name) const
{
   for(features_t::const_iterator i = from, last = features_.end(); i != last; ++i)
      if ((**i).definition().name() == name )
         return i;

   return features_.end();
}

feature_set::iterator feature_set::find(iterator from, const char* name)
{
   return deconstify(static_cast<const feature_set*>(this)->find(from, name));
}

const feature* feature_set::find(const char* name, const char* value) const
{
   for(features_t::const_iterator i = features_.begin(), last = features_.end(); i != last; ++i)
      if ((**i).definition().name() == name && (**i).value() == value)
         return *i;

   return 0;
}

feature_set::iterator
feature_set::find(const std::string& name,
                  const feature_value_ns_ptr& ns)
{
   return find_if(features_.begin(), features_.end(), [&](const feature* v) {
      return v->name() == name && v->get_value_ns() == ns;
   });
}

void feature_set::join_impl(feature_set* lhs, const feature_set& rhs) const
{
   for(const_iterator i = rhs.begin(), last = rhs.end(); i != last; ++i)
      lhs->join(*i);
}

feature_set* feature_set::join(const feature_set& rhs) const
{
   assert(fr_);
   feature_set* result = fr_->make_set();
   result->features_ = features_;
   join_impl(result, rhs);
   return result;
}

feature_set& feature_set::join(const feature_set& rhs)
{
   join_impl(this, rhs);
   return *this;
}

feature_set* feature_set::clone() const
{
   feature_set* result = fr_->make_set();
   result->features_ = features_;
   return result;
}

void feature_set::copy_propagated(const feature_set& v)
{
   for(const_iterator i = v.begin(), last = v.end(); i != last; ++i)
   {
      if ((*i)->attributes().propagated)
         join(*i);
   }
}

feature_set::const_iterator feature_set::find(const feature& f) const
{
   for(const_iterator i = find(f.name()), last = end(); i != last;)
   {
      if (**i == f)
         return i;

      if (f.attributes().free)
         i = find(++i, f.name());
      else
         break;
   }

   return end();
}

feature_set::const_iterator
feature_set::contains(const feature& f) const
{
   for(const_iterator i = find(f.name()), last = end(); i != last;) {
      if ((**i).contains(f))
         return i;

      if (f.attributes().free)
         i = find(++i, f.name());
      else
         break;
   }

   return end();
}

static
void extract_dependency_like_sources(sources_decl& result,
                                     const feature_set& fs,
                                     const basic_meta_target& relative_to_target,
                                     const char* feature_name)
{
   auto i = fs.find(feature_name);
   while (i != fs.end()) {
      source_decl sd_copy = (**i).get_dependency_data().source_;

      if (!sd_copy.target_path().empty() &&
          !sd_copy.target_path_is_global() &&
          !sd_copy.is_project_local_reference() &&
          &relative_to_target.get_project() != (**i).get_path_data().project_)
      {
         const location_t full_target_path = ((**i).get_path_data().project_->location() / sd_copy.target_path()).normalize();
         const boost::filesystem::path p = relative_path(full_target_path, relative_to_target.location());
         sd_copy.target_path(p.string(), sd_copy.type());
      }

      result.push_back(sd_copy);
      i = fs.find(++i, feature_name);
   }
}

void extract_sources(sources_decl& result,
                     const feature_set& fs,
                     const basic_meta_target& relative_to_target)
{
   extract_dependency_like_sources(result, fs, relative_to_target, "source");
}

void extract_dependencies(sources_decl& result,
                          const feature_set& fs,
                          const basic_meta_target& relative_to_target)
{
   extract_dependency_like_sources(result, fs, relative_to_target, "dependency");
}

void extract_uses(sources_decl& result,
                  const feature_set& fs,
                  const basic_meta_target& relative_to_target)
{
   extract_dependency_like_sources(result, fs, relative_to_target, "use");
}

// FIXME: this is wrong. If we compare two sets with same free features that placed in different order
// than we don't detects that difference
bool feature_set::operator == (const feature_set& rhs) const
{
   if (this == &rhs)
      return true;

   if (size() != rhs.size())
      return false;

   for(features_t::const_iterator i = rhs.begin(), last = rhs.end(); i != last; ++i)
      if (find(**i) == end())
         return false;

   return true;
}

bool feature_set::compatible_with(const feature_set& rhs) const
{
   if (this == &rhs)
      return true;

   const feature_set* rhs_p = &rhs;
   const feature_set* lhs_p = this;
   
   if (size() < rhs.size())
      swap(lhs_p, rhs_p);

   for(features_t::const_iterator i = lhs_p->begin(), last = lhs_p->end(); i != last; ++i)
      if (rhs_p->find(**i) == rhs_p->end())
      {
         if ((**i).attributes().free ||
             (**i).attributes().generated ||
             (**i).attributes().no_defaults)
         {
            return false;
         }
         else
         {
            if (rhs_p->find((**i).name()) == rhs_p->end())
            {
               if (!(**i).definition().defaults_contains((**i).value()))
                  return false;
            }
            else
               return false;
         }
      }

   return true;
}

void feature_set::clear()
{
   features_.clear();
}

bool feature_set::contains(const feature_set& rhs) const
{
   if (this == &rhs)
      return true;

   for(const_iterator i = rhs.begin(), last = rhs.end(); i != last; ++i)
      if (find(**i) == end())
         return false;

   return true;
}

feature_set* parse_simple_set(const std::string& s, feature_registry& r)
{
   using namespace boost::spirit::classic;

   vector<string> feature_names, feature_values;
   if (parse(s.begin(), s.end(), 
             list_p('<' >> (+(anychar_p - '>'))[push_back_a(feature_names)] >> '>' >> 
                           (+(anychar_p - '/'))[push_back_a(feature_values)], ch_p('/'))
            ).full)
   {
      feature_set* result = r.make_set();
      for(vector<string>::const_iterator i = feature_names.begin(), v_i = feature_values.begin(), last = feature_names.end(); i != last; ++i, ++v_i)
         result->join(i->c_str(), v_i->c_str());

      return result;
  }
   else
      throw std::runtime_error("Can't parse simple feature set from '" + s + "'");
}

static bool subf_less_by_name(const subfeature* lhs, const subfeature* rhs)
{
   return lhs->name() < rhs->name();
}

static bool less_by_name(const feature* lhs, const feature* rhs)
{
   return lhs->name() < rhs->name();
}

static void dump_value(std::ostream& s, const feature& f)
{
   if (f.attributes().path) {
      if (f.attributes().generated)
         s << f.get_generated_data().target_->location();
      else {
         const feature::path_data& pd = f.get_path_data();
         location_t l(f.value());
         if (!l.has_root_name()) {
            l = pd.project_->location() / l;
            l.normalize();
         }

         s << l;
      }
   } else if (f.attributes().dependency) {
      const feature::dependency_data& dd = f.get_dependency_data();
      s << dd.source_.target_path();
      if (!dd.source_.target_name().empty())
         s << "//" << dd.source_.target_name();

      if (f.get_path_data().project_)
         s << " " << f.get_path_data().project_->location();
   } else
      s << f.value();
}

static void dump_for_hash(std::ostream& s, const feature& f)
{
   s << '<' << f.name() << '>';
   dump_value(s, f);

   if (f.subfeatures().empty())
      return;
   
   typedef vector<const subfeature*> subfeatures_t;
   subfeatures_t subfeatures;
   for(feature::subfeatures_t::const_iterator i = f.subfeatures().begin(), last = f.subfeatures().end(); i != last; ++i)
      subfeatures.push_back(*i);

   if (subfeatures.empty())
      return;

   std::sort(subfeatures.begin(), subfeatures.end(), &subf_less_by_name);
   bool first = true;

   s << '(';
   for(subfeatures_t::const_iterator i = subfeatures.begin(), last = subfeatures.end(); i != last; ++i)
   {
      if (!first)
      {
         s << ' ';
         first = false;
      }

      s << '<' << (**i).name() << '>' << (**i).value();
   }
   s << ')';
}

void dump_for_hash(std::ostream& s, const feature_set& fs, bool dump_all)
{
   if (fs.empty())
   {
      s << "<!empty!>";
      return;
   }

   typedef vector<const feature*> features_t;
   features_t features;
   for(feature_set::const_iterator i = fs.begin(), last = fs.end(); i != last; ++i)
   {
      if (dump_all || 
          !((**i).attributes().free || 
            (**i).attributes().incidental ||
            (**i).attributes().path || 
            (**i).attributes().dependency ||
            (**i).attributes().generated))
      {
         features.push_back(*i);
      }
   }
   
   std::stable_sort(features.begin(), features.end(), &less_by_name);

   bool first = true;
   for(features_t::const_iterator i = features.begin(), last = features.end(); i != last; ++i)
   {
      if (!first)
         s << (dump_all ? '\n' : ' ');
      else
         first = false;

      dump_for_hash(s, **i);
   }
}

std::string dump_for_hash(const feature_set& fs, bool dump_all)
{
   std::ostringstream s;
   dump_for_hash(s, fs, dump_all);
   return s.str();
}

void feature_set::erase_all(const std::string& feature_name)
{
   for(features_t::iterator i = features_.begin(); i != features_.end();)
   {
      if ((**i).name() == feature_name)
         i = features_.erase(i);
      else
         ++i;
   }
}

std::string
md5(const feature_set& fs, bool use_all)
{
   std::ostringstream s;
   dump_for_hash(s, fs, use_all);
   return boost::crypto::md5(s.str()).to_string();
}

void apply_build_request(feature_set& dest,
                         const feature_set& build_request)
{
   for(feature_set::iterator i = dest.begin(), last = dest.end(); i != last; ++i)
      if ((**i).name() == "use")
      {
         const source_decl old = (**i).get_dependency_data().source_;
         feature_set& new_props = old.properties() == nullptr ? *build_request.clone() : old.properties()->clone()->join(build_request);
         (**i).set_dependency_data(source_decl(old.owner_project(), old.target_path(), old.target_name(), old.type(), &new_props), (**i).get_path_data().project_);
      }
}

std::vector<const feature*>
make_valuable_features(const feature_set& fs)
{
   std::vector<const feature*> result;

   append_valuable_features(result, fs);

   return result;
}

void append_valuable_feature(std::vector<const feature*>& result,
                             const feature& f,
                             feature_registry& f_owner)
{
   if (f.attributes().free) {
      auto i = std::find_if(result.begin(), result.end(), [&](const feature* v) {
         return v->name() == f.name();
      });

      if (i == result.end())
         result.push_back(f_owner.create_feature(f.name(), {}));

   } else {
      auto i = std::find_if(result.begin(), result.end(), [&](const feature* v) {
         return *v == f;
      });

      if (i == result.end())
         result.push_back(&f);
   }
}

void append_valuable_features(std::vector<const feature*>& result,
                              const feature_set& fs)
{
   for (const feature* f : fs) {
//      FIXME: add this later
//      if (f->attributes().incidental)
//         continue;

      if (f->attributes().free) {
         auto i = std::find_if(result.begin(), result.end(), [&](const feature* v) {
            return v->name() == f->name();
         });

         if (i == result.end())
            result.push_back(fs.owner().create_feature(f->name(), {}));

      } else {
         auto i = std::find_if(result.begin(), result.end(), [&](const feature* v) {
            return *v == *f;
         });

         if (i == result.end())
            result.push_back(f);
      }
   }
}

void merge(std::vector<const feature*>& result,
           const std::vector<const feature*>& features)
{
   for (const feature* f : features) {
      auto i = std::find_if(result.begin(), result.end(), [&](const feature* v) {
         return *v == *f;
      });

      if (i == result.end())
         result.push_back(f);
   }
}

}
