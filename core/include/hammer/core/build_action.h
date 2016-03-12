#if !defined(h_ee7eef40_46e2_4a70_bb84_999ada9c5565)
#define h_ee7eef40_46e2_4a70_bb84_999ada9c5565

#include <string>
#include <boost/shared_ptr.hpp>

namespace hammer
{
   class build_node;
   class build_environment;
   class basic_target;

   class build_action
   {
      public:
         build_action(const std::string& name) : name_(name) {}
         const std::string& name() const { return name_; }
         bool execute(const build_node& node, const build_environment& environment) const;
         virtual std::string target_tag(const build_node& node, const build_environment& environment) const = 0;

         void batched_action(const boost::shared_ptr<build_action>& ba) { batched_action_ = ba; }
         const build_action* batched_action() const { return batched_action_.get(); }

         virtual ~build_action();

      protected:
         virtual bool execute_impl(const build_node& node, const build_environment& environment) const = 0;
         virtual void clean_on_fail(const build_node& node, const build_environment& environment) const;
      
      private:
         std::string name_;
         boost::shared_ptr<build_action> batched_action_;
   };
}

#endif //h_ee7eef40_46e2_4a70_bb84_999ada9c5565
