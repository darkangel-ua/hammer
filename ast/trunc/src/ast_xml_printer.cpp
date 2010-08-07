#include "stdafx.h"
#include <hammer/ast/ast_xml_printer.h>
#include <hammer/ast/project_def.h>
#include <hammer/ast/path_like_seq.h>
#include <hammer/ast/requirement.h>
#include <hammer/ast/requirement_set.h>
#include <hammer/ast/target_ref.h>
#include <iostream>
#include <iomanip>

namespace hammer{namespace ast{

static void write_indent(std::ostream& os, unsigned indent)
{
   os << std::setw(indent);
}

ast_xml_printer::ast_xml_printer(std::ostream& os,
                                 bool write_header) 
   : os_(os), indent_(0)
{
   if (write_header)
      os_ << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
}

bool ast_xml_printer::visit_enter(const hamfile& v)
{
   os_ << "<hamfile>\n";
   indent_ += 3;

   return true; 
}

bool ast_xml_printer::visit_leave(const hamfile& v) 
{
   os_ << "</hamfile>\n";
   indent_ -= 3;
   
   return true; 
}

bool ast_xml_printer::visit_enter(const explicit_project_def& v)
{
   os_ << std::setw(indent_) << ' ' << "<explicit_project_def>\n";
   indent_ += 3;

   return true; 
}

bool ast_xml_printer::visit_leave(const explicit_project_def& v)
{
   indent_ -= 3;
   os_ << std::setw(indent_) << ' ' << "</explicit_project_def>\n";

   return true; 
}

bool ast_xml_printer::visit(const implicit_project_def& v)
{
   os_ << std::setw(indent_) << ' ' << "<implicit_project_def/>\n";

   return true; 
}

bool ast_xml_printer::visit_enter(const rule_invocation& v) 
{ 
   os_ << std::setw(indent_) << ' ' << "<rule_invocation name=\"" << v.name() << "\">\n";
   indent_ += 3;

   return true; 
}

bool ast_xml_printer::visit_leave(const rule_invocation& v) 
{ 
   indent_ -= 3;
   os_ << std::setw(indent_) << ' ' << "</rule_invocation>\n";
   
   return true; 
}

bool ast_xml_printer::visit_enter(const list_of& v)
{
   os_ << std::setw(indent_) << ' ' << "<list_of>\n";
   indent_ += 3;

   return true; 
}

bool ast_xml_printer::visit_leave(const list_of& v)
{
   indent_ -= 3;
   os_ << std::setw(indent_) << ' ' << "</list_of>\n";
   
   return true; 
}

bool ast_xml_printer::visit(const empty_expr& v)
{
   return true;
}

bool ast_xml_printer::visit(const id_expr& v) 
{ 
   return true; 
}

bool ast_xml_printer::visit(const path_like_seq& v)
{
   os_ << std::setw(indent_) << ' ' << "<path_like_seq value=\"" << v.to_identifier() << "\"/>\n";

   return true;
}

bool ast_xml_printer::visit(const statements_t& v)
{
   if (v.empty())
      return true; 

   os_ << std::setw(indent_) << ' ' << "<statements>\n";
   indent_ += 3;

   for(statements_t::const_iterator i = v.begin(), last = v.end(); i != last; ++i)
      (**i).accept(*this);

   indent_ -= 3;
   os_ << std::setw(indent_) << ' ' << "</statements>\n";
   
   return true;
}

bool ast_xml_printer::visit(const requirements_t& v)
{
   if (v.empty())
      return true; 

   os_ << std::setw(indent_) << ' ' << "<requirements>\n";
   indent_ += 3;

   for(requirements_t::const_iterator i = v.begin(), last = v.end(); i != last; ++i)
      (**i).accept(*this);

   indent_ -= 3;
   os_ << std::setw(indent_) << ' ' << "</requirements>\n";

   return true;

}

bool ast_xml_printer::visit(const expressions_t& v)
{
   if (v.empty())
      return true; 

   os_ << std::setw(indent_) << ' ' << "<expressions>\n";
   indent_ += 3;

   for(expressions_t::const_iterator i = v.begin(), last = v.end(); i != last; ++i)
      (**i).accept(*this);

   indent_ -= 3;
   os_ << std::setw(indent_) << ' ' << "</expressions>\n";

   return true;
}

bool ast_xml_printer::visit_enter(const requirement_set& v)
{
   os_ << std::setw(indent_) << ' ' << "<requirement_set>\n";
   indent_ += 3;

   return true; 
}

bool ast_xml_printer::visit_leave(const requirement_set& v)
{
   indent_ -= 3;
   os_ << std::setw(indent_) << ' ' << "</requirement_set>\n";

   return true; 
}

bool ast_xml_printer::visit(const simple_requirement& v)
{
   os_ << std::setw(indent_) << ' ' << "<simple_requirement public=\"" << v.is_public() << "\"\n";
   os_ << std::setw(indent_) << ' ' << "                    name=\"" << v.name() << "\">\n";

   indent_ += 3;
      os_ << std::setw(indent_) << ' ' << "<value>\n";
      indent_ += 3;
         v.value()->accept(*this);
      indent_ -= 3;
      os_ << std::setw(indent_) << ' ' << "</value>\n";
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</simple_requirement>\n";

   return true;
}

bool ast_xml_printer::visit(const conditional_requirement& v)
{
   os_ << std::setw(indent_) << ' ' << "<conditional_requirement public=\"" << v.is_public() << "\">\n";

   indent_ += 3;
      visit(v.features());
      os_ << std::setw(indent_) << ' ' << "<value>\n";
      indent_ += 3;
         v.value()->accept(*this);
      indent_ -= 3;
      os_ << std::setw(indent_) << ' ' << "</value>\n";
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</conditional_requirement>\n";

   return true;

}

bool ast_xml_printer::visit(const target_ref& v)
{
   os_ << std::setw(indent_) << ' ' << "<target_ref public=\"" << v.is_public() << "\"\n";
   os_ << std::setw(indent_) << ' ' << "            head=\"" << v.head()->to_identifier() << "\"\n";
   os_ << std::setw(indent_) << ' ' << "            target_name=\"";
   if (v.has_target_name())
      os_ << v.target_name();
   os_ << "\">\n";

   indent_ += 3;

      if (v.requirements())
         v.requirements()->accept(*this);

   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</target_ref>\n";

   return true;
}

}}
