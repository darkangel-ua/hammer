#pragma once
#include <boost/filesystem/path.hpp>

namespace hammer
{
   class engine;
   class project;

   struct hammer_walker_context
   {
      engine* engine_;
      boost::filesystem::path location_;
      project* project_;
   };
}
