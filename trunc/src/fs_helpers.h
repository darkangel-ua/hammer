#pragma once
#include <boost/filesystem/path.hpp>

namespace hammer
{
   boost::filesystem::path relative_path(boost::filesystem::path p, 
                                         const boost::filesystem::path& relative_to);

}