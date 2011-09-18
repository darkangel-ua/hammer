#if !defined(h_f23c1f7c_0299_4fb8_800f_27d4f6ff2a8d)
#define h_f23c1f7c_0299_4fb8_800f_27d4f6ff2a8d

#include <boost/filesystem/path.hpp>

namespace hammer
{
   boost::filesystem::path relative_path(boost::filesystem::path p, 
                                         boost::filesystem::path relative_to);

#if defined(_WIN32) && !defined(__MINGW32__)
   boost::filesystem::wpath to_wide(const boost::filesystem::path& narrow_path);
#endif
}

#endif //h_f23c1f7c_0299_4fb8_800f_27d4f6ff2a8d
