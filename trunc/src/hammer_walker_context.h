#pragma once
#include <boost/filesystem/path.hpp>

namespace hammer
{
   class engine;
   class project;
   class call_resolver;

   struct hammer_walker_context
   {
      hammer_walker_context() : project_(0) {}
      
      engine* engine_;
      call_resolver* call_resolver_;
      boost::filesystem::path location_;
      project* project_;
   };
}
