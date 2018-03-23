#if !defined(h_1fa42706_d348_48de_9e78_83103be8476d)
#define h_1fa42706_d348_48de_9e78_83103be8476d

#include "type_registry.h"

namespace hammer
{
   class feature_set;
   class target_type;

   // FIXME: type should not be determined at constructing time
   // FIXME: source_decl should have method resolve_type for resolving type of source
   // FIXME: and caching it. Without caching it will be huge performance hit
   class source_decl
   {
      public:
         source_decl() : type_(NULL), properties_(NULL), public_(false) {}
         source_decl(const std::string& target_path,
                     const std::string& target_name,
                     const target_type* t, 
                     feature_set* props)
                    :
			            target_path_(target_path),
						   target_name_(target_name),
                     type_(t),
                     properties_(props),
                     public_(false)
             {}
			
         void target_path(const std::string& v, const target_type* t) { target_path_ = v; type_ = t; }
         void target_name(const std::string& v) { target_name_ = v; }
         void set_public(bool v) { public_ = v; }

         // FIXME. feature_set should be ref counted
         void properties(feature_set* v) { properties_ = v; }

         const std::string& target_path() const { return target_path_; }
         const std::string& target_name() const { return target_name_; }
         bool target_path_is_global() const { return !target_path_.empty() && *target_path_.begin() == '/'; }
         const target_type* type() const { return type_; }
         void set_type(const target_type* v) { type_ = v; }
         // FIXME. feature_set should be ref counted
         feature_set* properties() const { return properties_; }
         bool is_public() const { return public_; }

         void set_locals_allowed(bool v) { locals_allowed_ = v; }
         bool locals_allowed() const { return locals_allowed_; }

         bool operator < (const source_decl& rhs) const
			{
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

			bool operator == (const source_decl& rhs) const
			{
				return target_path_ == rhs.target_path_ && 
					   target_name_ == rhs.target_name_;
			}

         bool operator != (const source_decl& rhs) const { return !(*this == rhs); }
         
      private:
         std::string target_path_;
         std::string target_name_;
         const target_type* type_;
         
         feature_set* properties_;
         bool public_;

         // this is used internally, when passing around <source> feature that can posibly point on local target
         // sources from hamfile-s can't have this enabled
         bool locals_allowed_ = false;
   };

   // check that source_decl looks like reference on target within same project
   bool looks_like_local_target_ref(const source_decl& sd);
}

#endif //h_1fa42706_d348_48de_9e78_83103be8476d
