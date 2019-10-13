#include "stdafx.h"
#include <boost/filesystem/convenience.hpp>
#include <boost/scoped_array.hpp>
#include <hammer/core/fs_helpers.h>

namespace fs = boost::filesystem;

namespace hammer
{

static void strip_slesh_dot(boost::filesystem::path& p)
{
   if (p.filename() == "." && p.string().size() > 2)
      p = p.branch_path();
}

boost::filesystem::path relative_path(boost::filesystem::path p,
                                      boost::filesystem::path relative_to)
{
   strip_slesh_dot(p);
   strip_slesh_dot(relative_to);

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


   path final(".");


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
      final /= path(*pit++);


   // .normalize()?
   return final;

}

#if defined(_WIN32)

   boost::filesystem::path resolve_symlinks(const boost::filesystem::path& p)
   {
      return p;
   }

#else
#  include <unistd.h>

   boost::filesystem::path resolve_symlinks(const boost::filesystem::path& p)
   {
      fs::path result;
      for(fs::path::const_iterator i = p.begin(), last = p.end(); i != last; ++i)
      {
         result /= *i;
         if (is_symlink(result))
         {
            char buff[1024];
            ssize_t s = readlink(result.native().c_str(), buff, sizeof(buff));
            if (s == -1)
               return p;
            else
            {
               fs::path link(buff, buff + s);
               if (link.has_root_directory())
                  result = link;
               else
                  result /= link;
               result.normalize();
            }
         }
      }

      return result;
   }

#endif

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
