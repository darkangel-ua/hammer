#if !defined(h_f7fe3cf9_e23b_4943_85bc_0046ca024c58)
#define h_f7fe3cf9_e23b_4943_85bc_0046ca024c58

#include <hammer/parscore/identifier.h>
#include <sstream>
#include <vector>

namespace hammer{ 

class diagnostic_builder;

class diagnostic
{
   public:
      friend class diagnostic_builder;
      
      struct type { enum value {error}; };
      struct arg_type { enum value{integer, chars, identifier}; };

      diagnostic();

      diagnostic_builder error(parscore::source_location loc, 
                               const char* message);
      int error_count() const { return error_count_; }

   protected:
      diagnostic::type::value type_;
      parscore::source_location loc_;
      const char* message_;
      std::vector<const void*> args_;
      std::vector<int> arg_types_;
      std::ostringstream stream_;
      int error_count_;

      virtual void report(const char* formated_message) = 0;
      virtual void format_message();
   
   private:
      void format_location();
};

class diagnostic_builder
{
   public:
      diagnostic_builder(diagnostic* d) : d_(d) {}
      ~diagnostic_builder() { d_->format_message(); }
   
      diagnostic_builder& operator << (const int& v)
      { 
         d_->args_.push_back(&v);
         d_->arg_types_.push_back(diagnostic::arg_type::integer);
         return *this;
      }

      diagnostic_builder& operator << (const parscore::identifier& v)
      { 
         d_->args_.push_back(&v);
         d_->arg_types_.push_back(diagnostic::arg_type::identifier);
         return *this;
      }

      diagnostic_builder& operator << (const char* v)
      { 
         d_->args_.push_back(v);
         d_->arg_types_.push_back(diagnostic::arg_type::chars);
         return *this;
      }

   private:
      diagnostic* d_;
};

class streamed_diagnostic : public diagnostic
{
   public:
      streamed_diagnostic(std::ostream& os) : os_(os) {}
   
   protected:
      virtual void report(const char* formated_message);

   private:
      std::ostream& os_;
};

}

#endif //h_f7fe3cf9_e23b_4943_85bc_0046ca024c58
