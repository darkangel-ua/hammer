#pragma once
#include <string>
#include <vector>
#include <iosfwd>
#include <hammer/core/feature_ref.h>

namespace hammer {

class build_node;
class build_environment;

class argument_writer {
   public:
      argument_writer(const std::string& name)
         : name_(name)
      {}

      const std::string& name() const { return name_; }
      void write(std::ostream& output,
                 const build_node& node,
                 const build_environment& environment) const
      {
         write_impl(output, node, environment);
      }

      virtual argument_writer* clone() const = 0;
      virtual std::vector<feature_ref> valuable_features() const = 0;

      virtual ~argument_writer() {}

   protected:
      virtual void write_impl(std::ostream& output,
                              const build_node& node,
                              const build_environment& environment) const = 0;

   private:
      std::string name_;
};

inline
argument_writer*
new_clone(const argument_writer& v) {
   return v.clone();
}

}
