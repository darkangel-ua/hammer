#if !defined(h_7511b8bd_5dcc_4be4_94ca_7d6c633fa1d2)
#define h_7511b8bd_5dcc_4be4_94ca_7d6c633fa1d2

#include <hammer/parscore/identifier.h>
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
#include <vector>
#include <map>

namespace hammer{

namespace ast{ class expression; }

class project;
class diagnostic;
class rule_manager;

enum class rule_argument_type {
	void_,
	invocation_context,
	target_invocation_context,
	identifier,
	identifier_of_list_of_identifiers,
	feature,
	feature_set,
	feature_or_feature_set,
	sources,
	requirement_set,
	usage_requirements,
	path,
	target_ref,
	ast_expression
};

template<typename T>
struct rule_argument_type_info {
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

	bool local_;
	bool explicit_;
};

template<>
struct rule_argument_type_info<parscore::identifier> { static const rule_argument_type ast_type = rule_argument_type::identifier; };

template<>
struct rule_argument_type_info<invocation_context> { static const rule_argument_type ast_type = rule_argument_type::invocation_context; };

template<>
struct rule_argument_type_info<target_invocation_context> { static const rule_argument_type ast_type = rule_argument_type::target_invocation_context; };

template<>
struct rule_argument_type_info<ast::expression> { static const rule_argument_type ast_type = rule_argument_type::ast_expression; };

class rule_argument
{
   public:   
		rule_argument(rule_argument_type type,
                    const parscore::identifier& name,
                    bool optional) 
         : type_(type), 
           name_(name),
           optional_(optional)
      {}
      
		rule_argument_type type() const { return type_; }
      const parscore::identifier& name() const { return name_; }
      bool is_optional() const { return optional_; }

   private:
		rule_argument_type type_;
      parscore::identifier name_;
      bool optional_;
};

typedef std::vector<rule_argument> rule_arguments;

struct rule_manager_arg_base {
   rule_manager_arg_base(void* v) : v_(v) {}
   virtual ~rule_manager_arg_base() {}
   void* v_;
};

template<typename T>
class rule_manager_arg : public rule_manager_arg_base
{
   public:
      rule_manager_arg(T* v) : rule_manager_arg_base(v), owned_(true) {}
		rule_manager_arg(std::unique_ptr<T> v) : rule_manager_arg_base(v.release()), owned_(true) {}
      rule_manager_arg(T& v) : rule_manager_arg_base(&v), owned_(false) {}
		rule_manager_arg(const T& v) : rule_manager_arg_base(&const_cast<T&>(v)), owned_(false) {}
      ~rule_manager_arg() { if (owned_) delete static_cast<T*>(v_); }

   private:
      bool owned_;
};

typedef std::unique_ptr<rule_manager_arg_base> rule_manager_arg_ptr;
typedef std::vector<rule_manager_arg_ptr> rule_manager_arguments_t;

struct rule_manager_invoker_base
{
   virtual rule_manager_arg_ptr invoke(rule_manager_arguments_t& args) const = 0;
   virtual ~rule_manager_invoker_base() {}
};

template<typename Function>
class rule_manager_invoker : public rule_manager_invoker_base
{
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
         return invoke_impl<reduced_indexes>(args, arg_t());
      }

   private:
      boost::function<Function> f_;

		template<typename T>
		T get_arg_impl(rule_manager_arg_ptr& arg,
		               boost::mpl::bool_<true>) const
		{
			return arg ? static_cast<T>(arg->v_) : nullptr;
		}

		template<typename T>
		T get_arg_impl(rule_manager_arg_ptr& arg,
		               boost::mpl::bool_<false>) const
		{
			typedef typename boost::remove_reference<T>::type non_reference_T;
			return *static_cast<non_reference_T*>(arg->v_);
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
                  boost::mpl::integral_c<int, 0> first_arg,
                  Args... args) const
      {
			typedef typename boost::function_types::result_type<Function>::type result_type;
			return invoke_impl_r(r_args, boost::mpl::bool_<boost::is_same<void, result_type>::value>(), first_arg, args...);
		}

      template<typename FArgs, typename... Args>
      rule_manager_arg_ptr
		invoke_impl(rule_manager_arguments_t& r_args,
                  Args... args) const
      {
         typedef typename boost::mpl::front<FArgs>::type arg_t;
         typedef typename boost::mpl::pop_front<FArgs>::type reduced_args;
         return invoke_impl<reduced_args>(r_args, arg_t(), args...);
      }

};

class rule_declaration
{
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
make_one_arg(const parscore::identifier& arg_name,
             boost::mpl::tag<T>)
{
	typedef typename std::remove_cv<
	      typename std::remove_pointer<
	      typename std::remove_reference<T>::type
	      >::type
	      >::type pure_arg_t;

	return rule_argument(rule_argument_type_info<pure_arg_t>::ast_type,
	                     arg_name,
	                     boost::mpl::bool_<boost::is_pointer<T>::value>());
}

inline
rule_argument
make_one_arg(const parscore::identifier& arg_name,
             boost::mpl::tag<void>)
{
	return rule_argument(rule_argument_type::void_, arg_name, false);
}

template<typename T>
rule_argument
make_one_arg(const parscore::identifier& arg_name,
             boost::mpl::tag<std::unique_ptr<T>>)
{
	return rule_argument(rule_argument_type_info<T>::ast_type, arg_name, false);
}

template<typename T>
void push_arg_impl(std::vector<rule_argument>* args,
                   const std::vector<parscore::identifier>& arg_names,
                   boost::mpl::int_<-1>)
{}

template<typename T, int idx>
void push_arg_impl(std::vector<rule_argument>* args,
                   const std::vector<parscore::identifier>& arg_names,
                   boost::mpl::int_<idx>)
{
	typedef typename boost::mpl::at_c<typename boost::function_types::parameter_types<T>::type, idx>::type arg_t;

	args->insert(args->begin(), make_one_arg(arg_names[idx], boost::mpl::tag<arg_t>()));
	push_arg_impl<T>(args, arg_names, boost::mpl::int_<idx - 1>());
}

template<typename T>
std::vector<rule_argument>
make_args(const std::vector<parscore::identifier>& arg_names)
{
	std::vector<rule_argument> result;
	push_arg_impl<T>(&result, arg_names, boost::mpl::int_<boost::function_types::function_arity<T>::value - 1>());
	return result;
}

}

// FIXME: ast_expression can't be optional
class rule_manager
{
   public:
      typedef std::map<parscore::identifier, rule_declaration> rules_t;
      typedef rules_t::const_iterator const_iterator;

      const_iterator begin() const { return rules_.begin(); }
      const_iterator end() const { return rules_.end(); }
		const_iterator find(const parscore::identifier& id) const { return rules_.find(id); }

      template<typename FunctionPointer>
      void add_target(const parscore::identifier& id, 
                      FunctionPointer f,
                      const std::vector<parscore::identifier>& arg_names)
      {
         typedef typename boost::remove_pointer<FunctionPointer>::type f_type;
			typedef typename boost::mpl::at_c<typename boost::function_types::parameter_types<f_type>::type, 0>::type arg_0_type;

			static_assert(boost::is_same<target_invocation_context&, arg_0_type>::value, "First argument for target definition MUST be target_invocation_context");

			add_impl(id, boost::function<f_type>(f), arg_names, /*is_target =*/true);
      }

      template<typename FunctionPointer>
      void add_rule(const parscore::identifier& id, 
                    FunctionPointer f,
                    const std::vector<parscore::identifier>& arg_names)
      {
			typedef typename boost::remove_pointer<FunctionPointer>::type f_type;
			typedef typename boost::mpl::at_c<typename boost::function_types::parameter_types<f_type>::type, 0>::type arg_0_type;

			static_assert(boost::is_same<invocation_context&, arg_0_type>::value, "First argument for rule MUST be invocation_context");
         add_impl(id, boost::function<f_type>(f), arg_names, /*is_target =*/false);
      }

   private:
      rules_t rules_;

      template<typename T>
      void add_impl(const parscore::identifier& id, 
                    boost::function<T> f,
                    std::vector<parscore::identifier> arg_names,
                    bool is_target)
      {
         typedef typename boost::function_types::result_type<T>::type result_type;
         
         if (arg_names.size() != boost::function_types::function_arity<T>::value - 1)
            throw std::runtime_error("[hammer.rule_manager] Not enought argument names");

			arg_names.insert(arg_names.begin(), "$!@ctx$!@");
			rule_declaration decl(id,
                               details::make_args<T>(arg_names), 
                               details::make_one_arg(parscore::identifier(), boost::mpl::tag<result_type>()),
                               is_target,
                               std::shared_ptr<rule_manager_invoker_base>(new rule_manager_invoker<T>(f)));

         if (!rules_.insert(std::make_pair(id, decl)).second)
            throw std::runtime_error("[hammer.rule_manager] Rule '" + id.to_string() + "' already added");
      }
};

}

#endif
