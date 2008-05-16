#pragma once

#include "../../location.h"
#include <vector>
#include <utility>
#include "../../feature_set.h"
#include "../../pstring.h"
#include <boost/guid.hpp>
#include <boost/intrusive_ptr.hpp>
#include "../../build_node.h"

namespace hammer
{
   class feature_set;
   class source_target;
   class basic_target;
   class type;
   class engine;
   class meta_target;
   class main_target;

   namespace project_generators
   {
      class msvc_project
      {
         public:
            typedef std::vector<const hammer::main_target*> dependencies_t;

            msvc_project(engine& e);
            void add_variant(boost::intrusive_ptr<const build_node> node);
            bool has_variant(const main_target* v) const;
            void generate() const;
            const std::string& id() const { return id_; }
            const hammer::meta_target& meta_target() const { return *meta_target_; }
            const dependencies_t& dependencies() const { return dependencies_;}

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
               
               const basic_target* target;
               file_config_t file_config;
            };

            struct filter_t
            {
                  typedef std::vector<const type*> types_t;

                  std::string name;
                  std::string uid;
                  std::map<const basic_target*, file_with_cfgs_t> files_;

                  filter_t(const types_t& t, 
                           const std::string& name,
                           const std::string& uid) : types_(t), name(name), uid(uid) {}
                  std::ostream& write(std::ostream& s) const;
                  bool accept(const type* t) const;
                  void insert(const basic_target* t);

               private:
                  types_t types_;
            };

            struct variant
            {
               boost::intrusive_ptr<const build_node> node_;
               const main_target* target_;
               const feature_set* properties;
               std::string name;
            };

            typedef std::vector<variant> variants_t;
            typedef std::vector<filter_t> files_t;
            
            engine* engine_;
            mutable variants_t variants_;
            mutable files_t files_;
            boost::guid uid_;
            std::string id_;
            const hammer::meta_target* meta_target_;
            mutable dependencies_t dependencies_;

            const pstring& name() const;
            void fill_filters() const;
            void write_header(std::ostream& s) const;
            void write_configurations(std::ostream& s) const;
            void write_files(std::ostream& s) const;
            void gether_files_impl(const build_node& node) const;
            void gether_files() const;
            void insert_into_files(const basic_target* t) const;
      };
   }
}