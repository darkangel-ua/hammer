#if !defined(h_34db5037_4bfd_4178_8169_9c21d59b821e)
#define h_34db5037_4bfd_4178_8169_9c21d59b821e

#include <vector>
#include <boost/intrusive_ptr.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

namespace hammer
{
   class basic_target; 
   class type;
   class build_action;
   class feature_set;

   class build_node
   {
      public:
         struct source_t
         {
            source_t(const basic_target* source_target, 
                     boost::intrusive_ptr<build_node> source_node) 
                   :
                    source_target_(source_target),
                    source_node_(source_node)
            {}

            const basic_target* source_target_;
            boost::intrusive_ptr<build_node> source_node_;
         };
         
         typedef std::vector<source_t> sources_t;
         typedef std::vector<const basic_target*> targets_t;
         typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;

         build_node() 
            : up_(0), 
              targeting_type_(0), 
              ref_counter_(0), 
              up_to_date_(boost::indeterminate), 
              action_(NULL) 
         {}

         const basic_target* find_product(const basic_target* t) const;
         
         boost::tribool::value_t up_to_date() const { return up_to_date_.value; }
         void up_to_date(boost::tribool::value_t v);
         
         void timestamp(const boost::posix_time::ptime v) { timestamp_ = v; }
         const boost::posix_time::ptime& timestamp() const { return timestamp_; }
         
         const build_action* action() const { return action_; }
         void action(const build_action* a) { action_ = a; }

         const feature_set& build_request() const;

         boost::intrusive_ptr<build_node> up_; // owner of that node. Not used yet
         nodes_t down_;                        // all sources that came into node
         sources_t sources_;                   // sources that was consumed 
         targets_t products_;                  // targets that was produced
         nodes_t dependencies_;                
         const hammer::type* targeting_type_;  // target type that was requested for building
         mutable unsigned long ref_counter_;
      
      private:
         boost::tribool up_to_date_;
         boost::posix_time::ptime timestamp_;
         const build_action* action_;
   };

   inline void intrusive_ptr_add_ref(const build_node* t)
   {
      ++t->ref_counter_;
   }

   inline void intrusive_ptr_release(const build_node* t)
   {
      if (--t->ref_counter_ == 0) delete t;
   }
}

#endif //h_34db5037_4bfd_4178_8169_9c21d59b821e
