#pragma once
#include <map>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/vector_c.hpp>
#include <boost/mpl/reverse.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <hammer/parscore/identifier.h>

namespace hammer {

namespace ast {
   class expression;
   class context;
}

class project;
class diagnostic;
class rule_manager;

enum class rule_argument_meta_type {
   simple,
   one_or_list,
   variant,
   struct_
};

template<typename T>
struct one_or_list {
   using const_iterator = typename std::vector<T>::const_iterator;

   const_iterator begin() const { return value_.begin(); }
   const_iterator end() const { return value_.end(); }

   std::vector<T> value_;
};

template<typename T>
struct rule_argument_meta_type_info {
   static const rule_argument_meta_type meta_type_ = rule_argument_meta_type::simple;
};

template<typename T>
struct rule_argument_meta_type_info<one_or_list<T>> {
   static const rule_argument_meta_type meta_type_ = rule_argument_meta_type::one_or_list;
   using nested_type = T;
};

enum class rule_argument_type {
	void_,
	invocation_context,
	target_invocation_context,
	identifier,
   feature,
	feature_set,
	feature_or_feature_set,
	sources,
	requirement_set,
	usage_requirements,
	path,  // wildcards not allowed
   wcpath, // wildcards allowed
	target_ref,
   target_ref_mask, // target ref without target name and properties and wildcards allowed
	ast_expression
};

class rule_argument;
class rule_manager_arg_base;
struct rule_manager_invoker_base;

struct rule_argument_struct_desc {
   std::string typename_;
   std::shared_ptr<std::vector<rule_argument>> fields_;
   std::shared_ptr<rule_manager_invoker_base> constructor_;
};

struct rule_argument_type_desc;

struct rule_argument_list_desc {
   class basic_list_adaptor;

   template<typename T>
   class list_adaptor;

   using adaptor_constructor_t = boost::function<std::unique_ptr<basic_list_adaptor>()>;

   adaptor_constructor_t make_list;
   std::shared_ptr<rule_argument_type_desc> nested_type_;
};

struct rule_argument_type_desc {
   using variant_types = std::vector<std::shared_ptr<rule_argument_type_desc>>;
   using type = boost::variant<rule_argument_type, rule_argument_list_desc, variant_types, rule_argument_struct_desc>;

   template<typename T>
   rule_argument_type_desc(T&& v) : type_(std::forward<T>(v)) {}

   const rule_argument_type* as_simple() const { return boost::get<rule_argument_type>(&type_); }
   const rule_argument_list_desc* as_list() const { return boost::get<rule_argument_list_desc>(&type_); }
   const variant_types* as_variant() const { return boost::get<variant_types>(&type_); }
   const rule_argument_struct_desc* as_struct() const { return boost::get<rule_argument_struct_desc>(&type_); }

   bool operator == (const rule_argument_type v) const { if (auto s = as_simple()) return v == *s; else return false; }

   type type_;
};

struct invocation_context {
	invocation_context(project& current_project,
							 diagnostic& diag,
	                   rule_manager& rule_manager)
	   : current_project_(current_project),
	     diag_(diag),
	     rule_manager_(rule_manager)
	{}

	project& current_project_;
	diagnostic& diag_;
	rule_manager& rule_manager_;
   parscore::source_location rule_location_;
};

struct target_invocation_context : invocation_context {
	target_invocation_context(project& current_project,
									  diagnostic& diag,
									  rule_manager& rule_manager,
	                          bool local_tag,
	                          bool explicit_tag)
	   : invocation_context(current_project, diag, rule_manager),
	     local_(local_tag),
	     explicit_(explicit_tag)
	{}

   target_invocation_context(invocation_context& ctx,
                             bool local_tag,
	                          bool explicit_tag)
      : invocation_context(ctx),
        local_(local_tag),
	     explicit_(explicit_tag)
	{}

	bool local_;
	bool explicit_;
};

class rule_argument {
   public:   
      using raw_ast_transformer_t = const ast::expression*(*)(ast::context& ctx, diagnostic& diag, const ast::expression*);
      using ast_transformer_t = boost::function<const ast::expression*(ast::context& ctx, diagnostic& diag, const ast::expression*)>;

		rule_argument(const rule_argument_type_desc& type,
                    const parscore::identifier& name,
                    bool optional,
                    ast_transformer_t ast_transformer)
         : type_(type),
           name_(name),
           ast_transformer_(ast_transformer),
           optional_(optional)
      {}
      
		const rule_argument_type_desc& type() const { return type_; }
      const parscore::identifier& name() const { return name_; }
      bool is_optional() const { return optional_; }
      const ast_transformer_t& ast_transformer() const { return ast_transformer_; }

   private:
		rule_argument_type_desc type_;
      parscore::identifier name_;
      ast_transformer_t ast_transformer_;
      bool optional_;
};

typedef std::vector<rule_argument> rule_arguments;

struct rule_argument_decl {
   rule_argument_decl(const char* name) : name_(name) {}
   rule_argument_decl(const char* name,
                      rule_argument::raw_ast_transformer_t transformer)
      : name_(name),
        transformer_(transformer)
   {}

   parscore::identifier name_;
   rule_argument::ast_transformer_t transformer_;
};

using rule_args_decl = std::vector<rule_argument_decl>;

class rule_manager_arg_base : public boost::noncopyable {
	public:
      rule_manager_arg_base(void* v) : v_(v) {}
		virtual ~rule_manager_arg_base() = default;
		void* value() { return v_; }

	protected:
		void* v_;
};

template<typename T>
class rule_manager_arg : public rule_manager_arg_base {
   public:
      rule_manager_arg(T* v) : rule_manager_arg_base(v), owned_(true) {}
		rule_manager_arg(std::unique_ptr<T> v) : rule_manager_arg_base(v.release()), owned_(true) {}
      rule_manager_arg(T& v) : rule_manager_arg_base(&v), owned_(false) {}
      rule_manager_arg(T&& v) : rule_manager_arg_base(new T{std::forward<T>(v)}), owned_(true) {}
		rule_manager_arg(const T& v) : rule_manager_arg_base(&const_cast<T&>(v)), owned_(false) {}
      ~rule_manager_arg() { if (owned_) delete static_cast<T*>(v_); }

   private:
      bool owned_;
};

typedef std::unique_ptr<rule_manager_arg_base> rule_manager_arg_ptr;
typedef std::vector<rule_manager_arg_ptr> rule_manager_arguments_t;

class rule_argument_list_desc::basic_list_adaptor : public rule_manager_arg_base {
   public:
      basic_list_adaptor(void* v) : rule_manager_arg_base(v) {}
      virtual void push_back(rule_manager_arg_ptr arg) = 0;
      virtual ~basic_list_adaptor() = default;
};

template<typename T>
class rule_argument_list_desc::list_adaptor : public rule_argument_list_desc::basic_list_adaptor {
      using value_type = one_or_list<T>;

   public:
      list_adaptor() : basic_list_adaptor(new one_or_list<T>) {}

      void push_back(rule_manager_arg_ptr arg) override {
         static_cast<value_type*>(value())->value_.push_back(*static_cast<T*>(arg->value()));
      }

      ~list_adaptor() { delete static_cast<value_type*>(v_); }
};

template<typename T>
struct rule_argument_type_info {
   static
   rule_argument_type_desc
   ast_type(typename boost::enable_if_c<rule_argument_meta_type::one_or_list == rule_argument_meta_type_info<T>::meta_type_>::type* = nullptr) {
      using nested_type = typename rule_argument_meta_type_info<T>::nested_type;

      auto make_list = [] { return std::unique_ptr<rule_argument_list_desc::basic_list_adaptor>{ new rule_argument_list_desc::list_adaptor<nested_type> }; };

      return rule_argument_list_desc{ make_list, std::make_shared<rule_argument_type_desc>(rule_argument_type_info<nested_type>::ast_type()) };
   }
};

#define HAMMER_RULE_MANAGER_SIMPLE_TYPE(type, atype) \
   template<> \
   struct rule_argument_type_info< type > { static rule_argument_type_desc ast_type() { return { rule_argument_type:: atype }; } }

HAMMER_RULE_MANAGER_SIMPLE_TYPE(parscore::identifier, identifier);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(invocation_context, invocation_context);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(target_invocation_context, target_invocation_context);
HAMMER_RULE_MANAGER_SIMPLE_TYPE(ast::expression, ast_expression);

struct rule_manager_invoker_base {
   virtual rule_manager_arg_ptr invoke(rule_manager_arguments_t& args) const = 0;
   virtual ~rule_manager_invoker_base() {}
};

template<typename Function>
class rule_manager_invoker : public rule_manager_invoker_base {
   public:
      rule_manager_invoker(boost::function<Function> f) : f_(f) {}

      rule_manager_arg_ptr
      invoke(rule_manager_arguments_t& args) const override
      {
         typedef boost::mpl::range_c<int, 0, boost::function_types::function_arity<Function>::value> indexes_t;
         typedef typename boost::mpl::copy<indexes_t, boost::mpl::back_inserter<boost::mpl::vector_c<int>>>::type v_indexes_t;
         typedef typename boost::mpl::reverse<v_indexes_t>::type reversed_v_indexes_t;

         if (args.size() != boost::function_types::function_arity<Function>::value)
            throw std::runtime_error("[rule_manager_invoker] Not enought arguments");

         typedef typename boost::mpl::front<reversed_v_indexes_t>::type arg_t;
         typedef typename boost::mpl::pop_front<reversed_v_indexes_t>::type reduced_indexes;
			return invoke_impl<reduced_indexes>(args, typename boost::mpl::empty<reduced_indexes>::type{}, arg_t{});
      }

   private:
      boost::function<Function> f_;

		template<typename T>
		T get_arg_impl(rule_manager_arg_ptr& arg,
		               boost::mpl::bool_<true>) const
		{
			return arg ? static_cast<T>(arg->value()) : nullptr;
		}

		template<typename T>
		T get_arg_impl(rule_manager_arg_ptr& arg,
		               boost::mpl::bool_<false>) const
		{
			typedef typename boost::remove_reference<T>::type non_reference_T;
			return *static_cast<non_reference_T*>(arg->value());
		}

		template<int Idx>
		typename boost::mpl::at_c<typename boost::function_types::parameter_types<Function>::type, Idx>::type
		get_arg(rule_manager_arguments_t& args,
		        boost::mpl::integral_c<int, Idx>) const
		{
			typedef typename boost::mpl::at_c<typename boost::function_types::parameter_types<Function>::type, Idx>::type arg_t;
			return get_arg_impl<arg_t>(args[Idx], boost::mpl::bool_<boost::is_pointer<arg_t>::value>());
		}

		template<typename... Args>
		rule_manager_arg_ptr
		invoke_impl_r(rule_manager_arguments_t& r_args,
                    boost::mpl::bool_<true>,
                    Args... args) const
      {
			f_(get_arg(r_args, args)...);
			return {};
		}

		template<typename T>
		rule_manager_arg_ptr
		make_result(std::unique_ptr<T> from) const
		{
			rule_manager_arg_ptr result{new rule_manager_arg<T>{from.get()}};
			from.release();

			return result;
		}

		template<typename... Args>
		rule_manager_arg_ptr
		invoke_impl_r(rule_manager_arguments_t& r_args,
                    boost::mpl::bool_<false>,
                    Args... args) const
      {
			return make_result(f_(get_arg(r_args, args)...));
		}

		template<typename FArgs, typename... Args>
		rule_manager_arg_ptr
		invoke_impl(rule_manager_arguments_t& r_args,
                  boost::mpl::true_,
                  Args... args) const
      {
			typedef typename boost::function_types::result_type<Function>::type result_type;
			return invoke_impl_r(r_args, boost::mpl::bool_<boost::is_same<void, result_type>::value>(), args...);
		}

      template<typename FArgs, typename... Args>
      rule_manager_arg_ptr
		invoke_impl(rule_manager_arguments_t& r_args,
		            boost::mpl::false_,
                  Args... args) const
      {
         typedef typename boost::mpl::front<FArgs>::type arg_t;
         typedef typename boost::mpl::pop_front<FArgs>::type reduced_args;
			return invoke_impl<reduced_args>(r_args, typename boost::mpl::empty<reduced_args>::type{}, arg_t{}, args...);
      }

};

class rule_declaration {
   public:
      typedef rule_arguments::const_iterator const_iterator;

      rule_declaration(parscore::identifier name,
                       const rule_arguments& args,
                       const rule_argument& result,
                       bool is_target,
                       std::shared_ptr<rule_manager_invoker_base> invoker)
         : name_(name),
           args_(args),
           result_(result),
           is_target_(is_target),
           invoker_(invoker)
      {}

      const parscore::identifier& name() const { return name_; }
      const rule_argument& result() const { return result_; }
      const rule_arguments& arguments() const { return args_; }
      const_iterator begin() const { return args_.begin(); }
      const_iterator end() const { return args_.end(); }
      const_iterator find(const parscore::identifier& arg_name) const;
      bool is_target() const { return is_target_; }

      rule_manager_arg_ptr
      invoke(rule_manager_arguments_t& arguments) const;


   private:
      parscore::identifier name_;
      rule_arguments args_;
      rule_argument result_;
      bool is_target_;
      std::shared_ptr<rule_manager_invoker_base> invoker_;
};

namespace details {

template<typename T>
rule_argument
make_one_arg(const rule_argument_decl& arg_decl,
             boost::mpl::tag<T>)
{
	typedef typename std::remove_cv<
	      typename std::remove_pointer<
	      typename std::remove_reference<T>::type
	      >::type
	      >::type pure_arg_t;

   return rule_argument(rule_argument_type_info<pure_arg_t>::ast_type(),
	                     arg_decl.name_,
	                     boost::mpl::bool_<boost::is_pointer<T>::value>(),
                        arg_decl.transformer_);
}

inline
rule_argument
make_one_arg(const rule_argument_decl& arg_decl,
             boost::mpl::tag<void>)
{
	return rule_argument(rule_argument_type::void_, arg_decl.name_, false, arg_decl.transformer_);
}

template<typename T>
rule_argument
make_one_arg(const rule_argument_decl& arg_decl,
             boost::mpl::tag<std::unique_ptr<T>>)
{
	return rule_argument(rule_argument_type_info<T>::ast_type(), arg_decl.name_, false, arg_decl.transformer_);
}

template<typename T>
void push_arg_impl(std::vector<rule_argument>* args,
                   const rule_args_decl& args_decl,
                   boost::mpl::int_<-1>)
{}

template<typename T, int idx>
void push_arg_impl(std::vector<rule_argument>* args,
                   const rule_args_decl& args_decl,
                   boost::mpl::int_<idx>)
{
	typedef typename boost::mpl::at_c<typename boost::function_types::parameter_types<T>::type, idx>::type arg_t;

	args->insert(args->begin(), make_one_arg(args_decl[idx], boost::mpl::tag<arg_t>()));
	push_arg_impl<T>(args, args_decl, boost::mpl::int_<idx - 1>());
}

template<typename T>
std::vector<rule_argument>
make_args(const rule_args_decl& args_decl)
{
	std::vector<rule_argument> result;
	push_arg_impl<T>(&result, args_decl, boost::mpl::int_<boost::function_types::function_arity<T>::value - 1>());
	return result;
}

template<typename T>
rule_argument_struct_desc
make_rule_argument_struct_desc_impl(const std::string& type,
                                    boost::function<T>&& constructor,
                                    const rule_args_decl& fields_decl) {
   std::vector<rule_argument> fields;
   for (auto& f : make_args<T>(fields_decl))
      fields.emplace_back(std::move(f));

   auto invoker = std::make_shared<rule_manager_invoker<T>>(constructor);

   return { type, std::make_shared<rule_arguments>(std::move(fields)), std::move(invoker) };
}

}

template<typename FunctionPointer>
rule_argument_struct_desc
make_rule_argument_struct_desc(const std::string& type,
                               FunctionPointer constructor,
                               const rule_args_decl& fields_decl) {
   using f_type = typename boost::remove_pointer<FunctionPointer>::type;

   if (fields_decl.size() != boost::function_types::function_arity<f_type>::value)
      throw std::runtime_error("[hammer.rule_manager] Not enought field descriptions for struct '" + type + "'");

   return details::make_rule_argument_struct_desc_impl(type, boost::function<f_type>(constructor), fields_decl);
}

template< class Ret, class T, class ...Args >
auto bind_this(T* this_, Ret (T::* f)(Args...)) -> boost::function<Ret(Args...)> {
    return [=]( Args... args ) { return (this_->*f)(std::forward<Args>(args)...); };
}

// FIXME: ast_expression can't be optional
class rule_manager {
   public:
      typedef std::map<parscore::identifier, rule_declaration> rules_t;
      typedef rules_t::const_iterator const_iterator;

      const_iterator begin() const { return rules_.begin(); }
      const_iterator end() const { return rules_.end(); }
		const_iterator find(const parscore::identifier& id) const { return rules_.find(id); }

      template<typename FunctionPointer>
      void add_target(const parscore::identifier& id, 
                      FunctionPointer f,
                      const rule_args_decl& args_decl)
      {
         typedef typename boost::remove_pointer<FunctionPointer>::type f_type;
			typedef typename boost::mpl::at_c<typename boost::function_types::parameter_types<f_type>::type, 0>::type arg_0_type;

			static_assert(boost::is_same<target_invocation_context&, arg_0_type>::value, "First argument for target definition MUST be target_invocation_context");

			add_impl(id, boost::function<f_type>(f), args_decl, /*is_target =*/true);
      }

      template<typename FunctionPointer>
      void add_rule(const parscore::identifier& id, 
                    FunctionPointer f,
                    const rule_args_decl& args_decl)
      {
			typedef typename boost::remove_pointer<FunctionPointer>::type f_type;
			typedef typename boost::mpl::at_c<typename boost::function_types::parameter_types<f_type>::type, 0>::type arg_0_type;

			static_assert(boost::is_same<invocation_context&, arg_0_type>::value, "First argument for rule MUST be invocation_context");
         add_impl(id, boost::function<f_type>(f), args_decl, /*is_target =*/false);
      }

		void add(const rule_declaration& decl)
		{
			if (!rules_.insert(std::make_pair(decl.name(), decl)).second)
            throw std::runtime_error("[hammer.rule_manager] Rule/target '" + decl.name().to_string() + "' already added");
		}

		template<typename FunctionType>
		static
		rule_declaration
		make_rule_declaration(const parscore::identifier& id,
		                      boost::function<FunctionType> f,
		                      const rule_args_decl& args_decl)
		{
			typedef typename boost::mpl::at_c<typename boost::function_types::parameter_types<FunctionType>::type, 0>::type arg_0_type;

			static_assert(boost::is_same<invocation_context&, arg_0_type>::value || boost::is_same<target_invocation_context&, arg_0_type>::value,
			              "First argument for rule MUST be invocation_context or target_invocation_context");

         if (args_decl.size() != boost::function_types::function_arity<FunctionType>::value - 1)
            throw std::runtime_error("[hammer.rule_manager] Not enought argument names");

		   return make_rule_declaration_impl(id, std::move(f), args_decl, boost::is_same<target_invocation_context&, arg_0_type>::value);
		}

		template<typename FunctionPointer>
		static
		rule_declaration
		make_rule_declaration(const parscore::identifier& id,
		                      FunctionPointer f,
		                      const rule_args_decl& args_decl)
		{
			typedef typename boost::remove_pointer<FunctionPointer>::type f_type;
		   return make_rule_declaration(id, boost::function<f_type>(f), args_decl);
		}

      template<typename T, typename R>
		static
		rule_declaration
		make_rule_declaration(const parscore::identifier& id,
                            T* this_,
		                      R T::* f,
		                      const rule_args_decl& args_decl)
		{
		   return make_rule_declaration(id, bind_this(this_, f), args_decl);
		}

   private:
      rules_t rules_;

      template<typename T>
      void add_impl(const parscore::identifier& id, 
                    boost::function<T> f,
                    rule_args_decl args_decl,
                    bool is_target)
      {
         typedef typename boost::function_types::result_type<T>::type result_type;
         
         if (args_decl.size() != boost::function_types::function_arity<T>::value - 1)
            throw std::runtime_error("[hammer.rule_manager] Not enought argument names");

			args_decl.insert(args_decl.begin(), "$!@ctx$!@");
			rule_declaration decl(id,
                               details::make_args<T>(args_decl),
                               details::make_one_arg("", boost::mpl::tag<result_type>()),
                               is_target,
                               std::shared_ptr<rule_manager_invoker_base>(new rule_manager_invoker<T>(f)));

         add(decl);
      }

		template<typename T>
		static
		rule_declaration
		make_rule_declaration_impl(const parscore::identifier& id,
		                           boost::function<T> f,
		                           rule_args_decl args_decl,
		                           bool is_target)
		{
			typedef typename boost::function_types::result_type<T>::type f_result_type;

         args_decl.insert(args_decl.begin(), "$!@ctx$!@");
			rule_declaration decl(id,
                               details::make_args<T>(args_decl),
                               details::make_one_arg("", boost::mpl::tag<f_result_type>()),
                               is_target,
                               std::shared_ptr<rule_manager_invoker_base>(new rule_manager_invoker<T>(f)));

			return decl;
		}

};

}
