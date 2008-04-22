#include "stdafx.h"
#include "generator.h"
#include "type.h"
#include "basic_target.h"
#include "main_target.h"
#include "meta_target.h"
#include "file_target.h"
#include "engine.h"

namespace hammer{

generator::generator(const std::string& name,
                     const consumable_types_t& source_types,
                     const producable_types_t& target_types,
                     const feature_set* c) : 
   name_(name), source_types_(source_types),
   target_types_(target_types),
   constraints_(c)
{

}

bool generator::is_consumable(const type& t) const
{
   for(generator::consumable_types_t::const_iterator i = consumable_types().begin(), last = consumable_types().end(); i != last; ++i)
      if (*i->type_ == t)
         return true;

   return false;
}
 
boost::intrusive_ptr<build_node> 
generator::construct(const type& target_type, 
                     const feature_set& props,
                     const std::vector<boost::intrusive_ptr<build_node> >& sources,
                     const basic_target* t) const
{
   if (!t)
   {
      boost::intrusive_ptr<build_node> result(new build_node);

      typedef std::vector<boost::intrusive_ptr<build_node> >::const_iterator iter;
      for(iter i = sources.begin(), last = sources.end(); i != last; ++i)
      {
         bool node_added = false;
         for(build_node::targets_t::const_iterator p_i = (**i).products_.begin(), p_last = (**i).products_.end(); p_i != p_last; ++p_i)
         {
            if (is_consumable((**p_i).type()))
            {
               result->sources_.push_back(*p_i);
               if (!node_added)
               {
                  result->down_.push_back(*i);
                  node_added = true;
               }
            }
         }
      }

      engine& e = *sources.front()->products_.front()->mtarget()->meta_target()->project()->engine();
      result->products_.push_back(new(e.targets_pool()) file_target(sources.front()->products_.front()->mtarget(), 
                                                                    pstring(e.pstring_pool(), "?"), 
                                                                    producable_types().front().type_, &props));
      return result;
   }
   else
   {
      assert(sources.size() == 1);
      
      boost::intrusive_ptr<build_node> result(new build_node);
      result->sources_.push_back(t);
      result->down_.push_back(sources.front());
      engine& e = *t->mtarget()->meta_target()->project()->engine();
      result->products_.push_back(new(e.targets_pool()) file_target(t->mtarget(), pstring(e.pstring_pool(), "?"), producable_types().front().type_, &props));
      
      return result;
   }
}

}