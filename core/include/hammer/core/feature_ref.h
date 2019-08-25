#pragma once

namespace hammer {

class feature;

class feature_ref {
   public:
      feature_ref(const feature& v) : v_(&v) {}
      const feature* operator ->() const { return v_; }
      operator const feature&() const { return *v_; }
      const feature& operator *() const { return *v_; }
      const feature& get() const { return *v_; }

      bool operator == (const feature_ref& rhs) const { return v_ == rhs.v_; }
      bool operator != (const feature_ref& rhs) const { return !(*this == rhs); }
      bool operator < (const feature_ref& rhs) const { return v_ < rhs.v_; }

   private:
      const feature* v_;
};
}
