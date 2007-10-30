#include "stdafx.h"
#include "feature.h"

using namespace std;

namespace hammer{

   feature::feature(const feature_def* def, const pstring& value)
                   : 
                    def_(def), value_(value)
   {
   }
}