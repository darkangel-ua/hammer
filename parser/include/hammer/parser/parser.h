#if !defined(h_25c0e67c_ed8e_4fb9_9da4_e8bfb9aff439)
#define h_25c0e67c_ed8e_4fb9_9da4_e8bfb9aff439

#include <boost/filesystem/path.hpp>
#include <hammer/sema/actions.h>
#include <functional>

namespace hammer{

typedef std::unique_ptr<const ast::hamfile, std::function<void (const ast::hamfile*)>> ast_hamfile_ptr;

ast_hamfile_ptr
parse_hammer_script(const boost::filesystem::path& hamfile,
                    sema::actions& actions);

ast_hamfile_ptr
parse_hammer_script(const std::string content,
                    const std::string content_name,
                    sema::actions& actions);

}

#endif
