#if !defined(h_fb0a94cd_1c6c_486d_9d05_0442af28116e)
#define h_fb0a94cd_1c6c_486d_9d05_0442af28116e

#include <boost/filesystem/path.hpp>
#include <hammer/src/engine.h>

extern boost::filesystem::path test_data_path;

struct setuped_engine
{
   setuped_engine(bool install_toolsets = true);

   hammer::engine engine_;
};

#endif //h_fb0a94cd_1c6c_486d_9d05_0442af28116e
