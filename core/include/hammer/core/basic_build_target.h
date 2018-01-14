#pragma once

#include <boost/noncopyable.hpp>
#include <string>
#include <hammer/core/location.h>
#include <hammer/core/timestamp_info.h>
#include <hammer/core/build_node.h>

namespace hammer {

class main_target;
class target_type;
class feature_set;
class basic_meta_target;
class project;
class engine;
class build_environment;

class basic_build_target : private boost::noncopyable
{
   public:
		basic_build_target(const main_target* mt,
		                   const std::string& name,
	                      const target_type* t,
		                   const feature_set* f)
                  : main_target_(mt),
                    type_(t),
                    name_(name),
                    features_(f)
      {}

		const std::string& name() const { return name_; }
      const target_type& type() const { return *type_; }
      const feature_set& properties() const { return *features_; }
      const main_target* get_main_target() const { return main_target_; }
      const basic_meta_target* get_meta_target() const;
      const project* get_project() const;
      engine* get_engine() const;

		const timestamp_info_t& timestamp_info() const;
		virtual const location_t& location() const;

		// FIXME: const here seams not good solution because after clean timestamp_info should return
		// something different than before. But I leave this for future clean-up
		virtual void clean(const build_environment& environment) const = 0;

	protected:
		mutable timestamp_info_t timestamp_info_;
		virtual void timestamp_info_impl() const = 0;

	private:
		const main_target* main_target_;
      const target_type* type_;
      const std::string name_;
      const feature_set* features_;
};

}
