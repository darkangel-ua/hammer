#include "stdafx.h"
#include "feature.h"

using namespace std;

namespace hammer{

   feature::feature(const string& name, const string& value, 
                    feature_type::value t)
                   : 
                    name_(name), value_(value), type_(t)
   {

   }
}