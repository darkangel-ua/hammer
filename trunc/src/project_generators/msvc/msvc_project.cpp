#include "stdafx.h"
#include "msvc_project.h"
#include "../../main_target.h"
#include "../../meta_target.h"
#include "../../project.h"
#include "../../engine.h"
#include "../../type_registry.h"
#include "../../feature.h"
#include "../../source_target.h"

using namespace std;
   
namespace hammer{ namespace project_generators{

msvc_project::msvc_project() : uid_(boost::guid::create())
{
}

static std::string make_variant_name(const feature_set& fs)
{
   const feature* f = fs.get("variant");
   return f->value().to_string();
}

void msvc_project::add_variant(const main_target* t, 
                               const feature_set& props)
{
   variant v;
   v.properties = &props;
   v.target = t;
   v.name = make_variant_name(props);
   variants_.push_back(v);
}

void msvc_project::fill_filters()
{
   engine* e = variants_.front().target->meta_target()->project()->engine();
   filter_t::types_t source_types;
   source_types.push_back(e->get_type_registry().resolve_from_suffix(".cpp"));
   files_.push_back(filter_t(source_types, "Source Files", "{4FC737F1-C7A5-4376-A066-2A32D752A2FF}"));
   filter_t::types_t header_types;
   header_types.push_back(e->get_type_registry().resolve_from_suffix(".h"));
   files_.push_back(filter_t(header_types, "Header Files", "{93995380-89BD-4b04-88EB-625FBE52EBFB}"));
}

const pstring& msvc_project::name() const
{
   return variants_.front().target->name();
}

void msvc_project::write_header(ostream& s)
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

void msvc_project::write_configurations(std::ostream& s)
{
   s << "   <Configurations>\n";
   
   for(variants_t::const_iterator i = variants_.begin(), last = variants_.end(); i != last; ++i)
   {
      s << "      <Configuration\n"
           "         Name=\"" << i->name << "|Win32\"\n"
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
        "            RelativePath=\"" << target->name() << "\"\n"
        "         </File>\n";

}

std::ostream& msvc_project::filter_t::write(std::ostream& s) const
{
   s << "         <Filter\n"
        "            Name=\"" << name << "\"\n"
        "            Filter=\"cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx\"\n"
        "            UniqueIdentifier=\"" << uid << "\">\n";

   for(vector<file_with_cfgs_t>::const_iterator i = files_.begin(), last = files_.end(); i != last; ++i)
      i->write(s);

   s << "         </Filter>\n";
   return s;
}

void msvc_project::write_files(std::ostream& s)
{
   s << "      <Files>\n";
   for(files_t::const_iterator i = files_.begin(), last = files_.end(); i != last; ++i)
      i->write(s);

   s << "      </Files>\n";
}

void msvc_project::generate()
{
   if (variants_.empty())
      throw runtime_error("Can't generate empty msvc project");

   fill_filters();

   const main_target& mt = *variants_.front().target;
   location_t l = mt.meta_target()->project()->engine()->root() / 
                  mt.meta_target()->project()->location() /
                  "vc80" / (mt.name().to_string() + ".vcproj");
   
   create_directories(l.branch_path());
   boost::filesystem::ofstream f(l, std::ios_base::trunc);
   write_header(f);
   write_configurations(f);
   write_files(f);
   write_bottom(f);
}
}}