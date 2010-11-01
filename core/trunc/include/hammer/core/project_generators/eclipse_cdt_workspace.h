#if !defined(h_ddfdf439_6224_460b_9179_df25823f8cfb)
#define h_ddfdf439_6224_460b_9179_df25823f8cfb

#include <boost/noncopyable.hpp>
#include <memory>
#include <hammer/core/build_node.h>
#include <hammer/core/location.h>

namespace hammer{ 
   class project;
}

namespace hammer{ namespace project_generators{

class eclipse_cdt_workspace : public boost::noncopyable
{
   public:
      struct impl_t;

      eclipse_cdt_workspace(const location_t& output_path,
                            const build_nodes_t& nodes,
                            const location_t& templates_dir,
                            const hammer::project& master_project);
      void construct();
      void write() const;
      const location_t& get_output_path() const;
      const location_t& get_templates_dir() const;
      ~eclipse_cdt_workspace();

   private:
      std::auto_ptr<impl_t> impl_;
};

}}

#endif //h_ddfdf439_6224_460b_9179_df25823f8cfb
