#include <regex>
#include <boost/filesystem/fstream.hpp>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/feature_registry.h>
#include "htmpl_build_target.h"

using namespace std;
namespace fs = boost::filesystem;

namespace hammer {

static
const regex htmpl_pattern("(%%<(.+?)>%%)");

htmpl_build_target::htmpl_build_target(const main_target* mt,
                                       const std::string& name,
                                       const location_t& l,
                                       const target_type* t,
                                       const feature_set* f)
   : source_build_target(mt, name, l, t, f)
{
	const fs::path source_file = l / name ;
	fs::ifstream source_stream(source_file);
	const vector<char> content{istreambuf_iterator<char>(source_stream), istreambuf_iterator<char>()};
	for (regex_iterator<vector<char>::const_iterator> first(content.cbegin(), content.cend(), htmpl_pattern), last = {}; first != last; ++first) {
		const string feature_name = (*first)[2];
		auto i = f->find(feature_name);
		if (i != f->end())
         append_valuable_feature(valuable_features_, *properties().owner().create_feature(feature_name, (**i).value()), properties().owner());
	}
}

}
