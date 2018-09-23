#pragma once
#include <boost/filesystem/path.hpp>

namespace hammer {

boost::filesystem::path
relative_path(boost::filesystem::path p,
              boost::filesystem::path relative_to);

// takes full normalized path and resolve all symlinks
boost::filesystem::path
resolve_symlinks(const boost::filesystem::path& p);

#if defined(_WIN32) && !defined(__MINGW32__)
boost::filesystem::wpath to_wide(const boost::filesystem::path& narrow_path);
#endif

}
