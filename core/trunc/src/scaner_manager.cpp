#include "stdafx.h"
#include <hammer/core/scaner_manager.h>
#include <boost/unordered_map.hpp>
#include <hammer/core/type.h>
#include <hammer/core/scaner.h>

namespace hammer{

struct scanner_manager::impl_t
{
   typedef boost::unordered_map<const type*, 
                                boost::shared_ptr<scanner> > scanners_t;
   scanners_t scanners_;
};

scanner_manager::scanner_manager()
   : impl_(new impl_t)
{

}

void scanner_manager::register_scanner(const type& t, const boost::shared_ptr<scanner>& scanner)
{
   if (const hammer::scanner* s = find(t))
      throw std::runtime_error("Scanner '" + s->name() + "' for type '" + t.tag().name() + "' already registered.");

   impl_->scanners_.insert(std::make_pair(&t, scanner));
}

const scanner* scanner_manager::find(const type& t) const
{
   impl_t::scanners_t::const_iterator i = impl_->scanners_.find(&t);
   if (i == impl_->scanners_.end())
      return NULL;
   else
      return i->second.get();
}

const scanner& scanner_manager::get(const type& t) const
{
   const scanner* result = find(t);
   if (result == NULL)
      throw std::runtime_error("No scanner for type '" + t.tag().name() + "'.");
   else
      return *result;
}

}

