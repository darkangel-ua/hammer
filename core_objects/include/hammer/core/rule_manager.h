#if !defined(h_7511b8bd_5dcc_4be4_94ca_7d6c633fa1d2)
#define h_7511b8bd_5dcc_4be4_94ca_7d6c633fa1d2

#include <hammer/parscore/identifier.h>
#include <boost/function.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <vector>
#include <map>

namespace hammer{
	class feature;
	class feature_set;
   class sources_decl;
   class requirements_decl;
   class usage_requirements;
	class path;
	class target;
}

namespace hammer{

class rule_argument_type
{
	public:
		enum value { VOID, IDENTIFIER, FEATURE, FEATURE_SET, SOURCES, REQUIREMENTS_SET, USAGE_REQUIREMENTS_SET, PATH, TARGET_REF };

		static rule_argument_type::value
		type(const hammer::parscore::identifier*) { return IDENTIFIER; }

		static rule_argument_type::value
		type(const feature*) { return FEATURE; }

		static rule_argument_type::value
		type(const feature_set*) { return FEATURE_SET; }

		static rule_argument_type::value
		type(const sources_decl*) { return SOURCES; }

		static rule_argument_type::value
		type(const requirements_decl*) { return REQUIREMENTS_SET; }

		static rule_argument_type::value
		type(const usage_requirements*) { return USAGE_REQUIREMENTS_SET; }

		static rule_argument_type::value
		type(const path*) { return PATH; }

		static rule_argument_type::value
		type(const target*) { return TARGET_REF; }
};

class rule_argument
{
   public:   
      rule_argument(rule_argument_type::value type,
                    const parscore::identifier& name,
                    bool optional) 
         : type_(type), 
           name_(name),
           optional_(optional)
      {}
      
      rule_argument_type::value type() const { return type_; }
      const parscore::identifier& name() const { return name_; }
      bool is_optional() const { return optional_; }

   private:
      rule_argument_type::value type_;
      parscore::identifier name_;
      bool optional_;
};

typedef std::vector<rule_argument> rule_arguments;

class rule_declaration
{
   public:
      typedef rule_arguments::const_iterator const_iterator;

      rule_declaration(parscore::identifier name,
                const rule_arguments& args,
                const rule_argument& result,
                bool is_target) 
         : name_(name),
           args_(args),
           result_(result),
           is_target_(is_target)
      {}

      const parscore::identifier& name() const { return name_; }
      const rule_argument& result() const { return result_; }
      const rule_arguments& arguments() const { return args_; }
      const_iterator begin() const { return args_.begin(); }
      const_iterator end() const { return args_.end(); }
      const_iterator find(const parscore::identifier& arg_name) const;
      bool is_target() const { return is_target_; }
      

   private:
      parscore::identifier name_;
      rule_arguments args_;
      rule_argument result_;
      bool is_target_;
};

namespace details{
   template<typename T>
   rule_argument make_one_arg(const parscore::identifier& arg_name)
   {
      typedef typename boost::remove_pointer<T>::type not_a_pointer_arg_t;
      typedef typename boost::remove_reference<not_a_pointer_arg_t>::type pure_arg_t;
      
      return rule_argument(rule_argument_type::type(static_cast<const pure_arg_t*>(NULL)), 
                           arg_name,
                           boost::mpl::bool_<boost::is_pointer<T>::value>());
   }

    template<>
    inline rule_argument make_one_arg<void>(const parscore::identifier& arg_name)
    {
       return rule_argument(rule_argument_type::VOID, arg_name, false);
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
      
      args->insert(args->begin(), make_one_arg<arg_t>(arg_names[idx]));
      push_arg_impl<T>(args, arg_names, boost::mpl::int_<idx - 1>());
   }

   template<typename T>
   std::vector<rule_argument> make_args(const std::vector<parscore::identifier>& arg_names)
   {
      std::vector<rule_argument> result;
      push_arg_impl<T>(&result, arg_names, boost::mpl::int_<boost::function_types::function_arity<T>::value - 1>());
      return result;
   }
}

class rule_manager
{
   public:
      typedef std::map<parscore::identifier, rule_declaration> rules_t;
      typedef rules_t::const_iterator const_iterator;

      const_iterator begin() const { return rules_.begin(); }
      const_iterator end() const { return rules_.end(); }
      const_iterator find(const parscore::identifier& id) const
      {
         return rules_.find(id);
      }

      template<typename T>
      void add_target(const parscore::identifier& id, 
                      boost::function<T> f,
                      const std::vector<parscore::identifier>& arg_names)
      {
         add_impl(id, f, arg_names, /*is_target =*/true);
      }

      template<typename T>
      void add_rule(const parscore::identifier& id, 
                    boost::function<T> f,
                    const std::vector<parscore::identifier>& arg_names)
      {
         add_impl(id, f, arg_names, /*is_target =*/false);
      }

   private:
      rules_t rules_;

      template<typename T>
      void add_impl(const parscore::identifier& id, 
                   boost::function<T> f, 
                   const std::vector<parscore::identifier>& arg_names,
                   bool is_target)
      {
         typedef typename boost::function_types::result_type<T>::type result_type;
         
         if (arg_names.size() != boost::function_types::function_arity<T>::value)
            throw std::runtime_error("[hammer.rule_manager] Not enought argument names");

         rule_declaration decl(id, 
                               details::make_args<T>(arg_names), 
                               details::make_one_arg<result_type>(parscore::identifier()), 
                               is_target);

         if (!rules_.insert(std::make_pair(id, decl)).second)
            throw std::runtime_error("[hammer.rule_manager] Rule '" + id.to_string() + "' already added");
      }
};

}

#endif
