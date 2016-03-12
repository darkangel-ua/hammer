#if !defined(h_fb17bc2b_fe14_455d_9b16_21915650a1f7)
#define h_fb17bc2b_fe14_455d_9b16_21915650a1f7

#include <boost/noncopyable.hpp>
#include <hammer/core/build_node.h>
#include <hammer/core/location.h>

#include <map>
#include <set>

namespace hammer{

class main_target;
class meta_target;
class project;

namespace project_generators{

class qmake_pro : public boost::noncopyable
{
   public:
      qmake_pro(const project& source_project,
                const location_t& output_prefix);
      void add_target(boost::intrusive_ptr<const build_node> node);
      void write() const;

   private:
      typedef std::map<const meta_target*, const main_target*> m2m_t;
      typedef std::map<const project*, const meta_target*> p2m_t;
      
      const project& source_project_;
      const location_t output_prefix_;
      const meta_target* meta_target_;
      m2m_t m2m_;
      p2m_t p2m_;

      void gather_dependencies(const main_target* mt);
};

}}

#endif //h_fb17bc2b_fe14_455d_9b16_21915650a1f7
