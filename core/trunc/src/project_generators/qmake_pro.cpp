#include "stdafx.h"
#include <iostream>
#include <hammer/core/project_generators/qmake_pro.h>
#include <hammer/core/project.h>
#include <hammer/core/basic_target.h>
#include <hammer/core/main_target.h>
#include <hammer/core/meta_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/target_type.h>
#include <hammer/core/types.h>
#include <boost/foreach.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>

using namespace std;

namespace hammer{namespace project_generators{

qmake_pro::qmake_pro(const project& source_project,
                     const location_t& output_prefix)
   : source_project_(source_project),
     output_prefix_(output_prefix),
     meta_target_(NULL)
{
}

void qmake_pro::add_target(boost::intrusive_ptr<const build_node> node)
{
   if (node->products_.empty())
      throw std::runtime_error("Can't add target with empty products");

   const basic_target* bt = node->products_.at(0);

   if (bt->get_project() != &source_project_)
      return;

   if (bt->type().equal_or_derived_from(types::SEARCHED_LIB) ||
       bt->type().equal_or_derived_from(types::PREBUILT_STATIC_LIB) ||
       bt->type().equal_or_derived_from(types::PREBUILT_SHARED_LIB))
   {
      // skip pre built and search targets
      return;
   }

   if (meta_target_ == NULL)
   {
      meta_target_ = bt->get_meta_target();
      if (meta_target_->get_project() != &source_project_)
         throw std::runtime_error("Can't add target from different project");
   }
   else
   {
      if (bt->get_project() != &source_project_)
         throw std::runtime_error("Can't add target from different project");
   }

   m2m_t::const_iterator i = m2m_.find(bt->get_meta_target());
   if (i != m2m_.end())
   {
      if (i->second != bt->get_main_target())
         throw std::runtime_error("Can't add two target ");
   }
   else
   {
      m2m_.insert(make_pair(bt->get_meta_target(), bt->get_main_target()));
      p2m_.insert(make_pair(bt->get_project(), bt->get_meta_target()));
      gather_dependencies(bt->get_main_target());
   }
}

void qmake_pro::gather_dependencies(const main_target* mt)
{
   BOOST_FOREACH(const basic_target* bt, mt->sources())
   {
      const main_target* mt = dynamic_cast<const main_target*>(bt);
      if (mt)
      {
         if (mt->type().equal_or_derived_from(types::SEARCHED_LIB) ||
             mt->type().equal_or_derived_from(types::PREBUILT_STATIC_LIB) ||
             mt->type().equal_or_derived_from(types::PREBUILT_SHARED_LIB))
         {
            // skip pre built and search targets
            continue;
         }

         m2m_t::const_iterator i = m2m_.find(mt->get_meta_target());
         if (i != m2m_.end())
         {
            if (i->second != mt)
               throw std::runtime_error("Can't handle two targets from one metatarget");
         }
         else
         {
            p2m_t::const_iterator p = p2m_.find(mt->get_project());
            if (p != p2m_.end())
               throw std::runtime_error("Can't handle two meta targets from one project\n"
                                        "target 1: '" + p->second->name().to_string() + "'\n"
                                        "target 2: '" + mt->name().to_string() + "'\n");

            m2m_.insert(make_pair(mt->get_meta_target(), mt));
            p2m_.insert(make_pair(mt->get_project(), mt->get_meta_target()));
            gather_dependencies(mt);
         }
      }
   }
}

void qmake_pro::write() const
{
   cout << "found " << m2m_.size() << " targets\n";
   BOOST_FOREACH(const m2m_t::value_type& mm, m2m_)
   {
      location_t p = mm.first->get_project()->location() / output_prefix_ / "qmake";
      boost::filesystem::create_directories(p);
      boost::filesystem::ofstream f(p / (mm.second->name().to_string() + ".pro"), ios::trunc);
      f << "QT -= core gui\n"
        << "TARGET = " << mm.first->name() << '\n'
        << "TEMPLATE = ";

      if (mm.second->type().equal_or_derived_from(types::EXE))
         f << "app\n";
      else
         if (mm.second->type().equal_or_derived_from(types::LIB))
            f << "lib\n";
         else
            f << "unknown\n";
      
      std::vector<location_t> includes;
      feature_set::const_iterator i = mm.second->properties().find("include");
      while(i != mm.second->properties().end())
      {
         location_t full_path = (**i).get_path_data().target_->location() / (**i).value().to_string();
         full_path.normalize();
         includes.push_back(full_path);
         i = mm.second->properties().find(i + 1, "include");
      }
      
      if (!includes.empty())
      {
         f << "INCLUDEPATH = \\\n";
         for(size_t i = 0; i < includes.size(); ++i)
         {
            f << "   " << includes[i];
            if (i + 1 != includes.size())
               f << " \\\n";
            else
               f << '\n';
         }
      }
      f << "\n";

      bool has_prev = false;
      BOOST_FOREACH(const basic_target* bt, mm.second->sources())
      {
         if (bt->type().equal_or_derived_from(types::CPP) || 
             bt->type().equal_or_derived_from(types::C))
         {
            if (has_prev)
               f << " \\\n";
            else
            {
               f << "SOURCES = \\\n";
               has_prev = true;
            }

            f << "    " << (bt->location() / bt->name().to_string());
         }
      }
      f << "\n";

      has_prev = false;
      BOOST_FOREACH(const basic_target* bt, mm.second->sources())
      {
         if (bt->type().equal_or_derived_from(types::H))
         {
            if (has_prev)
               f << " \\\n";
            else
            {
               f << "HEADERS = \\\n";
               has_prev = true;
            }

            f << "    " << (bt->location() / bt->name().to_string());
         }
      }
      f << "\n";

   }
}

}}
