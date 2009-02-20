#if !defined(h_d7ff7f5f_e931_49f6_9c2f_e8bc7f3b0fde)
#define h_d7ff7f5f_e931_49f6_9c2f_e8bc7f3b0fde

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <vector>

namespace hammer
{
   class type;
   class scanner;

   class scanner_manager : public boost::noncopyable
   {
      public:
         scanner_manager(); 
         void register_scanner(const type& t, const boost::shared_ptr<scanner>& scanner);
         const scanner* find(const type& t) const;
         const scanner& get(const type& t) const;

      private:
         struct impl_t;
         boost::shared_ptr<impl_t> impl_;
   };
}

#endif //h_d7ff7f5f_e931_49f6_9c2f_e8bc7f3b0fde
