#if !defined(h_fc44552f_5323_4e10_ac17_50675e11d01b)
#define h_fc44552f_5323_4e10_ac17_50675e11d01b

#include "argument_writer.h"
#include <vector>
#include <utility>

namespace hammer
{
   class feature_set;
   class feature_registry;

   class fs_argument_writer : public argument_writer
   {
      public:
         fs_argument_writer(const std::string& name, feature_registry& fr)
            : argument_writer(name), feature_registry_(&fr) 
         {}

         fs_argument_writer& add(const feature_set* pattern, const std::string& what_write);
         fs_argument_writer& add(const std::string& pattern, const std::string& what_write);
         fs_argument_writer* clone() const override;
			std::vector<const feature*> valuable_features() const override;

      private:
         typedef std::vector<std::pair<const feature_set* /*pattern*/, std::string /*what write*/> > patterns_t;
         
         patterns_t patterns_;
         feature_registry* feature_registry_;

         void write_impl(std::ostream& output,
			                const build_node& node,
			                const build_environment& environment) const override;
   };
}

#endif //h_fc44552f_5323_4e10_ac17_50675e11d01b
