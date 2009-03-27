#include "stdafx.h"
#include <hammer/core/scm_manager.h>
#include <hammer/core/subversion_scm_client.h>

using namespace std;

namespace hammer{

scm_manager::scm_manager()
{
   std::auto_ptr<scm_client> new_client(new subversion_scm_client);
   scm_clients_.insert(string("svn"), new_client);
}

const scm_client* scm_manager::find(const std::string& id) const
{
   scm_clients::const_iterator i = scm_clients_.find(id);
   if (i != scm_clients_.end())
      return i->second;
   else
      return NULL;
}

}