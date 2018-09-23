#pragma once
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

namespace hammer {

class target_type;
class scanner;

class scanner_manager : public boost::noncopyable {
   public:
      scanner_manager();
      void register_scanner(const target_type& t, const boost::shared_ptr<scanner>& scanner);
      const scanner* find(const target_type& t) const;
      const scanner& get(const target_type& t) const;

   private:
      struct impl_t;
      boost::shared_ptr<impl_t> impl_;
};

}
