#include <hammer/core/source_build_target.h>

namespace hammer {

static
std::string empty_hash;

const std::string&
source_build_target::hash() const
{
	return empty_hash;
}

}
