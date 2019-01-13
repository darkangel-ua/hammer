#pragma once
#include <vector>
#include <memory>

namespace hammer {
class feature_set;
class feature;
class feature_registry;
class project;

class requirement_base {
   public:
      requirement_base(const bool public_ = false) : public_(public_) {}
      virtual void eval(const feature_set& build_request,
                        feature_set* result,
                        feature_set* public_result) const = 0;
      virtual requirement_base* clone() const = 0;
      virtual void setup_path_data(const project* p) = 0;
      virtual ~requirement_base() {}
      void set_public(bool v) { public_ = v; }
      bool is_public() const { return public_; }

   private:
      bool public_;
};

class just_feature_requirement : public requirement_base {
   public:
      just_feature_requirement(feature* f) : f_(f) {}
      void eval(const feature_set& build_request,
                feature_set* result,
                feature_set* public_result) const override;
      requirement_base* clone() const override { return new just_feature_requirement(*this); }
      void setup_path_data(const project* p) override;

   private:
      feature* f_;
};

class linear_and_condition : public requirement_base {
   public:
      void add(feature* c);
      void result(feature *r) { result_ = r; }
      void eval(const feature_set& build_request,
                feature_set* result,
                feature_set* public_result) const override;
      requirement_base* clone() const override { return new linear_and_condition(*this); }
      void setup_path_data(const project* p) override;

   private:
      typedef std::vector<const feature*> features_t;
      feature* result_;
      features_t features_;
};

class requirement_condition_op_base {
   public:
      virtual bool eval(const feature_set& build_request,
                        feature_set* result) const = 0;
      virtual requirement_condition_op_base* clone() const = 0;
      virtual ~requirement_condition_op_base() {}
};

class requirement_condition_op_binary : public requirement_condition_op_base {
   protected:
      requirement_condition_op_binary(std::unique_ptr<requirement_condition_op_base> lhs,
                                      std::unique_ptr<requirement_condition_op_base> rhs)
         : lhs_(std::move(lhs)),
           rhs_(std::move(rhs))
      {}

      std::unique_ptr<requirement_condition_op_base> lhs_;
      std::unique_ptr<requirement_condition_op_base> rhs_;
};

class requirement_condition_op_and : public requirement_condition_op_binary {
   public:
      requirement_condition_op_and(std::unique_ptr<requirement_condition_op_base> lhs,
                                   std::unique_ptr<requirement_condition_op_base> rhs)
         : requirement_condition_op_binary(std::move(lhs), std::move(rhs))
      {}

      bool eval(const feature_set& build_request,
                feature_set* result) const override
      {
         return lhs_->eval(build_request, result) && rhs_->eval(build_request, result);
      }

      requirement_condition_op_base*
      clone() const override {
         return new requirement_condition_op_and(std::unique_ptr<requirement_condition_op_base>(lhs_->clone()),
                                                 std::unique_ptr<requirement_condition_op_base>(rhs_->clone()));
      }
};

class requirement_condition_op_or : public requirement_condition_op_binary {
   public:
      requirement_condition_op_or(std::unique_ptr<requirement_condition_op_base> lhs,
                                  std::unique_ptr<requirement_condition_op_base> rhs)
         : requirement_condition_op_binary(std::move(lhs), std::move(rhs))
      {}

      bool eval(const feature_set& build_request,
                feature_set* result) const override
      {
         return lhs_->eval(build_request, result) || rhs_->eval(build_request, result);
      }

      requirement_condition_op_base*
      clone() const override {
         return new requirement_condition_op_or(std::unique_ptr<requirement_condition_op_base>(lhs_->clone()),
                                                std::unique_ptr<requirement_condition_op_base>(rhs_->clone()));
      }
};

class requirement_condition_op_feature : public requirement_condition_op_base {
   public:
      requirement_condition_op_feature(const feature* f) : f_(f) {}
      bool eval(const feature_set& build_request,
                feature_set* result) const override;

      requirement_condition_op_base*
      clone() const override { return new requirement_condition_op_feature( f_ ); }

   private:
      const feature* f_;
};

class requirement_condition : public requirement_base {
   public:
      struct result_element {
         feature* f_;
         const bool public_;
      };
      typedef std::vector<result_element> result_t;

      requirement_condition(std::unique_ptr<requirement_condition_op_base> cond,
                            const result_t& result,
                            const bool public_);

      void eval(const feature_set& build_request,
                feature_set* result,
                feature_set* public_result) const override;
      requirement_base* clone() const override;
      void setup_path_data(const project* p) override;

   private:
      std::unique_ptr<requirement_condition_op_base> cond_;
      result_t result_;
};

class requirements_decl {
   public:
      requirements_decl();
      requirements_decl(const requirements_decl& rhs);
      requirements_decl& operator = (const requirements_decl& rhs);
      void add(std::auto_ptr<requirement_base> r);
      void add(const feature& f);
      void eval(const feature_set& build_request,
                feature_set* result,
                feature_set* public_result = NULL) const;
      void setup_path_data(const project* p);
      void insert_infront(const requirements_decl& v);
      void insert(const requirements_decl& v); // insert in the end
      virtual ~requirements_decl();

   private:
      struct impl_t;
      impl_t* impl_;
};

class usage_requirements_decl : public requirements_decl {};

}
