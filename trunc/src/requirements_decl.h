#pragma once

namespace hammer
{
   class feature_set;
   class feature;
   class feature_registry;
   class basic_meta_target;

   class requirement_base
   {
      public:
         virtual void eval(feature_set* result, 
                           feature_registry& fr) const = 0; // FIX: война с const-ами разразилась не на шутку :(
         virtual void eval(const feature_set& build_request,
                           feature_set* result) const = 0;
         virtual requirement_base* clone() const = 0;
         virtual void setup_path_data(const basic_meta_target* t) = 0;
         virtual ~requirement_base() {}
   };
   
   class just_feature_requirement : public requirement_base
   {
      public:
         just_feature_requirement(feature* f) : f_(f) {}
         virtual void eval(feature_set* result, 
                           feature_registry& fr) const;
         virtual void eval(const feature_set& build_request,
                           feature_set* result) const;
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
                           feature_set* result) const;
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
         void eval(feature_set* result, 
                   feature_registry& fr) const;
         void eval(const feature_set& build_request, 
                   feature_set* result) const;
         void setup_path_data(const basic_meta_target* t);
         ~requirements_decl();

      private:
         struct impl_t;
         impl_t* impl_;
};
}