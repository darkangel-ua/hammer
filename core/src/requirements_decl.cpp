#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/requirements_decl.h>

namespace hammer {

struct requirements_decl::impl_t {
   typedef std::vector<std::unique_ptr<requirement_base>> requirements_t;
   typedef requirements_t::const_iterator const_iterator;
   typedef requirements_t::iterator iterator;

   impl_t* clone() const;

   requirements_t requirements_;
};

requirements_decl::impl_t*
requirements_decl::impl_t::clone() const {
   std::unique_ptr<impl_t> result{new impl_t};

   result->requirements_.reserve(requirements_.size());
   for (auto& r : requirements_)
      result->requirements_.push_back(r->clone());

   return result.release();
}

requirements_decl::requirements_decl()
   : impl_(new impl_t)
{
}

requirements_decl::requirements_decl(const requirements_decl& rhs)
   : impl_(rhs.impl_->clone())
{
}

requirements_decl&
requirements_decl::operator = (const requirements_decl& rhs) {
   if (impl_ != rhs.impl_) {
      impl_t* new_impl{rhs.impl_->clone()};
      delete impl_;
      impl_ = new_impl;
   }

   return *this;
}

void requirements_decl::add(std::unique_ptr<requirement_base> r) {
   impl_->requirements_.push_back(std::move(r));
}

void requirements_decl::add(const feature& f)
{
   add(boost::make_unique<just_feature_requirement>(const_cast<feature*>(&f)));
}

requirements_decl::~requirements_decl() {
   delete impl_;
}

void requirements_decl::eval(const feature_set& build_request, 
                             feature_set* result,
                             feature_set* public_result) const
{
   for (auto& r : impl_->requirements_)
      r->eval(build_request, result, public_result);
}

void linear_and_condition::eval(const feature_set& build_request,
                                feature_set* result,
                                feature_set* public_result) const
{
   bool satisfy = true;
   for(features_t::const_iterator i = features_.begin(), last = features_.end(); i != last; ++i)
   {
      if (build_request.contains(**i) != build_request.end() ||
          result->contains(**i) != result->end())
      {
         continue;
      }
      else
         if (build_request.find((**i).name()) != build_request.end() ||
             result->find((**i).name()) != result->end() ||
             !(**i).definition().defaults_contains((**i).value()))
         {
            satisfy = false;
            break;
         }
   }

   if (satisfy)
   {
      result->join(result_);

      if (is_public() && public_result)
         public_result->join(result_);
   }
}

void just_feature_requirement::eval(const feature_set& build_request,
                                    feature_set* result,
                                    feature_set* public_result) const
{
   result->join(f_);
   
   if (is_public() && public_result)
      public_result->join(f_);
}

void linear_and_condition::add(feature* c)
{
   features_.push_back(c);
}

void requirements_decl::insert_infront(const requirements_decl& v)
{
   for (auto& i : v.impl_->requirements_)
      impl_->requirements_.insert(impl_->requirements_.begin(), i->clone());
}

void requirements_decl::insert(const requirements_decl& v)
{
   for (auto& i : v.impl_->requirements_)
      impl_->requirements_.push_back(i->clone());
}

bool requirements_decl::empty() const
{
   return impl_->requirements_.empty();
}

requirement_condition::requirement_condition(std::unique_ptr<requirement_condition_op_base> cond,
                                             const result_t& result,
                                             const bool public_)
   : requirement_base(public_),
     cond_(std::move(cond)),
     result_(result)
{}

void requirement_condition::eval(const feature_set& build_request,
                                 feature_set* result,
                                 feature_set* public_result) const
{
   if (cond_->eval(build_request, result)) {
      for(const result_element& re : result_) {
         result->join(re.f_);

         if ((is_public() || re.public_) && public_result)
            public_result->join(re.f_);
      }
   }
}

std::unique_ptr<requirement_base>
requirement_condition::clone() const
{
   return boost::make_unique<requirement_condition>(cond_->clone(), result_, is_public());
}

bool requirement_condition_op_feature::eval(const feature_set& build_request,
                                            feature_set* result) const
{
   if (build_request.contains(*f_) != build_request.end() || result->contains(*f_) != result->end())
      return true;
   else {
      if (build_request.find(f_->name()) != build_request.end() ||
          result->find(f_->name()) != result->end() ||
          !f_->definition().defaults_contains(f_->value()))
      {
         return false;
      } else
         return true;
   }
}

}
