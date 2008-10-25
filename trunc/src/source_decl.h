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
         source_decl() : properties_(NULL), is_type_resolved_(false) {};
         source_decl(const pstring& target_path,
				         const pstring& target_name,
			            feature_set* props) 
                    :
			            target_path_(target_path),
						   target_name_(target_name),
                     properties_(props),
                     is_type_resolved_(false)
             {}
			
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
         
         const hammer::type* type(const type_registry& tr) const
         {
            if (!is_type_resolved_)
            {
               type_ = tr.resolve_from_target_name(target_path_);
               is_type_resolved_ = true;
            }
         
            return type_;
         }
         
         // FIXME: this members must be in private part with access methods
			pstring target_path_;
         pstring target_name_;

         // FIXME: ��� ������ ���� const, �� ��� ��� ����� ������ set_dependency_data ����������� �� ����� ������������
         // ����� ��������� feature_set � feature �� reference counted ������ � ����� ��� ����� ��������
			feature_set* properties_;
      
      private:
         mutable const hammer::type* type_; // cached value;
         mutable bool is_type_resolved_;
	 };
}

#endif //h_1fa42706_d348_48de_9e78_83103be8476d
