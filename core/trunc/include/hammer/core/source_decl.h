#if !defined(h_1fa42706_d348_48de_9e78_83103be8476d)
#define h_1fa42706_d348_48de_9e78_83103be8476d

#include "pstring.h"
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
         source_decl() : properties_(NULL), public_(false), type_(NULL) {};
         source_decl(const pstring& target_path,
				         const pstring& target_name,
                     const target_type* t, 
                     feature_set* props)
                    :
			            target_path_(target_path),
						   target_name_(target_name),
                     type_(t),
                     properties_(props),
                     public_(false)
             {}
			
         void target_path(const pstring& v, const target_type* t) { target_path_ = v; type_ = t; }
         void target_name(const pstring& v) { target_name_ = v; }
         void set_public(bool v) { public_ = v; }

         // FIXME. feature_set should be ref counted
         void properties(feature_set* v) { properties_ = v; }

         const pstring& target_path() const { return target_path_; }
         const pstring& target_name() const { return target_name_; }
         bool target_path_is_global() const { return !target_path_.empty() && *target_path_.begin() == '/'; }
         const target_type* type() const { return type_; }
         void set_type(const target_type* v) { type_ = v; }
         // FIXME. feature_set should be ref counted
         feature_set* properties() const { return properties_; }
         bool is_public() const { return public_; }

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
         pstring target_path_;
         pstring target_name_;
         const target_type* type_;
         
         // FIXME: это должно быть const, но так как нужно делать set_dependency_data приходиться от этого отказываться
         // нужно перевести feature_set и feature на reference counted основу и тогда все будет зашибись
         feature_set* properties_;
         bool public_;
	 };
}

#endif //h_1fa42706_d348_48de_9e78_83103be8476d
