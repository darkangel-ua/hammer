#pragma once
#include <hammer/core/argument_writer.h>

namespace hammer {

class pch_argument_writer : public argument_writer {
	public:
		struct part { enum value { header, product}; };

		pch_argument_writer(const std::string& name, part::value part = part::header,
								  const std::string& prefix = std::string(),
								  const std::string& suffix = std::string())
			: argument_writer(name),
			  part_(part),
			  prefix_(prefix),
			  suffix_(suffix)
		{}

		pch_argument_writer* clone() const override { return new pch_argument_writer(*this); }
		std::vector<feature_ref> valuable_features() const override { return {}; }

	protected:
		void write_impl(std::ostream& output,
		                const build_node& node,
		                const build_environment& environment) const override;

		part::value part_;
		std::string prefix_;
		std::string suffix_;
};

}
