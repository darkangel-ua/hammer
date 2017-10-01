#if !defined(h_bd428576_c9b2_4eb5_8a5a_1fe3dd0e85cb)
#define h_bd428576_c9b2_4eb5_8a5a_1fe3dd0e85cb

#include <hammer/ast/expression.h>
#include <hammer/parscore/identifier.h>
#include <hammer/ast/types.h>

namespace hammer{ namespace ast{

class path_like_seq;
class requirement_set;

class target : public expression
{
   public:
		target(parscore::source_location public_tag,
				 const path_like_seq* target_path,
				 const parscore::identifier& target_name,
				 const features_t& build_request);

		const path_like_seq* target_path() const { return target_path_; }
		const parscore::identifier& target_name() const { return target_name_; }
		const features_t& build_request() const { return build_request_; }
		parscore::source_location public_tag() const { return public_tag_; }

      bool has_target_name() const;
      bool is_public() const { return public_tag_.valid(); }
      
      parscore::source_location start_loc() const override;
      bool accept(visitor& v) const override;

   private:
      parscore::source_location public_tag_;
	  const path_like_seq* target_path_;
      parscore::identifier target_name_;
	  const features_t build_request_;
};

}}

#endif
