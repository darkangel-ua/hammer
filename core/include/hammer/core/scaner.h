#pragma once
#include <string>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/shared_ptr.hpp>
#include <hammer/core/scaner_context.h>

namespace hammer {

class basic_build_target;
class build_node;
class build_environment;

class scanner {
   public:
      scanner(const std::string& name) : name_(name)
      {}

      virtual
      boost::posix_time::ptime
      process(const basic_build_target& t,
              const build_node& node, // t in node.products
              scanner_context& context) const = 0;

      virtual
      boost::shared_ptr<scanner_context>
      create_context(const build_environment& env) const = 0;

      const std::string& name() const { return name_; }
      virtual ~scanner() {}

   private:
      std::string name_;
};

}
