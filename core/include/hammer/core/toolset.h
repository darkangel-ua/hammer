#pragma once
#include <string>
#include <hammer/core/location.h>
#include <hammer/core/rule_manager.h>

namespace hammer {

class engine;

class toolset {
	public:
		toolset(const std::string& name,
		        const rule_declaration& use_rule);

		const std::string& name() const { return name_; }
		const rule_declaration& use_rule() const { return use_rule_; }

		// try to autoconfigure all known versions
		virtual void autoconfigure(engine& e) const = 0;
		// try to configure specific version
		virtual void configure(engine& e,
		                       const std::string& version) const = 0;
		virtual ~toolset();

	private:
		const std::string name_;
		const rule_declaration use_rule_;
};

}
