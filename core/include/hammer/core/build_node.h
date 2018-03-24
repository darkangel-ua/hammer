#if !defined(h_34db5037_4bfd_4178_8169_9c21d59b821e)
#define h_34db5037_4bfd_4178_8169_9c21d59b821e

#include <vector>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <hammer/core/build_action_fwd.h>

namespace hammer
{
   class basic_build_target;
   class main_target; 
   class target_type;
   class feature_set;
   class build_node;

   // this very common type to use
	typedef boost::intrusive_ptr<build_node> build_node_ptr;
   typedef std::vector<build_node_ptr> build_nodes_t;

	class build_node : public boost::intrusive_ref_counter<build_node, boost::thread_unsafe_counter>
   {
      public:
         struct source_t
         {
            source_t(const basic_build_target* source_target,
                     build_node_ptr source_node)
                   :
                    source_target_(source_target),
                    source_node_(source_node)
            {}

            const basic_build_target* source_target_;
            boost::intrusive_ptr<build_node> source_node_;
         };
         
         typedef std::vector<source_t> sources_t;
         typedef std::vector<const basic_build_target*> targets_t;
         typedef std::vector<boost::intrusive_ptr<build_node> > nodes_t;

         build_node(const main_target& products_owner,
			           bool composite,
			           const build_action_ptr& action)
            : products_owner_(&products_owner),
              is_composite_(composite),
			     action_(action)
         {}

         const basic_build_target* find_product(const basic_build_target* t) const;
         
         boost::tribool::value_t up_to_date() const { return up_to_date_.value; }
         void up_to_date(boost::tribool::value_t v);
         
         void timestamp(const boost::posix_time::ptime v) { timestamp_ = v; }
         const boost::posix_time::ptime& timestamp() const { return timestamp_; }
         
         const build_action_ptr& action() const { return action_; }

         const feature_set& build_request() const;
         const main_target& products_owner() const { return *products_owner_; }
         bool is_composite() const { return is_composite_; }

         build_node_ptr up_;                   // owner of that node. Not used yet
         nodes_t down_;                        // all sources that came into node
         sources_t sources_;                   // sources that was consumed 
         targets_t products_;                  // targets that was produced
         nodes_t dependencies_;                
         const target_type* targeting_type_ = nullptr;  // target type that was requested for building
      
      private:
         const main_target* products_owner_;
         bool is_composite_;
         boost::tribool up_to_date_ = boost::logic::indeterminate;
         boost::posix_time::ptime timestamp_;
         const build_action_ptr action_;
   };
}

#endif //h_34db5037_4bfd_4178_8169_9c21d59b821e
