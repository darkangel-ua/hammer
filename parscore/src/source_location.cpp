#include <cassert>
#include <antlr3commontoken.h>
#include <antlr3input.h>
#include <hammer/parscore/source_location.h>

namespace hammer { namespace parscore {

std::string
source_location::full_source_name() const
{
   assert(antlr_token_);

   // for imaginary tokens there will be nullptr
   if (antlr_token_->input) {
      return std::string(reinterpret_cast<const char*>(antlr_token_->input->fileName->chars),
                         reinterpret_cast<const char*>(antlr_token_->input->fileName->chars) + antlr_token_->input->fileName->len);
   } else
      return "unknown";
}

unsigned
source_location::line() const
{
   assert(antlr_token_);

   return antlr_token_->line;
}

unsigned
source_location::char_pos() const
{
   assert(antlr_token_);
   return antlr_token_->charPosition + 1 + (antlr_token_->line == 1);
}

std::string
source_location::line_content() const
{
   std::string result;

   if (antlr_token_->type == ANTLR3_TOKEN_EOF)
      return "EOF";

   if (!antlr_token_->lineStart)
      return "Internal error: lineStart == nullptr";

   const char* chars = static_cast<const char*>(antlr_token_->lineStart);

   while (*chars && *chars != '\n') {
      result.push_back(*chars);
      ++chars;
   }

   return result;
}

}}
