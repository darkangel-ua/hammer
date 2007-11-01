#include "stdafx.h"
#include "msvc_project.h"
#include "../../main_target.h"
#include "../../meta_target.h"
#include "../../project.h"
#include "../../engine.h"
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>

using namespace std;

namespace hammer{ namespace project_generators{

   void msvc_project::add_variant(const main_target* t, 
                                  const feature_set& props)
   {
      variants_.push_back(make_pair(t, &props));
   }

   void msvc_project::generate()
   {
      if (variants_.empty())
         throw runtime_error("Can't generate empty msvc project");

      const main_target& mt = *variants_.front().first;
      location_t l = mt.meta_target()->project()->engine()->root() / 
                     mt.meta_target()->project()->location() /
                     "vc80" / (mt.name().to_string() + ".vcproj");
      
      create_directories(l.branch_path());
      boost::filesystem::ofstream f(l, std::ios_base::trunc);
      f << "this is a test"; 
   }
}}