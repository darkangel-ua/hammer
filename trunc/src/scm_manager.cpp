#include "stdafx.h"
#include "scm_manager.h"
#include "subversion_scm_client.h"

using namespace std;

namespace hammer{

scm_manager::scm_manager()
{
   scm_clients_.insert(string("svn"), new subversion_scm_client);
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