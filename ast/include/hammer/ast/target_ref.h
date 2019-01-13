#pragma once
#include <hammer/ast/expression.h>
#include <hammer/parscore/identifier.h>
#include <hammer/ast/types.h>

namespace hammer { namespace ast {

class path;
class requirement_set;

class target_ref : public expression {
   public:
		target_ref(parscore::source_location public_tag,
                 parscore::source_location project_local_ref_tag,
					  const path* target_path,
					  const parscore::identifier& target_name,
					  const features_t& build_request);

		const path* target_path() const { return target_path_; }
		const parscore::identifier& target_name() const { return target_name_; }
		const features_t& build_request() const { return build_request_; }
		parscore::source_location public_tag() const { return public_tag_; }
      parscore::source_location project_local_ref_tag() const { return project_local_ref_tag_; }

      bool has_target_name() const;
      bool is_public() const { return public_tag_.valid(); }
      bool is_project_local_ref() const { return project_local_ref_tag_.valid(); }

      parscore::source_location start_loc() const override;
      bool accept(visitor& v) const override;

   private:
      parscore::source_location public_tag_;
      parscore::source_location project_local_ref_tag_;
		const path* target_path_;
      parscore::identifier target_name_;
		const features_t build_request_;
};

}}
