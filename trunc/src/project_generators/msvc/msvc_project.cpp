#include "stdafx.h"
#include "msvc_project.h"
#include "../../main_target.h"
#include "../../meta_target.h"
#include "../../project.h"
#include "../../engine.h"
#include "../../type_registry.h"
#include "../../types.h"
#include "../../feature.h"

using namespace std;
   
namespace hammer{ namespace project_generators{

msvc_project::msvc_project(engine& e) : engine_(&e), uid_(boost::guid::create())
{
}

static std::string make_variant_name(const feature_set& fs)
{
   const feature* f = fs.get("variant");
   return f->value().to_string();
}

void msvc_project::add_variant(boost::intrusive_ptr<const build_node> node)
{
   assert(!node->products_.empty());
   variant v;
   const basic_target* t = node->products_[0];
   v.properties_ = &t->properties();
   v.node_ = node;
   v.target_ = node->products_[0]->mtarget();
   v.name_ = make_variant_name(t->properties());
   variants_.push_back(v);
   if (id_.empty())
   {
      id_ = v.target_->location().to_string();
      meta_target_ = v.target_->meta_target();
      location_ = engine_->root() / 
                  v.target_->meta_target()->project()->location().to_string() /
                  "vc80" / (name().to_string() + ".vcproj");
   }
}

void msvc_project::fill_filters() const
{
   filter_t::types_t source_types;
   source_types.push_back(engine_->get_type_registry().resolve_from_suffix(".cpp"));
   files_.push_back(filter_t(source_types, "Source Files", "{4FC737F1-C7A5-4376-A066-2A32D752A2FF}"));
   filter_t::types_t header_types;
   header_types.push_back(engine_->get_type_registry().resolve_from_suffix(".h"));
   files_.push_back(filter_t(header_types, "Header Files", "{93995380-89BD-4b04-88EB-625FBE52EBFB}"));
}

const pstring& msvc_project::name() const
{
   return variants_.front().target_->meta_target()->name();
}

void msvc_project::write_header(ostream& s) const
{
   s << "<?xml version=\"1.0\" encoding=\"windows-1251\"?>\n"
        "<VisualStudioProject\n"
        "   ProjectType=\"Visual C++\"\n"
        "   Version=\"8,00\"\n"
        "   Name=\"" << name() <<"\"\n"
        "   ProjectGUID=\"" << boost::guid::showbraces << uid_ << "\"\n"
        "   RootNamespace=\"" << name() << "\"\n"
        "   Keyword=\"Win32Proj\">\n"
        "	<Platforms>\n"
        "       <Platform Name=\"Win32\"/>\n"
        "   </Platforms>\n"
        "   <ToolFiles/>\n";
}

static void write_bottom(std::ostream& s)
{
   s << "</VisualStudioProject>";
}

void msvc_project::write_configurations(std::ostream& s) const
{
   s << "   <Configurations>\n";
   
   for(variants_t::const_iterator i = variants_.begin(), last = variants_.end(); i != last; ++i)
   {
      s << "      <Configuration\n"
           "         Name=\"" << i->name_ << "|Win32\"\n"
           "         OutputDirectory=\"$(SolutionDir)$(ConfigurationName)\"\n"
           "         IntermediateDirectory=\"$(ConfigurationName)\"\n"
           "         ConfigurationType=\"1\"\n"
           "         CharacterSet=\"1\">\n";

      s << "      </Configuration>\n";
   }

   s << "   </Configurations>\n";
}

void msvc_project::file_with_cfgs_t::write(std::ostream& s) const
{
   s << "         <File\n"
        "            RelativePath=\"" << "..\\" << target->name() << "\"\n"
        "         />\n";

}

std::ostream& msvc_project::filter_t::write(std::ostream& s) const
{
   s << "         <Filter\n"
        "            Name=\"" << name << "\"\n"
        "            Filter=\"cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx\"\n"
        "            UniqueIdentifier=\"" << uid << "\">\n";

   for(map<const basic_target*, file_with_cfgs_t>::const_iterator i = files_.begin(), last = files_.end(); i != last; ++i)
      i->second.write(s);

   s << "         </Filter>\n";
   return s;
}

void msvc_project::write_files(std::ostream& s) const
{
   s << "      <Files>\n";
   for(files_t::const_iterator i = files_.begin(), last = files_.end(); i != last; ++i)
      i->write(s);

   s << "      </Files>\n";
}

void msvc_project::generate() const
{
   if (variants_.empty())
      throw runtime_error("Can't generate empty msvc project");

   fill_filters();
   gether_files();

   const main_target& mt = *variants_.front().target_;
   
   create_directories(location_.branch_path());
   boost::filesystem::ofstream f(location_, std::ios_base::trunc);
   write_header(f);
   write_configurations(f);
   write_files(f);
   write_bottom(f);
}


bool msvc_project::filter_t::accept(const type* t) const
{
   for(types_t::const_iterator i = types_.begin(), last = types_.end(); i != last; ++i)
   {
      if (*i == t)
         return true;
   }

   return false;
}

void msvc_project::filter_t::insert(const basic_target* t)
{
   file_with_cfgs_t& fwc = files_[t];
   fwc.target = t;
   file_configuration& fc = fwc.file_config[&t->properties()];
   fc.exclude_from_build = false;
}

void msvc_project::insert_into_files(const basic_target* t) const
{
   const type* tp = &t->type();
   for(files_t::iterator fi = files_.begin(), flast = files_.end(); fi != flast; ++fi)
   {
      if (fi->accept(tp))
      {
         fi->insert(t);
         return;
      }
   }
}

void msvc_project::gether_files_impl(const build_node& node) const
{
   //const type& shared_lib = engine_->get_type_registry().resolve_from_name(types::SHARED_LIB.name());
   bool should_go_deeper = false;
   typedef build_node::targets_t::const_iterator iter;
   for(iter mi = node.sources_.begin(), mlast = node.sources_.end(); mi != mlast; ++mi)
   {
      if ((**mi).mtarget()->meta_target() == meta_target_)
      {
         insert_into_files(*mi);
         should_go_deeper = true;
      }
      else
         dependencies_.push_back((**mi).mtarget());
   }

   if (!should_go_deeper)
      return;

   typedef build_node::nodes_t::const_iterator niter;
   for(niter i = node.down_.begin(), last = node.down_.end(); i != last; ++i)
      gether_files_impl(**i);
}

void msvc_project::gether_files() const
{
   for(variants_t::const_iterator i = variants_.begin(), last = variants_.end(); i != last; ++i)
      gether_files_impl(*i->node_);

   std::sort(dependencies_.begin(), dependencies_.end());
   dependencies_.erase(std::unique(dependencies_.begin(), dependencies_.end()), dependencies_.end());
}

bool msvc_project::has_variant(const main_target* v) const
{
   for(variants_t::const_iterator i = variants_.begin(), last = variants_.end(); i != last; ++i)
   {
      if (i->target_ == v)
         return true;
   }

   return false;
}

}}