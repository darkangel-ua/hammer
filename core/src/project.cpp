#include "stdafx.h"
#include <hammer/core/project.h>
#include <hammer/core/engine.h>
#include <hammer/core/feature_registry.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <boost/format.hpp>
#include <sstream>

using namespace std;

namespace hammer{

project::project(hammer::engine* e,
                 const std::string& name,
                 const location_t& location,
                 const requirements_decl& req,
                 const requirements_decl& usage_req)
   :
    basic_meta_target(this, name, req, usage_req),
    engine_(e)
{
   this->location(location);
}

void project::location(const location_t& l)
{
   // all project paths must end on dot
   std::string path = l.string();
   if (!path.empty() && *path.rbegin() != '.')
   {
      location_ = l / ".";
      location_.normalize();
   }
   else
      location_ = l;
}


void project::add_target(std::unique_ptr<basic_meta_target> t)
{
   targets_.insert(std::make_pair(t->name(), move(t)));
}

const basic_meta_target* project::find_target(const std::string& name) const
{
   return const_cast<project*>(this)->find_target(name);
}

basic_meta_target* project::find_target(const std::string& name)
{
   auto i = targets_.find(name);
   if (i == targets_.end())
      return 0;
   else
      return i->second.get();
}

// -1 == not suitable
// > -1 == suitable with computed rank. Zero IS valid rank
static
int compute_alternative_rank(const feature_set& target_properties,
                             const feature_set& build_request)
{
   unsigned rank = 0;
   for(const feature* tf : target_properties) {
      if (!(tf->attributes().free ||
            tf->attributes().incidental ||
            tf->attributes().undefined_))
      {
         feature_set::const_iterator bf = build_request.find(tf->name());
         if (bf != build_request.end())
            if (tf->value() != (**bf).value())
               return -1;
            else
               ++rank;
         else {
            // feature is not in build_request
            if (tf->name() == "override")
               rank += 10000;
            else
               rank += tf->definition().defaults_contains(tf->value());
         }
      }
   }

   return rank;
}

project::selected_target
project::select_best_alternative(const std::string& target_name,
                                 const feature_set& build_request,
                                 const bool allow_locals) const
{

   selected_target result = try_select_best_alternative(target_name, build_request, allow_locals);
   if (result.target_ == NULL)
      throw std::runtime_error("Can't select alternative for target '" + target_name + "'.");

   return result;
}

static
bool s_great(const project::selected_target& lhs,
             const project::selected_target& rhs)
{
   return lhs.resolved_requirements_rank_ > rhs.resolved_requirements_rank_;
}

#if !defined(_WIN32)
[[ noreturn ]]
#endif
static
void error_cannot_choose_alternative(const project& p,
                                     const std::string& target_name,
                                     const feature_set& build_request_param)
{
   auto fmt = boost::format("Failed to selecting best alternatives for target '%s' in project '%s'\n"
                            "Build request: %s\n");

   stringstream s;
   dump_for_hash(s, build_request_param);
   throw std::runtime_error((fmt % target_name % p.location()
                                 % s.str()).str());
}

static
int compute_requirements_rank(const feature_set& requirements)
{
   int rank = 0;
   for (const feature* f : requirements) {
      if (f->attributes().free ||
          f->attributes().incidental ||
          f->attributes().undefined_)
         continue;

      if (!f->definition().defaults_contains(f->value()))
          return -1;

      ++rank;
   }

   return rank;
}

project::selected_target
project::try_select_best_alternative(const std::string& target_name,
                                     const feature_set& build_request_param,
                                     const bool allow_locals) const
{
   const feature_set& build_request = build_request_param.has_undefined_features()
                                         ? *try_resolve_local_features(build_request_param)
                                         : build_request_param;

   boost::iterator_range<targets_t::const_iterator> r = targets_.equal_range(target_name);

   if (r.empty())
      throw std::runtime_error("Can't find target '" + target_name + "'");

   vector<selected_target> selected_targets;

   for(targets_t::const_iterator first = r.begin(), last = r.end(); first != last; ++first) {
      if (!allow_locals && first->second->is_local())
         continue;

      feature_set* fs = engine_->feature_registry().make_set();
      first->second->requirements().eval(build_request, fs);
      int rank = compute_alternative_rank(*fs, build_request);
      if (rank != -1)
         selected_targets.push_back(selected_target(first->second.get(), fs, rank));
   }

   sort(selected_targets.begin(), selected_targets.end(), s_great);
   if (selected_targets.empty())
      return selected_target();

   if (selected_targets.size() == 1)
      return selected_targets.front();

   if (selected_targets[0].resolved_requirements_rank_ != selected_targets[1].resolved_requirements_rank_)
      return selected_targets.front();

   // okay, ranks are same, so we need to find target with best computed requirements
   vector<selected_target> selected_targets_2;
   for (const selected_target& t : selected_targets) {
      const int rank = compute_requirements_rank(*t.resolved_requirements_);
      if (rank >= 0)
         selected_targets_2.push_back(selected_target(t.target_, t.resolved_requirements_, static_cast<unsigned>(rank)));
   }

   sort(selected_targets_2.begin(), selected_targets_2.end(), s_great);

   if (selected_targets_2.size() == 1)
      return selected_targets_2.front();

   if (!selected_targets_2.empty() &&
       selected_targets_2[0].resolved_requirements_rank_ != selected_targets_2[1].resolved_requirements_rank_)
   {
      return selected_targets_2.front();
   } else
      error_cannot_choose_alternative(*this, target_name, build_request_param);
}

void project::instantiate(const std::string& target_name,
                          const feature_set& build_request,
                          std::vector<basic_target*>* result) const
{
   selected_target best_target = select_best_alternative(target_name, build_request);
   feature_set* usage_requirements = engine_->feature_registry().make_set();
   best_target.target_->instantiate(0, build_request, result, usage_requirements);
}

void project::instantiate_impl(const main_target* owner,
                               const feature_set& build_request,
                               std::vector<basic_target*>* result,
                               feature_set* usage_requirements) const
{
   assert(false && "not implemented.");
}

bool project::operator == (const project& rhs) const
{
   return this == &rhs;
}

project::selected_targets_t
project::select_best_alternative(const feature_set& build_request) const
{
   selected_targets_t result;

   targets_t::const_iterator first = targets_.begin(), last = targets_.end();
   while(first != last)
   {
      selected_target t = try_select_best_alternative(first->second->name(), build_request);
      if (t.target_ != NULL)
         result.push_back(t);

      // we just processed targets with name equal to 'first->second->name()', lets move to
      // group of targets with different name
      targets_t::const_iterator next = first; std::advance(next, 1);
      while(next != last && first->first == next->first)
         ++first, ++next;

      first = next;
   }

   return result;
}

feature_set*
project::try_resolve_local_features(const feature_set& fs) const
{
   feature_set* result = engine_->feature_registry().make_set();
   for(feature_set::const_iterator i = fs.begin(), last = fs.end(); i != last; ++i)
   {
      if ((**i).attributes().undefined_)
      {
         const feature_def* def = local_feature_registry_.find_def_from_full_name((**i).name().c_str());
         if (def != NULL)
            result->join(local_feature_registry_.create_feature((**i).name(), (**i).value()));
         else
            result->join(*i);
      }
      else
         result->join(*i);
   }

   return result;
}

}
