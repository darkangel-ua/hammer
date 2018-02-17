#pragma once
#include <string>
#include <boost/noncopyable.hpp>

namespace hammer {

class feature_registry;

class feature_value_ns : public boost::noncopyable
{
		friend class feature_registry;

	public:
		const std::string name() const { return name_; }

	private:
		const std::string name_;

		feature_value_ns(const std::string& name);
};

}
