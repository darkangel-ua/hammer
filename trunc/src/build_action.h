#if !defined(h_ee7eef40_46e2_4a70_bb84_999ada9c5565)
#define h_ee7eef40_46e2_4a70_bb84_999ada9c5565

namespace hammer
{
   class build_node;
   class build_environment;

   class build_action
   {
      public:
         void execute(const build_node& node, const build_environment& environment) const { execute_impl(node, environment); }
         virtual ~build_action() {}

      protected:
         virtual void execute_impl(const build_node& node, const build_environment& environment) const  = 0;
   };
}

#endif //h_ee7eef40_46e2_4a70_bb84_999ada9c5565
