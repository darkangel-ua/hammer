#pragma once
#include <functional>
#include <boost/filesystem/path.hpp>
#include <hammer/sema/actions.h>

namespace hammer {

class diagnostic;

typedef std::unique_ptr<const ast::hamfile, std::function<void (const ast::hamfile*)>> ast_hamfile_ptr;

ast_hamfile_ptr
parse_hammer_script(const boost::filesystem::path& hamfile,
                    sema::actions& actions,
                    diagnostic& diag);

ast_hamfile_ptr
parse_hammer_script(const std::string content,
                    const std::string content_name,
                    sema::actions& actions,
                    diagnostic& diag);

}
