#if !defined(h_31be843d_c21d_4b96_a9aa_89728389d553)
#define h_31be843d_c21d_4b96_a9aa_89728389d553

#include <hammer/core/generator.h>

namespace hammer{

class fake_generator : public generator
{
   public:
      fake_generator(engine& e,
                     const std::string& name,
                     const consumable_types_t& source_types,
                     const producable_types_t& target_types,
                     bool composite,
                     const feature_set* c = 0);
   protected:
      virtual basic_target* create_target(const main_target* mt, 
                                          const build_node::sources_t& sources,
                                          const std::string& n,
                                          const target_type* t, 
                                          const feature_set* f) const;
      
      build_nodes_t
      construct(const target_type& type_to_construct, 
                const feature_set& props,
                const std::vector<boost::intrusive_ptr<build_node> >& sources,
                const basic_target* source_target,
                const std::string* composite_target_name,
                const main_target& owner) const override;
};

}

#endif //h_31be843d_c21d_4b96_a9aa_89728389d553
