#if !defined(h_392947bd_0bce_4d6c_9796_24d72580e72e)
#define h_392947bd_0bce_4d6c_9796_24d72580e72e

#include <vector>

namespace hammer{ namespace parscore{

class identifier;
}}

namespace hammer{ namespace ast{

class expression;
class statement;
class feature;

typedef std::vector<const expression*> expressions_t;
typedef std::vector<parscore::identifier> identifiers_t;
typedef std::vector<const statement*> statements_t;
typedef std::vector<const feature*> features_t;
   
}}

#endif //h_392947bd_0bce_4d6c_9796_24d72580e72e
