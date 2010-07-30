#include "stdafx.h"
#include <hammer/ast/ast_xml_printer.h>
#include <hammer/ast/project_def.h>
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
   return true; 
}

bool ast_xml_printer::visit_leave(const rule_invocation& v) 
{ 
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
   return true;
}

}}
