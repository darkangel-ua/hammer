#include "stdafx.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include "requirements_decl.h"
#include "feature_set.h"
#include "feature.h"
#include "feature_registry.h"

namespace hammer{

struct requirements_decl::impl_t
{
   impl_t() : ref_counter_(1) {}   
   
   typedef boost::ptr_vector<requirement_base> requirements_t;
   typedef requirements_t::const_iterator const_iterator;
   typedef requirements_t::iterator iterator;

   impl_t* clone() const;
   requirements_t requirements_;
   mutable unsigned int ref_counter_;
};

requirements_decl::impl_t* requirements_decl::impl_t::clone() const
{
   std::auto_ptr<impl_t> result(new impl_t);

   for(requirements_t::const_iterator i = requirements_.begin(), last = requirements_.end(); i != last; ++i)
      result->requirements_.push_back(i->clone()); //  FIX: memory leak

   return result.release();
}

requirements_decl::requirements_decl() : impl_(new impl_t)
{
}

requirements_decl::requirements_decl(const requirements_decl& rhs) : impl_(rhs.impl_)
{
   ++impl_->ref_counter_;
}

requirements_decl& requirements_decl::operator = (const requirements_decl& rhs)
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

void requirements_decl::add(std::auto_ptr<requirement_base> r)
{
   if (impl_->ref_counter_ > 1)
   {
      impl_t* old = impl_;
      impl_ = impl_->clone();
      --old->ref_counter_;
   }

   impl_->requirements_.push_back(r);
}

// FIXME: это великий хак. Необходимо срочно переделать работу класса feature 
// на COW вариант ибо невозможно нормально работать
void requirements_decl::add(const feature& f)
{
   std::auto_ptr<requirement_base> r(new just_feature_requirement(const_cast<feature*>(&f)));
   add(r);
}

requirements_decl::~requirements_decl()
{
   if (--impl_->ref_counter_ == 0)
      delete impl_;
}

void requirements_decl::eval(const feature_set& build_request, 
                             feature_set* result,
                             feature_set* public_result) const
{
   for(impl_t::requirements_t::const_iterator i = impl_->requirements_.begin(), last = impl_->requirements_.end(); i != last; ++i)
      i->eval(build_request, result, public_result);
}

void linear_and_condition::eval(feature_set* result, 
                                feature_registry& fr) const
{
   bool satisfy = true;
   for(features_t::const_iterator i = features_.begin(), last = features_.end(); i != last; ++i)
   {
      feature_set::const_iterator f = result->find(**i);
      satisfy = satisfy && (f != result->end() ||
                            (f == result->end() && 
                             fr.get_def((**i).name()).get_default() == (**i).value()));
   }

   if (satisfy)
      result->join(result_);
}

void linear_and_condition::eval(const feature_set& build_request,
                                feature_set* result,
                                feature_set* public_result) const
{
   bool satisfy = true;
   for(features_t::const_iterator i = features_.begin(), last = features_.end(); i != last; ++i)
   {
      if (build_request.find(**i) != build_request.end() ||
          result->find(**i) != result->end())
      {
         continue;
      }
      else
         if (build_request.find((**i).name()) != build_request.end() ||
             result->find((**i).name()) != result->end() ||
             (**i).definition().get_default() != (**i).value())
         {
            satisfy = false;
            break;
         }
   }

   if (satisfy)
   {
      result->join(result_);
      
      if (is_public() && public_result != NULL)
         public_result->join(result_);
   }
}

void linear_and_condition::setup_path_data(const basic_meta_target* t)
{
   if (result_->attributes().path || result_->attributes().dependency)
      result_->get_path_data().target_ = t;
}

void just_feature_requirement::eval(feature_set* result, 
                                    feature_registry& fr) const
{
   result->join(f_);
}

void just_feature_requirement::eval(const feature_set& build_request,
                                    feature_set* result,
                                    feature_set* public_result) const
{
   result->join(f_);
   
   if (is_public() && public_result != NULL)
      public_result->join(f_);

}

void just_feature_requirement::setup_path_data(const basic_meta_target* t)
{
   if (f_->attributes().path || f_->attributes().dependency)
      f_->get_path_data().target_ = t;
}

void linear_and_condition::add(feature* c)
{
   features_.push_back(c);
}

void requirements_decl::setup_path_data(const basic_meta_target* t)
{
   for(impl_t::iterator i = impl_->requirements_.begin(), last = impl_->requirements_.end(); i != last; ++i)
      i->setup_path_data(t);
}

void requirements_decl::insert_infront(const requirements_decl& v)
{
   for(impl_t::const_iterator i = v.impl_->requirements_.begin(), last = v.impl_->requirements_.end(); i!= last; ++i)
      impl_->requirements_.insert(impl_->requirements_.begin(), i->clone());
}

void requirements_decl::insert(const requirements_decl& v)
{
   for(impl_t::const_iterator i = v.impl_->requirements_.begin(), last = v.impl_->requirements_.end(); i!= last; ++i)
      impl_->requirements_.insert(impl_->requirements_.end(), i->clone());
}

}