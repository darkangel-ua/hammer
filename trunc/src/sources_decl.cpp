#include "stdafx.h"
#include "sources_decl.h"
#include <vector>

namespace hammer{

struct sources_decl::impl_t
{
   impl_t() : ref_counter_(1) {}
   impl_t* clone() const;

   std::vector<source_decl> values_;
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

void sources_decl::push_back(const pstring& v)
{
   clone_if_needed();
   impl_->values_.push_back(source_decl(v, pstring(), NULL));
}

void sources_decl::push_back(const source_decl& v)
{
   clone_if_needed();
   impl_->values_.push_back(v);
}

void sources_decl::insert(const std::vector<pstring>& v)
{
   clone_if_needed();
   for(std::vector<pstring>::const_iterator i = v.begin(), last = v.end(); i != last; ++i)
	   push_back(*i);
}

sources_decl::const_iterator::const_iterator(const sources_decl& s, bool last) 
{
   if (s.impl_->values_.empty())
      i_ = 0;
   else
      i_ = (last ? &s.impl_->values_.front() + s.impl_->values_.size() : &s.impl_->values_.front());
}

void sources_decl::transfer_from(sources_decl& s)
{
   clone_if_needed();
   impl_->values_.insert(impl_->values_.end(), s.impl_->values_.begin(), s.impl_->values_.end());
   s.clear();
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

}