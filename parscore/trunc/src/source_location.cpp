#include "stdafx.h"
#include <hammer/parscore/source_location.h>
#include <antlr3commontoken.h>
#include <cassert>

namespace hammer{namespace parscore{

const char* source_location::begin() const
{
   assert(antlr_token_ != NULL);
   // FIXME: possible x64 malfunction
   return reinterpret_cast<const char*>(antlr_token_->start);
}

}}
