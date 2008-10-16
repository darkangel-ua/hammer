#if !defined(h_1fa42706_d348_48de_9e78_83103be8476d)
#define h_1fa42706_d348_48de_9e78_83103be8476d

#include "pstring.h"

namespace hammer
{
   class feature_set;

   class source_decl
	{
		public:
         source_decl() : properties_(NULL) {};
         source_decl(const pstring& target_path,
				         const pstring& target_name,
			            feature_set* props) 
                    :
			            target_path_(target_path),
						   target_name_(target_name),
                     properties_(props)
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

			pstring target_path_;
			pstring target_name_;
         // FIXME: ��� ������ ���� const, �� ��� ��� ����� ������ set_dependency_data ����������� �� ����� ������������
         // ����� ��������� feature_set � feature �� reference counted ������ � ����� ��� ����� ��������
			feature_set* properties_;
	 };
}

#endif //h_1fa42706_d348_48de_9e78_83103be8476d
