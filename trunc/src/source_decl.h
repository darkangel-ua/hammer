#if !defined(h_1fa42706_d348_48de_9e78_83103be8476d)
#define h_1fa42706_d348_48de_9e78_83103be8476d

#include "pstring.h"
#include "type_registry.h"

namespace hammer
{
   class feature_set;
   class type;

   class source_decl
	{
		public:
         source_decl() : properties_(NULL) {};
         source_decl(const pstring& target_path,
				         const pstring& target_name,
                     const type* t, 
                     feature_set* props)
                    :
			            target_path_(target_path),
						   target_name_(target_name),
                     type_(t),
                     properties_(props)
             {}
			
         void target_path(const pstring& v, const type* t) { target_path_ = v; type_ = t; }
         void target_name(const pstring& v) { target_name_ = v; }

         // FIXME. feature_set should be ref counted
         void properties(feature_set* v) { properties_ = v; }

         const pstring& target_path() const { return target_path_; }
         const pstring& target_name() const { return target_name_; }
         const hammer::type* type() const { return type_; }
         // FIXME. feature_set should be ref counted
         feature_set* properties() const { return properties_; }

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
         
      private:
         pstring target_path_;
         pstring target_name_;
         const hammer::type* type_;
         
         // FIXME: ��� ������ ���� const, �� ��� ��� ����� ������ set_dependency_data ����������� �� ����� ������������
         // ����� ��������� feature_set � feature �� reference counted ������ � ����� ��� ����� ��������
         feature_set* properties_;
	 };
}

#endif //h_1fa42706_d348_48de_9e78_83103be8476d
