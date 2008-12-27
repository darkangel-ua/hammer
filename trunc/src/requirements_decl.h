#if !defined(h_bd7a680f_5464_4a47_b274_185a105a6da0)
#define h_bd7a680f_5464_4a47_b274_185a105a6da0

namespace hammer
{
   class feature_set;
   class feature;
   class feature_registry;
   class basic_meta_target;

   class requirement_base
   {
      public:
         requirement_base() : public_(false) {}

         virtual void eval(feature_set* result, 
                           feature_registry& fr) const = 0; // FIX: война с const-ами разразилась не на шутку :(
         virtual void eval(const feature_set& build_request,
                           feature_set* result,
                           feature_set* public_result) const = 0;
         virtual requirement_base* clone() const = 0;
         virtual void setup_path_data(const basic_meta_target* t) = 0;
         virtual ~requirement_base() {}
         void set_public(bool v) { public_ = v; } 
         bool is_public() const { return public_; }

      private:
         bool public_;
   };
   
   class just_feature_requirement : public requirement_base
   {
      public:
         just_feature_requirement(feature* f) : f_(f) {}
         virtual void eval(feature_set* result, 
                           feature_registry& fr) const;
         virtual void eval(const feature_set& build_request,
                           feature_set* result,
                           feature_set* public_result) const;
         virtual requirement_base* clone() const { return new just_feature_requirement(*this); }
         virtual void setup_path_data(const basic_meta_target* t);
      
      private:
         feature* f_;
   };

   class linear_and_condition : public requirement_base
   {
      public:
         void add(feature* c);
         void result(feature *r) { result_ = r; }
         virtual void eval(feature_set* result, 
                           feature_registry& fr) const;
         virtual void eval(const feature_set& build_request,
                           feature_set* result,
                           feature_set* public_result) const;
         virtual requirement_base* clone() const { return new linear_and_condition(*this); }
         virtual void setup_path_data(const basic_meta_target* t);
      
      private:
         typedef std::vector<const feature*> features_t;
         feature* result_;
         features_t features_;
   };

   class requirements_decl
   {
      public:
         requirements_decl();
         requirements_decl(const requirements_decl& rhs);
         requirements_decl& operator = (const requirements_decl& rhs);
         void add(std::auto_ptr<requirement_base> r);
         void add(const feature& f);
//          void eval(feature_set* result, 
//                    feature_registry& fr) const;
         void eval(const feature_set& build_request, 
                   feature_set* result,
                   feature_set* public_result = NULL) const;
         void setup_path_data(const basic_meta_target* t);
         void insert_infront(const requirements_decl& v);
         void insert(const requirements_decl& v); // insert in the end
         ~requirements_decl();

      private:
         struct impl_t;
         impl_t* impl_;
};
}

#endif //h_bd7a680f_5464_4a47_b274_185a105a6da0
