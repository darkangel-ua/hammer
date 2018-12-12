#pragma once
#include <sstream>
#include <vector>
#include <hammer/parscore/identifier.h>

namespace hammer {

class diagnostic_builder;

class diagnostic {
   public:
      friend class diagnostic_builder;
      
      struct type { enum value {error}; };
      struct arg_type { enum value{integer, chars, identifier, std_string}; };

		diagnostic(const std::string& source_name,
                 bool verbose);

      diagnostic_builder error(parscore::source_location loc, 
                               const char* message);
      int error_count() const { return error_count_; }

   public:
      const bool verbose_;

   protected:
      diagnostic::type::value type_;
      parscore::source_location loc_;
      const char* message_;
      std::vector<const void*> args_;
      std::vector<int> arg_types_;
      std::ostringstream stream_;
      int error_count_ = 0;

      virtual void report(const char* formated_message) = 0;
   
   private:
      void format_location();
      void format_message();
      void add_source_snippet();

		const std::string source_name_;
};

class diagnostic_builder {
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

      diagnostic_builder&
      operator << (const std::string& v)
      {
         d_->args_.push_back(&v);
         d_->arg_types_.push_back(diagnostic::arg_type::std_string);
         return *this;
      }

   private:
      diagnostic* d_;
};

class streamed_diagnostic : public diagnostic {
   public:
		streamed_diagnostic(const std::string& source_name,
                          bool verbose,
								  std::ostream& os)
			: diagnostic(source_name, verbose),
			  os_(os)
		{}
   
   protected:
      void report(const char* formated_message) override;

   private:
      std::ostream& os_;
};

}
