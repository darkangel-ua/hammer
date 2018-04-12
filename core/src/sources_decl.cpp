#include "stdafx.h"
#include <vector>
#include <stdexcept>
#include <hammer/core/sources_decl.h>
#include <hammer/core/feature_set.h>
#include <utility>

namespace hammer{

struct sources_decl::impl_t
{
   impl_t() : ref_counter_(1) {}
   impl_t* clone() const;

   typedef std::vector<source_decl> values_t;
   values_t values_;
   mutable unsigned int ref_counter_;
};

sources_decl::impl_t* sources_decl::impl_t::clone() const
{
   std::auto_ptr<impl_t> result(new impl_t);

   result->values_ = values_;
   return result.release();   
}

sources_decl::sources_decl(const sources_decl& rhs) : impl_(rhs.impl_)
{
   ++impl_->ref_counter_;
}

sources_decl::const_iterator sources_decl::begin() const
{
   return impl_->values_.begin();
}

sources_decl::const_iterator sources_decl::end() const
{
   return impl_->values_.end();
}

sources_decl::iterator sources_decl::begin()
{
   clone_if_needed();
   return impl_->values_.begin();
}

sources_decl::iterator sources_decl::end()
{
   clone_if_needed();
   return impl_->values_.end();
}

sources_decl& sources_decl::operator = (const sources_decl& rhs)
{
   if (impl_ != rhs.impl_)
   {
      if (--impl_->ref_counter_ == 0)
         delete impl_;

      impl_ = rhs.impl_;
      ++impl_->ref_counter_;
   }

   return *this;
}

sources_decl::sources_decl() : impl_(new impl_t)
{

}

sources_decl::~sources_decl()
{
   if (--impl_->ref_counter_  == 0)
      delete impl_;
}

void sources_decl::clone_if_needed()
{
   if (impl_->ref_counter_ > 1)
   {
      impl_t* old = impl_;
      impl_ = impl_->clone();
      --old->ref_counter_;
   }
}

void sources_decl::push_back(const std::string& v, const type_registry& tr)
{
   clone_if_needed();
   impl_->values_.push_back(source_decl(v, std::string(), tr.resolve_from_target_name(v), NULL));
}

void sources_decl::push_back(const source_decl& v)
{
   clone_if_needed();
   impl_->values_.push_back(v);
}

void sources_decl::insert(const std::vector<std::string>& v, const type_registry& tr)
{
   clone_if_needed();
   for (auto& vv : v)
      push_back(vv, tr);
}

void sources_decl::add_to_source_properties(const feature_set& props)
{
   clone_if_needed();
   for (auto& v : impl_->values_) {
      if (v.properties() == NULL)
         v.properties(props.clone());
      else 
         v.properties(const_cast<const feature_set*>(v.properties())->join(props));
   }
}

void sources_decl::transfer_from(sources_decl& s)
{
   clone_if_needed();
   impl_->values_.insert(impl_->values_.end(), s.impl_->values_.begin(), s.impl_->values_.end());
   s.clear();
}

void sources_decl::insert(const sources_decl& s)
{
   clone_if_needed();
   impl_->values_.insert(impl_->values_.end(), s.impl_->values_.begin(), s.impl_->values_.end());
}

void sources_decl::clear()
{
   clone_if_needed();
   impl_->values_.clear();
}

void sources_decl::unique()
{
   clone_if_needed();
   sort(impl_->values_.begin(), impl_->values_.end());
   impl_->values_.erase(std::unique(impl_->values_.begin(), impl_->values_.end()), impl_->values_.end());
}

bool sources_decl::empty() const
{
   return impl_->values_.empty();
}

std::size_t sources_decl::size() const
{
   return impl_->values_.size();
}

void sources_decl::dump_for_hash(std::ostream& s) const
{
   for (auto& src : impl_->values_) {
      if (src.is_public())
         s << "@";
      s << src.target_path() << "|" << src.target_name();
      if (src.properties()) {
         s << "|";
         hammer::dump_for_hash(s, *src.properties());
      }
   }
}

}
