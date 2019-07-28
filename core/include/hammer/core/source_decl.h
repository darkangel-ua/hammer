#pragma once
#include <functional>
#include <hammer/core/type_registry.h>
#include <hammer/core/value_ptr.h>

namespace hammer {

class feature_set;
class target_type;
class project;
class build_request;

// FIXME: type should not be determined at constructing time
// FIXME: source_decl should have method resolve_type for resolving type of source
// FIXME: and caching it. Without caching it will be huge performance hit
class source_decl {
   public:
      // FIXME: need to remove this constructor
      source_decl();
      source_decl(const project& p,
                  const std::string& target_path,
                  const std::string& target_name,
                  const target_type* t);
      source_decl(const source_decl& v);
      source_decl(source_decl&& v);
      ~source_decl();

      source_decl& operator = (const source_decl& rhs);

      void target_path(const std::string& v, const target_type* t) { target_path_ = v; type_ = t; }
      void target_name(const std::string& v) { target_name_ = v; }
      void set_public(bool v) { public_ = v; }

      void build_request(const hammer::build_request& v);
      void build_request(hammer::build_request&& v);
      void reset_build_request();

      const hammer::build_request*
      build_request() const { return build_request_.get(); }

      hammer::build_request*
      build_request() { return build_request_.get(); }

      void build_request_join(const feature_set& props);

      const std::string& target_path() const { return target_path_; }
      const std::string& target_name() const { return target_name_; }
      bool target_path_is_global() const { return !target_path_.empty() && *target_path_.begin() == '/'; }
      const target_type* type() const { return type_; }
      void set_type(const target_type* v) { type_ = v; }
      bool is_public() const { return public_; }

      void set_locals_allowed(bool v) { locals_allowed_ = v; }
      bool locals_allowed() const { return locals_allowed_; }
      const project& owner_project() const { return project_; }

      bool operator < (const source_decl& rhs) const {
         if (target_path_ < rhs.target_path_)
            return true;
         else
            if (target_path_ == rhs.target_path_)
               return false;
            else
               if (target_name_ < rhs.target_name_)
                  return true;
               else
                  return false;
      }

      bool operator == (const source_decl& rhs) const {
         return &project_.get() == &rhs.project_.get() &&
                target_path_ == rhs.target_path_ &&
                target_name_ == rhs.target_name_;
      }

      bool operator != (const source_decl& rhs) const { return !(*this == rhs); }

      bool is_project_local_reference() const;
      bool is_meta_target() const { return type_ == nullptr; }

   private:
      std::reference_wrapper<const project> project_;
      std::string target_path_;
      std::string target_name_;
      const target_type* type_;

      value_ptr<hammer::build_request> build_request_;
      bool public_ = false;

      // this is used internally, when passing around <source> feature that can possibly point on local target
      // sources from hamfile-s can't have this enabled
      bool locals_allowed_ = false;
};

// check that source_decl looks like reference on target within same project
bool looks_like_local_target_ref(const source_decl& sd);
}
