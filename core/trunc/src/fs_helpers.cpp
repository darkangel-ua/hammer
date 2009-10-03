#include "stdafx.h"
#include <boost/filesystem/convenience.hpp>
#include <boost/scoped_array.hpp>
#include <hammer/core/fs_helpers.h>

namespace hammer
{

boost::filesystem::path relative_path(boost::filesystem::path p, 
                                      const boost::filesystem::path& relative_to)
{
   using namespace boost::filesystem;
   path current = relative_to; 

   // Trivial case 
   if(equivalent(current, p)) 
      return path("."); 


   // Doesn't share a root 
   if(!equivalent(current.root_path(), p.root_path())) 
      return p; 


   // We don't care about the root anymore 
   // (and makes the rest easier) 
   current = current.relative_path(); 
   p = p.relative_path(); 


   path final(".", native); 


   path::iterator pit = p.begin(), 
      cit = current.begin(); 
   // Find the shared directory 
   for(;pit != p.end() && cit != current.end(); ++pit, ++cit) 
      if(*pit != *cit) // May not be right 
         break; 


   // Put needed parent dirs in 
   while(cit != current.end()) 
   { 
      final = ".." / final; 
      ++cit; 
   } 


   // Add the path from shared 
   while(pit != p.end()) 
      // Gah! Why doesn't *path::iterator return paths? 
      final /= path(*pit++, native); 


   // .normalize()? 
   return final; 

}

#if defined(_WIN32) && !defined(__MINGW32__)
#include <windows.h>
boost::filesystem::wpath to_wide(const boost::filesystem::path& narrow_path)
{
   if (narrow_path.empty())
      return boost::filesystem::wpath();

   std::string p(narrow_path.string());
   boost::scoped_array<wchar_t> buf(new wchar_t[p.size() * 2 + 50]);
   int res = MultiByteToWideChar(CP_OEMCP, 0, p.c_str(), p.size(), buf.get(), p.size() * 2 + 50);
   if (res == 0)
      throw std::runtime_error("Can't convert narrow path '" + p + "'to wide.");

   return boost::filesystem::wpath(buf.get(), buf.get() + res);
}
#else
boost::filesystem::wpath to_wide(const boost::filesystem::path& narrow_path)
{
   throw std::runtime_error("Non windows platforms should not invoke 'to_wide'. Posible broken build of hammer.core");
}
#endif


}