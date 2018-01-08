#if !defined(h_ae72613c_b54f_43a4_ad8c_274cf2fdf589)
#define h_ae72613c_b54f_43a4_ad8c_274cf2fdf589

#include <hammer/core/argument_writer.h>

namespace hammer {
class pch_argument_writer : public argument_writer
{
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

	protected:
		void write_impl(std::ostream& output,
		                const build_node& node,
		                const build_environment& environment) const override;

		part::value part_;
		std::string prefix_;
		std::string suffix_;
};

}

#endif //h_ae72613c_b54f_43a4_ad8c_274cf2fdf589
