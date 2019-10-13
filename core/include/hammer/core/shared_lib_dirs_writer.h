#pragma once
#include <hammer/core/argument_writer.h>

namespace hammer {

class target_type;

class shared_lib_dirs_writer : public argument_writer {
	public:
		shared_lib_dirs_writer(const std::string& name,
									  const target_type& shared_lib_type)
			: argument_writer(name),
			  shared_lib_type_(shared_lib_type)
		{}

		argument_writer* clone() const override { return new shared_lib_dirs_writer(*this); }
		std::vector<feature_ref> valuable_features() const override { return {}; }

	protected:
		void write_impl(std::ostream& output,
							 const build_node& node,
							 const build_environment& environment) const override;
	private:
		const target_type& shared_lib_type_;
};

}
