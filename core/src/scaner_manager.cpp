#include <hammer/core/scaner_manager.h>
#include <boost/unordered_map.hpp>
#include <hammer/core/target_type.h>
#include <hammer/core/scaner.h>

namespace hammer {

struct scanner_manager::impl_t
{
   typedef boost::unordered_map<const target_type*, 
                                std::shared_ptr<scanner> > scanners_t;
   scanners_t scanners_;
};

scanner_manager::scanner_manager()
   : impl_(new impl_t)
{

}

scanner_manager::~scanner_manager() = default;

void scanner_manager::register_scanner(const target_type& t,
                                       std::shared_ptr<scanner> scanner)
{
   if (const hammer::scanner* s = find(t))
      throw std::runtime_error("Scanner '" + s->name() + "' for type '" + t.tag().name() + "' already registered.");

   impl_->scanners_.insert({&t, std::move(scanner)});
}

const scanner*
scanner_manager::find(const target_type& t) const
{
   impl_t::scanners_t::const_iterator i = impl_->scanners_.find(&t);
   if (i == impl_->scanners_.end())
      return nullptr;
   else
      return i->second.get();
}

const scanner&
scanner_manager::get(const target_type& t) const
{
   const scanner* result = find(t);
   if (!result)
      throw std::runtime_error("No scanner for type '" + t.tag().name() + "'.");
   else
      return *result;
}

}

