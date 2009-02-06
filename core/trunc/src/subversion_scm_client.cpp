#include "stdafx.h"
#include "subversion_scm_client.h"
#define BOOST_PROCESS_HERE
#include <boost/process.hpp>

namespace bp = boost::process ;

namespace hammer{

void subversion_scm_client::checkout(const location_t& path, const std::string& uri, bool recursive) const
{
   bp::launcher l;
   l.set_work_directory(path.native_file_string());
   l.set_stdin_behavior(bp::inherit_stream);
   l.set_stdout_behavior(bp::inherit_stream);
   l.set_stderr_behavior(bp::inherit_stream);

   bp::command_line cmd("svn.exe");
   cmd.argument("checkout");
   
   if (!recursive)
      cmd.argument("-N");
   
   cmd.argument(uri).
       argument(".");
   
   bp::status status = l.start(cmd).wait();
}

void subversion_scm_client::up(const location_t& where_up, const std::string& what_up, bool recursive) const
{
   bp::launcher l;
   l.set_work_directory(where_up.native_file_string());
   l.set_stdout_behavior(bp::inherit_stream);
   l.set_stderr_behavior(bp::inherit_stream);

   bp::command_line cmd("svn.exe");
   cmd.argument("up");

   if (!recursive)
      cmd.argument("-N");

   cmd.argument(what_up);

   bp::status status = l.start(cmd).wait();
}

}