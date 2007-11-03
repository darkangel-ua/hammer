#pragma once

#include "../../location.h"
#include <vector>
#include <utility>
#include "../../feature_set.h"
#include "../../pstring.h"
#include <boost/guid.hpp>

namespace hammer
{
   class feature_set;
   class main_target;
   class source_target;
   class type;

   namespace project_generators
   {
      class msvc_project
      {
         public:
            msvc_project();
            void add_variant(const main_target* t, 
                             const feature_set& props);
            void generate();

         private:
            struct file_configuration
            {
               file_configuration() : exclude_from_build(true) {}
               
               const feature_set* variant;
               bool exclude_from_build;
            };

            struct file_with_cfgs_t
            {
               typedef std::map<const feature_set* /*variant*/, file_configuration> file_config_t;

               void write(std::ostream& s) const;
               
               const source_target* target;
               file_config_t file_config;
            };

            struct filter_t
            {
                  typedef std::vector<const type*> types_t;

                  std::string name;
                  std::string uid;
                  std::vector<file_with_cfgs_t> files_;

                  filter_t(const types_t& t, 
                           const std::string& name,
                           const std::string& uid) : types_(t), name(name), uid(uid) {}
                  std::ostream& write(std::ostream& s) const;
                  bool accept(const type* t);

               private:
                  types_t types_;
            };


            struct variant
            {
               const main_target* target;
               const feature_set* properties;
               std::string name;
            };

            typedef std::vector<variant> variants_t;
            typedef std::vector<filter_t> files_t;
            
            variants_t variants_;
            files_t files_;
            boost::guid uid_;

            const pstring& name() const;
            void fill_filters();
            void write_header(std::ostream& s);
            void write_configurations(std::ostream& s);
            void write_files(std::ostream& s);
      };
   }
}