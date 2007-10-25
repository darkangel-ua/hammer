#include "stdafx.h"
#include "msvc_project_generator.h"
#include <vector>

using namespace std;
namespace hammer{ namespace project_generators{

   struct msvc::impl_t
   {
      impl_t(const engine& e) : engine_(e)
      {

      }

      const engine& engine_;
   };

   msvc::msvc(const engine& e) : impl_(new impl_t(e))
   {
   }

   msvc::~msvc()
   {
      delete impl_;
   }

   static void gather_all_targets(const basic_target& t, vector<const basic_target*>* result)
   {

   }

   void msvc::generate(const basic_target& t)
   {
      vector<const basic_target*> targets;
      gather_all_targets(t, &targets);
   }
}}
