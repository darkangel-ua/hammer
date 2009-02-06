#if !defined(h_f23c1f7c_0299_4fb8_800f_27d4f6ff2a8d)
#define h_f23c1f7c_0299_4fb8_800f_27d4f6ff2a8d

#include <boost/filesystem/path.hpp>

namespace hammer
{
   boost::filesystem::path relative_path(boost::filesystem::path p, 
                                         const boost::filesystem::path& relative_to);

}

#endif //h_f23c1f7c_0299_4fb8_800f_27d4f6ff2a8d
