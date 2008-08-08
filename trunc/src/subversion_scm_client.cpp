#include "stdafx.h"
#include "subversion_scm_client.h"
#define BOOST_PROCESS_HERE
#include <boost/process.hpp>

namespace bp = boost::process ;

namespace hammer{

void subversion_scm_client::checkout(const location_t& path, const std::string& uri) const
{
   bp::launcher l;
   l.set_work_directory(path.native_file_string());
   l.set_stdout_behavior(bp::stream_behavior::inherit_stream);
   l.set_stderr_behavior(bp::stream_behavior::inherit_stream);

   bp::command_line cmd("svn.exe");
   cmd.argument("checkout").
       argument(uri).
       argument(".");
   
   bp::status status = l.start(cmd).wait();
}

}