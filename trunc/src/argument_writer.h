#if !defined(h_2c15e695_c37e_498d_b12a_5bd65ba5e645)
#define h_2c15e695_c37e_498d_b12a_5bd65ba5e645

#include <string>
#include <iosfwd>

namespace hammer
{
   class build_node;
   class build_environment; 

   class argument_writer
   {
      public:
         argument_writer(const std::string& name) : name_(name) {}
         const std::string& name() const { return name_; }
         virtual void write(std::ostream& output, const build_node& node, const build_environment& environment) const = 0;
         virtual argument_writer* clone() const = 0;
         virtual ~argument_writer() {}
      
      private:
         std::string name_;
   };

   inline argument_writer* new_clone(const argument_writer& v)
   {
      return v.clone();
   }
}

#endif //h_2c15e695_c37e_498d_b12a_5bd65ba5e645
