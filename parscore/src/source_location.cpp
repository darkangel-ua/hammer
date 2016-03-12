#include "stdafx.h"
#include <hammer/parscore/source_location.h>
#include <antlr3commontoken.h>
#include <antlr3input.h>
#include <cassert>

namespace hammer{namespace parscore{

const char* source_location::begin() const
{
   assert(antlr_token_);
   // FIXME: possible x64 malfunction
   return reinterpret_cast<const char*>(antlr_token_->start);
}

std::string source_location::full_source_name() const
{
   assert(antlr_token_);

   return std::string(reinterpret_cast<const char*>(antlr_token_->input->fileName->chars), 
                      reinterpret_cast<const char*>(antlr_token_->input->fileName->chars) + antlr_token_->input->fileName->len);
}

unsigned source_location::line() const
{
   assert(antlr_token_);

   return antlr_token_->line;
}

}}
