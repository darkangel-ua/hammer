#include "stdafx.h"
#include <vector>
#include <stdexcept>
#include <hammer/core/sources_decl.h>
#include <hammer/core/feature_set.h>


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
   return impl_->values_.begin();
}

sources_decl::iterator sources_decl::end()
{
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

void sources_decl::clone_if_needed()
{
   if (impl_->ref_counter_ > 1)
   {
      impl_t* old = impl_;
      impl_ = impl_->clone();
      --old->ref_counter_;
   }
}

void sources_decl::push_back(const pstring& v, const type_registry& tr)
{
   clone_if_needed();

   if (std::find(v.begin(), v.end(), '<') != v.end())
      throw std::runtime_error("Feature signature found in sources");

   impl_->values_.push_back(source_decl(v, pstring(), tr.resolve_from_target_name(v), NULL));
}

void sources_decl::push_back(const source_decl& v)
{
   clone_if_needed();

   if (std::find(v.target_name().begin(), v.target_name().end(), '<') != v.target_name().end())
      throw std::runtime_error("Feature signature found in sources");

   impl_->values_.push_back(v);
}

void sources_decl::insert(const std::vector<pstring>& v, const type_registry& tr)
{
   clone_if_needed();
   for(std::vector<pstring>::const_iterator i = v.begin(), last = v.end(); i != last; ++i)
           push_back(*i, tr);
}

void sources_decl::add_to_source_properties(const feature_set& props)
{
   clone_if_needed();
   // FIXME: feature_set should be ref counted to not doing stupid cloning
   for(impl_t::values_t::iterator i = impl_->values_.begin(), last = impl_->values_.end(); i != last; ++i)
      if (i->properties() == NULL)
         i->properties(props.clone());
      else 
         i->properties(const_cast<const feature_set*>(i->properties())->join(props)); 
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

}