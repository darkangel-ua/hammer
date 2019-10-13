#pragma once
#include <vector>
#include <memory>
#include <boost/noncopyable.hpp>

namespace hammer {

class target_type;
class scanner;

class scanner_manager : public boost::noncopyable {
   public:
      scanner_manager();
      ~scanner_manager();

      void register_scanner(const target_type& t,
                            std::shared_ptr<scanner> scanner);
      const scanner*
      find(const target_type& t) const;

      const scanner&
      get(const target_type& t) const;

   private:
      struct impl_t;
      std::unique_ptr<impl_t> impl_;
};

}
