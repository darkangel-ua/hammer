#pragma once
#include <hammer/ast/types.h>

namespace hammer { namespace ast {

class hamfile;
class project_def;
class rule_invocation;
class rule_arguments;
class error_expression;
class empty_expr;
class named_expr;
class public_expr;
class id_expr;
class path;
class list_of;
class requirement_set;
class usage_requirements;
class target_ref;
class target_def;
class feature;
class feature_set;
class sources;
class condition_expr;
class logical_or;
class logical_and;
class expression_statement;
class struct_expr;

class visitor {
   public:
      virtual bool visit(const hamfile& v) { return true; }
      virtual bool visit(const project_def& v) { return true; }
      virtual bool visit(const rule_invocation& v) { return true; }
      virtual bool visit(const list_of& v) { return true; }
      virtual bool visit(const requirement_set& v) { return true; }
		virtual bool visit(const usage_requirements& v) { return true; }
		virtual bool visit(const error_expression& v) { return true; }
      virtual bool visit(const empty_expr& v) { return true; }
      virtual bool visit(const id_expr& v) { return true; }
      virtual bool visit(const named_expr& v) { return true; }
		virtual bool visit(const public_expr& v) { return true; }
		virtual bool visit(const path& v) { return true; }
		virtual bool visit(const target_ref& v) { return true; }
		virtual bool visit(const target_def& v) { return true; }
		virtual bool visit(const feature& v) { return true; }
		virtual bool visit(const feature_set& v) { return true; }
		virtual bool visit(const sources& v) { return true; }
		virtual bool visit(const condition_expr& v) { return true; }
		virtual bool visit(const logical_or& v) { return true; }
		virtual bool visit(const logical_and& v) { return true; }
		virtual bool visit(const expression_statement& v) { return true; }
      virtual bool visit(const struct_expr& v) { return true; }
};

}}
