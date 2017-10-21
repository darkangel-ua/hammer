#include <hammer/parscore/source_location.h>
#include <antlr3commontoken.h>
#include <antlr3input.h>
#include <cassert>

namespace hammer{namespace parscore{

std::string source_location::full_source_name() const
{
   assert(antlr_token_);

   // for imaginary tokens there will be nullptr
   if (antlr_token_->input) {
      return std::string(reinterpret_cast<const char*>(antlr_token_->input->fileName->chars),
                         reinterpret_cast<const char*>(antlr_token_->input->fileName->chars) + antlr_token_->input->fileName->len);
   } else
      return "unknown";
}

unsigned source_location::line() const
{
   assert(antlr_token_);

   return antlr_token_->line;
}

}}
