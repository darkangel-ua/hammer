#pragma once
#include <unordered_set>
#include <deque>

namespace hammer {

class basic_meta_target;

class instantiation_context {
   public:
      class guard;

   private:
      void enter(const basic_meta_target& mt);
      void leave();

      std::unordered_set<const basic_meta_target*> targets_;
      std::deque<const basic_meta_target*> stack_;
};

class instantiation_context::guard {
   public:
      guard(instantiation_context& ctx,
            const basic_meta_target& mt)
         : ctx_(ctx)
      {
         ctx_.enter(mt);
      }

      ~guard() { ctx_.leave(); }

   private:
      instantiation_context& ctx_;
};

}
