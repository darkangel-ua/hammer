#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace hammer {

class build_environment;
class argument_writer;
class feature;
class build_node;

class cmdline_builder {
   public:
      typedef std::map<const std::string, std::shared_ptr<argument_writer> > writers_t;

      cmdline_builder(const std::string& cmd);

      template<typename T>
      cmdline_builder&
      operator +=(std::shared_ptr<T> v) { add(std::move(v)); return *this; }

      void write(std::ostream& output,
                 const build_node& node,
                 const build_environment& environment) const;

      const writers_t&
      writers() const { return writers_; }

      void writers(const writers_t& w) { writers_ = w; }

      std::vector<const feature*>
      valuable_features() const;

   private:
      std::string cmd_;
      writers_t writers_;

      void add(std::shared_ptr<argument_writer> v);
};

}
