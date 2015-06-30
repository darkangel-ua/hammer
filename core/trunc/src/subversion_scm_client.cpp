#include "stdafx.h"
#include <hammer/core/subversion_scm_client.h>
#include <boost/process.hpp>

namespace bp = boost::process ;

namespace hammer{

void subversion_scm_client::checkout(const location_t& path, const std::string& uri, bool recursive) const
{
   bp::context ctx;
   ctx.environment = bp::self::get_environment();
   ctx.work_directory = path.string();
   ctx.stdin_behavior = bp::inherit_stream();
   ctx.stdout_behavior = bp::inherit_stream();
   ctx.stderr_behavior = bp::inherit_stream();

   std::vector<std::string> args;
   args.push_back("svn");
   args.push_back("checkout");
   
   if (!recursive)
      args.push_back("-N");
   
   args.push_back(uri);
   args.push_back(".");
   
   bp::status status = bp::launch(std::string(), args, ctx).wait();
}

void subversion_scm_client::up(const location_t& where_up, const std::string& what_up, bool recursive) const
{
   bp::context ctx;
   ctx.environment = bp::self::get_environment();
   ctx.work_directory = where_up.string();
   ctx.stdout_behavior = bp::inherit_stream();
   ctx.stderr_behavior = bp::inherit_stream();

   std::vector<std::string> cmd;
   cmd.push_back("svn");
   cmd.push_back("up");

   if (!recursive)
      cmd.push_back("-N");

   cmd.push_back(what_up);

   bp::status status = bp::launch(std::string(), cmd, ctx).wait();
}

}
