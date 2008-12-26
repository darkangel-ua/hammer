#include "stdafx.h"
#include "feature_set.h"
#include "feature_registry.h"
#include "feature.h"
#include <iterator>
#include <stdexcept>
#include "sources_decl.h"
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility/lists.hpp>
#include "subfeature.h"

using namespace std;

namespace hammer{

feature_set::feature_set(feature_registry* fr) : fr_(fr)
{
}

feature_set& feature_set::join(const char* name, const char* value)
{
   return join(fr_->create_feature(name, value));
}

feature_set& feature_set::join(feature* f)
{
   if (!f->attributes().free)
   {
      iterator i = find(f->name());
      if (i != end())
      {
         if ((**i).value() != f->value())
         {
            *i = f;
            if (f->attributes().composite)
               f->definition().expand_composites(f->value().to_string(), this);
         }
      }
      else
      {
         features_.push_back(f);
         if (f->attributes().composite)
            f->definition().expand_composites(f->value().to_string(), this);
      }
   }
   else
   {
      if (find(*f) == end())
         features_.push_back(f);
   }
   
   return *this;
}

const feature& feature_set::get(const char* name_) const
{
   const_iterator f = find(name_);
   if (f == features_.end())
      throw runtime_error("feature '" + string(name_) + "not founded");
   
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

void feature_set::join(const feature_set& rhs)
{
   join_impl(this, rhs);
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

void set_path_data(feature_set* f, const basic_meta_target* t)
{
   typedef feature_set::iterator iter;
   for(iter i = f->begin(), last = f->end(); i != last; ++i)
   {
      if ((**i).attributes().path)
         (**i).get_path_data().target_ = t;
   }
}

void extract_sources(sources_decl& result, const feature_set& fs)
{
   // FIXME: need refactor this two blocks
   feature_set::const_iterator i = fs.find("source");
   while(i != fs.end())
   {
      result.push_back((**i).get_dependency_data().source_);
      i = fs.find(++i, "source");
   }

   i = fs.find("library");
   while(i != fs.end())
   {
      result.push_back((**i).get_dependency_data().source_);
      i = fs.find(++i, "library");
   }
}

void extract_dependencies(sources_decl& result, const feature_set& fs)
{
   feature_set::const_iterator i = fs.find("dependency");
   while(i != fs.end())
   {
      result.push_back((**i).get_dependency_data().source_);
      i = fs.find(++i, "source");
   }
}

void extract_uses(sources_decl& result, const feature_set& fs)
{
   // FIXME: need refactor this block
   feature_set::const_iterator i = fs.find("use");
   while(i != fs.end())
   {
      result.push_back((**i).get_dependency_data().source_);
      i = fs.find(++i, "use");
   }
}

bool feature_set::operator == (const feature_set& rhs) const
{
   if (size() != rhs.size())
      return false;

   for(features_t::const_iterator i = rhs.begin(), last = rhs.end(); i != last; ++i)
      if (find(**i) == end())
         return false;

   return true;
}

void feature_set::clear()
{
   features_.clear();
}

bool feature_set::contains(const feature_set& rhs) const
{
   for(const_iterator i = rhs.begin(), last = rhs.end(); i != last; ++i)
      if (find(**i) == end())
         return false;

   return true;
}

feature_set* parse_simple_set(const std::string& s, feature_registry& r)
{
   using namespace boost::spirit;

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

static void dump_for_hash(std::ostream& s, const feature& f)
{
   s << '<' << f.name() << '>' << f.value();
   if (f.subfeatures().empty())
      return;
   
   typedef vector<const subfeature*> subfeatures_t;
   subfeatures_t subfeatures;
   for(feature::subfeatures_t::const_iterator i = f.subfeatures().begin(), last = f.subfeatures().end(); i != last; ++i)
      if (!(**i).attributes().incidental)
         subfeatures.push_back(*i);

   if (subfeatures.empty())
      return;

   std::sort(subfeatures.begin(), subfeatures.end(), &subf_less_by_name);
   bool first = true;

   for(subfeatures_t::const_iterator i = subfeatures.begin(), last = subfeatures.end(); i != last; ++i)
   {
      if (!first)
      {
         s << '-';
         first = false;
      }

      s << (**i).name() << ':' << (**i).value();
   }
}

void dump_for_hash(std::ostream& s, const feature_set& fs)
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
      if (!((**i).attributes().free || 
            (**i).attributes().incidental) ||
            (**i).attributes().path || 
            (**i).attributes().dependency ||
            (**i).attributes().generated)
      {
         features.push_back(*i);
      }
   }
   
   std::sort(features.begin(), features.end(), &less_by_name);

   bool first = true;
   for(features_t::const_iterator i = features.begin(), last = features.end(); i != last; ++i)
   {
      if (!first)
      {
         s << '/';
         first = false;
      }

      dump_for_hash(s, **i);
   }
}

}