#if !defined(h_754d46d2_cd26_4948_861c_c0e0096ac358)
#define h_754d46d2_cd26_4948_861c_c0e0096ac358

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <boost/shared_ptr.hpp>

namespace hammer
{
   class build_environment;
	class argument_writer;
	class feature;
	class build_node;

   class cmdline_builder
   {
      public:
         typedef std::map<const std::string, boost::shared_ptr<argument_writer> > writers_t;

         cmdline_builder(const std::string& cmd);
         template<typename T>
         cmdline_builder& operator +=(boost::shared_ptr<T>& v) { add(v); return *this; }
         void write(std::ostream& output, const build_node& node, const build_environment& environment) const;
         const writers_t& writers() const { return writers_; }
         void writers(const writers_t& w) { writers_ = w; }
			std::vector<const feature*> valuable_features() const;

      private:
         std::string cmd_;
         writers_t writers_;

         void add(const boost::shared_ptr<argument_writer>& v);
   };
}

#endif //h_754d46d2_cd26_4948_861c_c0e0096ac358
