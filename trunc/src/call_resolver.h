#pragma once
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <stdexcept>
#include <vector>
#include <map>

namespace hammer
{
   class call_resolver_arg_def
   {
      public:
         call_resolver_arg_def(const std::type_info* ti, 
                               bool is_optional = false) 
                               : 
                                ti_(ti), is_optional_(is_optional) 
         {}
         
         const std::type_info& ti() const { return *ti_; }

      private:
         const std::type_info* ti_;
         bool is_optional_;
   };

   class call_resolver_arg_base
   {
      
   };

   class call_resolver_call_arg_base : public boost::noncopyable
   {
      public:
         call_resolver_call_arg_base(void *a, bool owned) : arg_(a), owned_(owned) {}
         virtual ~call_resolver_call_arg_base(){}
         void* value() const { return arg_; }

      protected:
         void* arg_;
         bool owned_;
   };

   typedef boost::ptr_vector<call_resolver_call_arg_base> args_list_t;

   struct null_arg{};

   template<typename T>
   class call_resolver_call_arg : public call_resolver_call_arg_base
   {
      public:
         call_resolver_call_arg(void *a, bool owned) : call_resolver_call_arg_base(a, owned) {}
         virtual ~call_resolver_call_arg()
         {
            if (owned_)
               delete static_cast<T*>(arg_);
         }
         T* value() const { return static_cast<T*>(arg_);}
   };

   class call_resolver_function_base : public boost::noncopyable
   {
      public:
         typedef std::vector<call_resolver_arg_def> args_t;
         call_resolver_function_base(const args_t& args, 
                                     const call_resolver_arg_def& result_def) 
                                     : result_def_(result_def), args_(args) {}
         virtual std::auto_ptr<call_resolver_call_arg_base> invoke(args_list_t& args) = 0;
         virtual ~call_resolver_function_base() {}

      protected:
         args_t args_;
         call_resolver_arg_def result_def_;
   };

   template<typename arg_t>
   struct arg_getter
   {
      typedef call_resolver_function_base::args_t arg_defs_t;
      static arg_t get(args_list_t& args, const arg_defs_t& defs, unsigned int idx, boost::mpl::true_)
      {
         if (args.size() <= idx)
            return static_cast<arg_t>(0);
         else
            return static_cast<arg_t>(args.at(idx).value());
      }

      static arg_t get(args_list_t& args, const arg_defs_t& defs, unsigned int idx, boost::mpl::false_)
      {
         if (args.size() <= idx)
            throw std::runtime_error("Not enough arguments for function.");

         typedef typename boost::remove_reference<arg_t>::type T;
         return *static_cast<T*>(args.at(idx).value());
      }

      static arg_t get(args_list_t& args, const arg_defs_t& defs, unsigned int idx)
      {
         return get(args, defs, idx, boost::mpl::bool_<boost::is_pointer<arg_t>::value>());
      }
   };

   template<typename T>
   class call_resolver_function : public call_resolver_function_base
   {
      public:
         call_resolver_function(const boost::function<T>& f,
                                const args_t& args, 
                                const call_resolver_arg_def& result_def) 
            : call_resolver_function_base(args, result_def), f_(f) 
         {}
         
         std::auto_ptr<call_resolver_call_arg_base> invoke(args_list_t& args, boost::mpl::int_<0>)
         {
            f_();
            
            return std::auto_ptr<call_resolver_call_arg_base>();
         }

         std::auto_ptr<call_resolver_call_arg_base> invoke(args_list_t& args, boost::mpl::int_<1>)
         {
            typedef boost::mpl::at_c<boost::function_types::parameter_types<T>::type, 0>::type arg0_t;
            
            f_(arg_getter<arg0_t>::get(args, args_, 0));
            
            return std::auto_ptr<call_resolver_call_arg_base>();
         }

         std::auto_ptr<call_resolver_call_arg_base> invoke(args_list_t& args, boost::mpl::int_<2>)
         {
            typedef boost::mpl::at_c<boost::function_types::parameter_types<T>::type, 0>::type arg0_t;
            typedef boost::mpl::at_c<boost::function_types::parameter_types<T>::type, 1>::type arg1_t;
            
            f_(arg_getter<arg0_t>::get(args, args_, 0), arg_getter<arg1_t>::get(args, args_, 1));
            
            return std::auto_ptr<call_resolver_call_arg_base>();
         }

         std::auto_ptr<call_resolver_call_arg_base> invoke(args_list_t& args, boost::mpl::int_<3>)
         {
            typedef boost::mpl::at_c<boost::function_types::parameter_types<T>::type, 0>::type arg0_t;
            typedef boost::mpl::at_c<boost::function_types::parameter_types<T>::type, 1>::type arg1_t;
            typedef boost::mpl::at_c<boost::function_types::parameter_types<T>::type, 2>::type arg2_t;
            
            f_(arg_getter<arg0_t>::get(args, args_, 0), arg_getter<arg1_t>::get(args, args_, 1), 
               arg_getter<arg2_t>::get(args, args_, 2));

            return std::auto_ptr<call_resolver_call_arg_base>();
         }

         std::auto_ptr<call_resolver_call_arg_base> invoke(args_list_t& args, boost::mpl::int_<4>)
         {
            typedef boost::mpl::at_c<boost::function_types::parameter_types<T>::type, 0>::type arg0_t;
            typedef boost::mpl::at_c<boost::function_types::parameter_types<T>::type, 1>::type arg1_t;
            typedef boost::mpl::at_c<boost::function_types::parameter_types<T>::type, 2>::type arg2_t;
            typedef boost::mpl::at_c<boost::function_types::parameter_types<T>::type, 3>::type arg3_t;
            
            f_(arg_getter<arg0_t>::get(args, args_, 0), arg_getter<arg1_t>::get(args, args_, 1), 
               arg_getter<arg2_t>::get(args, args_, 2), arg_getter<arg3_t>::get(args, args_, 3));

            return std::auto_ptr<call_resolver_call_arg_base>();
         }

         virtual std::auto_ptr<call_resolver_call_arg_base> invoke(args_list_t& args)
         {
            return invoke(args, boost::mpl::int_<boost::function_types::function_arity<T>::value>());
         }
      
      private:
         boost::function<T> f_;
   };

   template<typename T>
   std::vector<call_resolver_arg_def> make_args()
   {
      return std::vector<call_resolver_arg_def>();
   }

   class call_resolver : public boost::noncopyable
	{
         typedef std::map<std::string, boost::shared_ptr<call_resolver_function_base> > functions_t;

      public:
         template<typename T>
         void insert(const std::string& func_name, boost::function<T> f)
         {
            functions_t::const_iterator i = functions_.find(func_name);
            if (i != functions_.end())
               throw std::runtime_error("Function already registered '" + func_name + "'");

            call_resolver_arg_def result_def(&typeid(typename boost::function_types::result_type<T>::type));
            std::vector<call_resolver_arg_def> call_args = make_args<T>();
            std::auto_ptr<call_resolver_function<T> > crf(new call_resolver_function<T>(f, call_args, result_def));

            functions_.insert(std::make_pair(func_name, boost::shared_ptr<call_resolver_function_base>(crf)));
         }

         std::auto_ptr<call_resolver_call_arg_base> invoke(const char* func_name, args_list_t& args);
      
      private:
         functions_t functions_;

	};
}