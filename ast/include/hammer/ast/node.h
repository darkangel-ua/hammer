#if !defined(h_dd3f4d38_7ae4_4bce_ae10_b72304b1cf83)
#define h_dd3f4d38_7ae4_4bce_ae10_b72304b1cf83

namespace hammer{namespace ast{

class visitor;

class node
{
   public:
      virtual bool accept(visitor& v) const = 0;
      virtual ~node() {}
};

}}

#endif
