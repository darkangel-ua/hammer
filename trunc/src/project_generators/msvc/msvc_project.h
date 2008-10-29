#pragma once

#include "../../location.h"
#include <vector>
#include <utility>
#include "../../feature_set.h"
#include "../../pstring.h"
#include <boost/guid.hpp>
#include <boost/intrusive_ptr.hpp>
#include "../../build_node.h"
#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace hammer
{
   class feature_set;
   class source_target;
   class basic_target;
   class type;
   class engine;
   class meta_target;
   class main_target;
   class pch_main_target;

   namespace project_generators
   {
      namespace configuration_types
      {
         enum value {exe = 1, static_lib = 4, shared_lib = 2, utility = 10};
      }

      class msvc_project
      {
         public:
            typedef std::vector<const hammer::main_target*> dependencies_t;

            class options
            {
               public:
                  struct character_set { enum value {unknown, unicode, multi_byte}; };
                  struct pch_usage_t { enum value {not_use, use, create}; };

                  options() : has_compiler_options_(false), 
                              has_linker_options_(false),
                              has_librarian_options_(false),
                              compile_as_cpp_(false),
                              character_set_(character_set::unicode),
                              pch_target_(NULL),
                              pch_usage_(pch_usage_t::not_use)
                  {}
                  
                  void add_include(const std::string& v) { includes_ << v << ';'; has_compiler_options_ = true; }
                  void add_define(const pstring& v) { defines_ << v << ';'; has_compiler_options_ = true; }
                  void add_searched_lib(const std::string& v) { searched_libs_ << v << ' '; has_linker_options_ = true; }
                  void add_cxx_flag(const pstring& v) { cxxflags_ << v << ' '; has_compiler_options_ = true; } 
                  void compile_as_cpp(bool v) { compile_as_cpp_ = true; has_compiler_options_ = true; }
                  void character_set(character_set::value v) { character_set_ = v; }
                  void pch_target(const pch_main_target* t) { pch_target_ = t; }
                  void pch_usage(pch_usage_t::value v) { pch_usage_ = v; }

                  const std::ostringstream& includes() const { return includes_; }
                  const std::ostringstream& defines() const { return defines_; }
                  const std::ostringstream& searched_libs() const { return searched_libs_; }
                  const std::ostringstream& cxxflags() const { return cxxflags_; }
                  bool compile_as_cpp() const { return compile_as_cpp_; }
                  character_set::value character_set() const { return character_set_; }
                  const pch_main_target& pch_target() const { return *pch_target_; }
                  pch_usage_t::value pch_usage() const { return pch_usage_; }

                  bool has_compiler_options() const { return has_compiler_options_ || 
                                                             pch_usage_ != pch_usage_t::not_use; }
                  bool has_linker_options() const { return has_linker_options_; }
                  bool has_librarian_options() const { return has_librarian_options_; }

               private:
                  std::ostringstream defines_;
                  std::ostringstream includes_;
                  std::ostringstream searched_libs_;
                  std::ostringstream cxxflags_;
                  
                  bool has_compiler_options_ : 1;
                  bool has_linker_options_ : 1;
                  bool has_librarian_options_ : 1;
                  bool compile_as_cpp_ : 1;
                  character_set::value character_set_;
                  const pch_main_target* pch_target_;
                  pch_usage_t::value pch_usage_;
            };

            struct variant
            {
               boost::intrusive_ptr<const build_node> node_;
               const main_target* target_;
               const feature_set* properties_;
               std::string name_;
               boost::shared_ptr<options> options_;
               msvc_project* owner_; // FIXME: выглядит это очень странно, но без этого поля не работает file_configuration::write
            };

            typedef boost::ptr_vector<variant> variants_t;

            msvc_project(engine& e, const location_t& output_dir, const boost::guid& uid = boost::guid::create());
            void add_variant(boost::intrusive_ptr<const build_node> node);
            bool has_variant(const main_target* v) const;
            void generate() const;
            const std::string& id() const { return id_; }
            const boost::guid& guid() const { return uid_; }
            const hammer::meta_target& meta_target() const { return *meta_target_; }
            const dependencies_t& dependencies() const { return dependencies_;}
            location_t full_project_name() const { return full_project_name_; } // путь относительно meta_target проекта и имя файла проекта
            const location_t& output_dir() const { return output_dir_; }
            const location_t& project_output_dir() const { return project_output_dir_; }
            const pstring& name() const;
            const variants_t& variants() const { return variants_; }
            
         private:
            struct write_context
            {
               write_context(std::ostream& s,
                             const type& h_type) : output_(s), h_type_(h_type) {}   

               const type& h_type_;
               std::ostream& output_;
            };

            struct file_configuration
            {
               file_configuration() : exclude_from_build(true) {}
               
               void write(write_context& ctx, const variant& v) const;
               const basic_target* target_;
               bool exclude_from_build;
            };

            struct file_with_cfgs_t
            {
               typedef std::map<const variant*, file_configuration> file_config_t;

               void write(write_context& ctx, const std::string& path_prefix) const;
               
               pstring file_name_; // Это файл который будет в секции File в vcproj
               file_config_t file_config;  // для каждого варианта своя basic_target со своими свойствами
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
                  void write(write_context& ctx, const std::string& path_prefix) const;
                  bool accept(const type* t) const;
                  void insert(const basic_target* t, const variant& v);

               private:
                  types_t types_;
            };

            typedef std::vector<filter_t> files_t;
            
            engine* engine_;
            mutable variants_t variants_;
            mutable files_t files_;
            boost::guid uid_;
            std::string id_;
            const hammer::meta_target* meta_target_;
            mutable dependencies_t dependencies_;
            location_t full_project_name_;
            location_t output_dir_;
            location_t project_output_dir_;
            location_t meta_target_relative_to_output_;
            
            const type* searched_lib_;
            const type* obj_type_;
            const type* pch_type_;

            void fill_filters() const;
            void write_header(std::ostream& s) const;
            void write_configurations(std::ostream& s) const;
            void write_files(write_context& ctx) const;
            void gether_files_impl(const build_node& node, variant& v) const;
            void gether_files() const;
            void insert_into_files(const basic_target* t, const variant& v) const;
            configuration_types::value resolve_configuration_type(const variant& v) const;
            void fill_options(const feature_set& props, options* opts, const main_target& mt) const;
      };
   }
}