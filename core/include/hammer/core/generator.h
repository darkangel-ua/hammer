#if !defined(h_9b0699bd_cc04_4e19_874b_45c95b909551)
#define h_9b0699bd_cc04_4e19_874b_45c95b909551

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include "build_node.h"

namespace hammer
{
   class target_type;
   class feature;
	class feature_set;
   class basic_target;
   class pstring;
   class engine;
   class main_target;
   class type_tag;

   class generator
   {
      public:
         struct consumable_type
         {
            consumable_type(const target_type& t, unsigned int f = 0, const feature_set* fs = 0) : type_(&t), flags_(f), features_(fs) {}
            
            const target_type* type_;
            unsigned int flags_;
            const feature_set* features_;
         };

         struct produced_type
         {
            produced_type(const target_type& t, bool need_tag = false) : type_(&t), need_tag_(need_tag) {}

            const target_type* type_;
            bool need_tag_;
         };

         typedef std::vector<consumable_type> consumable_types_t;
         typedef std::vector<produced_type> producable_types_t;
         
         generator(engine& e,
                   const std::string& name,
                   const consumable_types_t& source_types,
                   const producable_types_t& target_types,
                   bool composite,
			          const build_action_ptr& action,
                   const feature_set* c = nullptr);
         virtual ~generator();

         hammer::engine& get_engine() { return *engine_; }
         hammer::engine& get_engine() const { return *engine_; }
         const std::string& name() const { return name_; }
         const consumable_types_t& consumable_types() const { return source_types_; }
         const producable_types_t& producable_types() const { return target_types_; }
         const feature_set* constraints() const { return constraints_; }
 
         virtual build_nodes_t
         construct(const target_type& type_to_construct, 
                   const feature_set& props,
                   const build_nodes_t& sources,
                   const basic_build_target* source_target,
                   const std::string* composite_target_name,
                   const main_target& owner) const;

         bool is_consumable(const target_type& t) const;
         bool is_composite() const { return composite_; }
         bool include_composite_generators() const { return include_composite_generators_; } 
         void include_composite_generators(bool v) { include_composite_generators_ = v; }

         const build_action_ptr& action() const { return action_; }

      protected:
         /// composite_target_name will be presented only for composite targets
			virtual
			basic_build_target*
			create_target(const main_target* mt,
			              const build_node::sources_t& sources,
			              const std::string* composite_target_name,
			              const produced_type& type,
			              const feature_set* f) const;
			static
			std::pair<std::string /*name*/, std::string /*hash*/>
			make_product_name_and_hash(const build_node::sources_t& sources,
			                           const std::string* composite_target_name,
							               const produced_type& product_type,
							               const feature_set& product_properties);

      private:
         hammer::engine* engine_;
         const std::string name_;
         consumable_types_t source_types_;
         producable_types_t target_types_;
         bool composite_;
         const feature_set* constraints_; // == null if no constraints specified
         build_action_ptr action_;
         bool include_composite_generators_; // include composite generators while searching for sources indirect transformations

		protected:
			const std::vector<const feature*> action_valuable_features_;
			const std::vector<const feature*> constraints_valuable_features_;
			const std::vector<const feature*> generator_valuable_features_;

			feature_set*
			make_valuable_properties(const feature_set& target_props,
			                         const std::vector<const feature*>& target_type_valuable_features,
			                         const std::vector<const feature*>& source_target_valuable_features) const;
   };

   // we must not change order in nodes, because if that we use multi_index
   // because of gcc library ordering we leave only last ocurrancies of some library.
   // if input is A B C B D then output will be A C B D
   void remove_dups(build_node::nodes_t& nodes);

   generator::producable_types_t
   make_product_types(engine& e,
                      const std::vector<type_tag>& types);
   generator::consumable_types_t
   make_consume_types(engine& e,
                      const std::vector<type_tag>& types);
}

#endif //h_9b0699bd_cc04_4e19_874b_45c95b909551
