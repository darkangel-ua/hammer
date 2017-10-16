#include "stdafx.h"
#include <hammer/ast/ast_xml_printer.h>
#include <hammer/ast/path.h>
#include <hammer/ast/requirement_set.h>
#include <hammer/ast/usage_requirements.h>
#include <hammer/ast/target_ref.h>
#include <hammer/ast/feature.h>
#include <hammer/ast/feature_set.h>
#include <hammer/ast/sources.h>
#include <hammer/ast/list_of.h>
#include <hammer/ast/rule_invocation.h>
#include <hammer/ast/hamfile.h>
#include <hammer/ast/condition.h>
#include <hammer/ast/target_def.h>
#include <iostream>
#include <iomanip>

namespace hammer{namespace ast{

ast_xml_printer::ast_xml_printer(std::ostream& os,
                                 bool write_header) 
   : os_(os), indent_(0)
{
   if (write_header)
      os_ << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
}

bool ast_xml_printer::visit(const hamfile& v)
{
   os_ << "<hamfile>\n";

   indent_ += 3;
   if (v.get_project_def())
      v.get_project_def()->accept(*this);
   for(const statement* s : v.get_statements())
      s->accept(*this);
   indent_ -= 3;

   os_ << "</hamfile>\n";

   return true; 
}

bool ast_xml_printer::visit(const rule_invocation& v)
{ 
   os_ << std::setw(indent_) << ' ' << "<rule_invocation name=\"" << v.name() << "\">\n";

   indent_ += 3;
      os_ << std::setw(indent_) << ' ' << "<arguments>\n";

      indent_ += 3;
      for(const expression* e : v.arguments())
         e->accept(*this);
      indent_ -= 3;

      os_ << std::setw(indent_) << ' ' << "</arguments>\n";
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</rule_invocation>\n";

   return true; 
}

bool ast_xml_printer::visit(const list_of& v)
{
   os_ << std::setw(indent_) << ' ' << "<list_of>\n";

   indent_ += 3;
   for(const expression* e : v.values())
      e->accept(*this);
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</list_of>\n";

   return true; 
}

bool ast_xml_printer::visit(const empty_expr& v)
{
   os_ << std::setw(indent_) << ' ' << "<empty_expr/>\n";

   return true;
}

bool ast_xml_printer::visit(const id_expr& v)
{ 
   os_ << std::setw(indent_) << ' ' << "<id_expr value=\"" << v.id().to_string() << "\"/>\n";

   return true;
}

bool ast_xml_printer::visit(const named_expr& v)
{
   os_ << std::setw(indent_) << ' ' << "<named_expr name=\"" << v.name() << "\">\n";

   indent_ += 3;
   v.value()->accept(*this);
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</named_expr>\n";

   return true;   
}

bool ast_xml_printer::visit(const public_expr& v)
{
   os_ << std::setw(indent_) << ' ' << "<public_expr>\n";

   indent_ += 3;
   v.value()->accept(*this);
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</public_expr>\n";

   return true;
}

bool ast_xml_printer::visit(const path& v)
{
   os_ << std::setw(indent_) << ' ' << "<path value=\"" << v.to_string() << "\"/>\n";

   return true;
}

bool ast_xml_printer::visit(const requirement_set& v)
{
   os_ << std::setw(indent_) << ' ' << "<requirement_set>\n";

   indent_ += 3;
      v.requirements()->accept(*this);
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</requirement_set>\n";

   return true;
}

bool ast_xml_printer::visit(const usage_requirements& v)
{
   os_ << std::setw(indent_) << ' ' << "<usage_requirements>\n";

   indent_ += 3;
      v.requirements()->accept(*this);
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</usage_requirements>\n";

   return true;
}

bool ast_xml_printer::visit(const error_expression& v)
{
   os_ << std::setw(indent_) << ' ' << "<error_expression/>\n";

   return true;
}

bool ast_xml_printer::visit(const target_ref& v)
{
   os_ << std::setw(indent_) << ' ' << "<target_ref public=\"" << v.is_public() << "\"\n";
   os_ << std::setw(indent_) << ' ' << "            target_path=\"" << v.target_path()->to_string() << "\"\n";
   os_ << std::setw(indent_) << ' ' << "            target_name=\"";
   if (v.has_target_name())
      os_ << v.target_name();
   os_ << "\">\n";

   indent_ += 3;

      if (!v.build_request().empty()) {
         os_ << std::setw(indent_) << ' ' << "<build_request>\n";
         indent_ += 3;
            for (const feature* f : v.build_request())
               f->accept(*this);
         indent_ -= 3;
         os_ << std::setw(indent_) << ' ' << "</build_request>\n";
      }

   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</target_ref>\n";

   return true;
}

bool ast_xml_printer::visit(const target_def& v)
{
   os_ << std::setw(indent_) << ' ' << "<target_def local=\"" << v.local_tag().valid() << "\"\n"
       << std::setw(indent_) << ' ' << "            explicit=\"" << v.explicit_tag().valid() << "\">\n";

   indent_ += 3;
      v.body()->accept(*this);
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</target_def>\n";

   return true;
}

bool ast_xml_printer::visit(const feature& v)
{
   os_ << std::setw(indent_) << ' ' << "<feature name=\"" << v.name() << "\">\n";

   indent_ += 3;
      os_ << std::setw(indent_) << ' ' << "<value>\n";
      indent_ += 3;
         v.value()->accept(*this);
      indent_ -= 3;
      os_ << std::setw(indent_) << ' ' << "</value>\n";
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</feature>\n";

   return true;
}

bool ast_xml_printer::visit(const feature_set& v)
{
   os_ << std::setw(indent_) << ' ' << "<feature_set>\n";

   indent_ += 3;
      v.values()->accept(*this);
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</feature_set>\n";

   return true;
}

bool ast_xml_printer::visit(const sources& v)
{
   os_ << std::setw(indent_) << ' ' << "<sources>\n";

   indent_ += 3;
      v.content()->accept(*this);
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</sources>\n";

   return true;
}

bool ast_xml_printer::visit(const condition_expr& v)
{
   os_ << std::setw(indent_) << ' ' << "<condition_expr>\n";

   indent_ += 3;
      os_ << std::setw(indent_) << ' ' << "<condition>\n";
      indent_ += 3;
         v.condition()->accept(*this);
      indent_ -= 3;
      os_ << std::setw(indent_) << ' ' << "</condition>\n";

      os_ << std::setw(indent_) << ' ' << "<result>\n";
      indent_ += 3;
         v.result()->accept(*this);
      indent_ -= 3;
      os_ << std::setw(indent_) << ' ' << "</result>\n";
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</condition_expr>\n";

   return true;
}

bool ast_xml_printer::visit(const logical_or& v)
{
   os_ << std::setw(indent_) << ' ' << "<logical_or>\n";

   indent_ += 3;
      os_ << std::setw(indent_) << ' ' << "<left>\n";
      indent_ += 3;
         v.left()->accept(*this);
      indent_ -= 3;
      os_ << std::setw(indent_) << ' ' << "</left>\n";

      os_ << std::setw(indent_) << ' ' << "<right>\n";
      indent_ += 3;
         v.right()->accept(*this);
      indent_ -= 3;
      os_ << std::setw(indent_) << ' ' << "</right>\n";
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</logical_or>\n";

   return true;
}

bool ast_xml_printer::visit(const logical_and& v)
{
   os_ << std::setw(indent_) << ' ' << "<logical_and>\n";

   indent_ += 3;
      os_ << std::setw(indent_) << ' ' << "<left>\n";
      indent_ += 3;
         v.left()->accept(*this);
      indent_ -= 3;
      os_ << std::setw(indent_) << ' ' << "</left>\n";

      os_ << std::setw(indent_) << ' ' << "<right>\n";
      indent_ += 3;
         v.right()->accept(*this);
      indent_ -= 3;
      os_ << std::setw(indent_) << ' ' << "</right>\n";
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</logical_and>\n";

   return true;
}

bool ast_xml_printer::visit(const expression_statement& v)
{
   os_ << std::setw(indent_) << ' ' << "<expression_statement>\n";

   indent_ += 3;
      v.content()->accept(*this);
   indent_ -= 3;

   os_ << std::setw(indent_) << ' ' << "</expression_statement>\n";

   return true;
}

}}
