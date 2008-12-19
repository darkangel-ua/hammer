#if !defined(h_ee7eef40_46e2_4a70_bb84_999ada9c5565)
#define h_ee7eef40_46e2_4a70_bb84_999ada9c5565

#include <string>

namespace hammer
{
   class build_node;
   class build_environment;

   class build_action
   {
      public:
         build_action(const std::string& name) : name_(name) {}
         const std::string& name() const { return name_; }
         void execute(const build_node& node, const build_environment& environment) const;
         virtual std::string target_tag(const build_node& node, const build_environment& environment) const = 0;

         virtual ~build_action();

      protected:
         virtual bool execute_impl(const build_node& node, const build_environment& environment) const  = 0;
      
      private:
         std::string name_;
   };
}

#endif //h_ee7eef40_46e2_4a70_bb84_999ada9c5565
