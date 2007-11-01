#include "stdafx.h"
#include <iostream>
#include "engine.h"
#include "feature_set.h"
#include "feature_registry.h"
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include "msvc_project_generator.h"

using namespace std;                                                                                        
namespace fs = boost::filesystem;
using namespace hammer;

int main(int argc, char* argv[])
{
	try
	{
      if (argc != 2)
      {
         cout << "Usage: hammer jamfile\n\n";
         return 1;
      }

      engine e(fs::current_path());
      const project& p = e.load_project(location_t());
      cout << (boost::format("Loaded project id '%s' location '%s'\n") % p.id() % p.location().string());
      cout << "targets size " << p.targets().size();

      feature_set* build_request = e.feature_registry().make_set();
      build_request->insert("variant", "debug");
      vector<basic_target*> result(p.instantiate("test1", *build_request));

      project_generators::msvc g(e);
      g.add_variant(result, *build_request);
      g.generate();
	}
   catch (std::exception& e)
	{
      cout << "Exception: " << e.what() << "\n\n";		
      return -1;
	}
   catch(...)
   {
      cout << "Unknown exception \n\n";		
      return -1;
   }

   return 0;
}

