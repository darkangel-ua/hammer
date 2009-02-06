#include "stdafx.h"
#include "fs_helpers.h"

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

}