#include <hammer/core/diagnostic.h>
#include <cassert>
#include <string>

using namespace std;

namespace hammer{ 

diagnostic::diagnostic(const std::string& source_name)
	: error_count_(0),
	  source_name_(source_name)
{}

diagnostic_builder diagnostic::error(parscore::source_location loc, 
                                     const char* message)
{
   ++error_count_;

   type_ = type::error;
   loc_ = loc;
   message_ = message;
   args_.clear();
   arg_types_.clear();

   return diagnostic_builder(this);
}

void diagnostic::format_location()
{
	stream_ << source_name_ << '(' << loc_.line() << ") : ";
}

void diagnostic::format_message()
{
   assert(message_);
   stream_.str(string());

   format_location();
   stream_ << "error: ";

   size_t arg_pos = 0;
   while(*message_ != 0)
   {
      switch(*message_)
      {
         case '%':
         {
            assert(arg_pos < args_.size() && "Too much argument");
            
            ++message_;
            assert(*message_ && "Missing argument type specifier");
            switch(*message_)
            {
               case 's':
               {
                  switch(arg_types_[arg_pos])
                  {
                     case arg_type::chars:
                     {
                        stream_ << static_cast<const char*>(args_[arg_pos]);
                        break;
                     }

                     case arg_type::identifier:
                     {
                        stream_ << *static_cast<const parscore::identifier*>(args_[arg_pos]);
                        break;
                     }

                     default:
                        assert(false && "Don't know how to map this argument to string");
                  }
                  break;
               }

               case 'd':
               {
                  assert(arg_types_[arg_pos] == arg_type::integer && 
                         "Format specifier doesn't match passed argument");
                  
                  stream_ << *static_cast<const int*>(args_[arg_pos]);

                  break;
               }
               
               default:
                  assert(false && "Unknown argument type specifier");
            }
            ++arg_pos;
            break;
         }

         default:
         {
            stream_ << *message_;
            break;
         }
      }
      
      ++message_;
   }

   report(stream_.str().c_str());
   message_ = NULL;
}

void streamed_diagnostic::report(const char* formated_message)
{
   os_ << formated_message;
}

}
