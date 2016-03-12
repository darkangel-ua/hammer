#if !defined(h_01e69c62_3501_486a_b7d0_faccef6a4390)
#define h_01e69c62_3501_486a_b7d0_faccef6a4390

#include <hammer/core/build_node.h>
#include <hammer/core/location.h>
#include <boost/noncopyable.hpp>

namespace boost
{
   class guid; 
}

namespace hammer
{
   class basic_target;
   class engine;
   class project;

   namespace project_generators
   {
      class msvc_solution : public boost::noncopyable
      {
         public:
            struct impl_t;
            struct generation_mode
            {
               enum value {NON_LOCAL, LOCAL};
            };

            msvc_solution(const project& source_project, const location_t& output_path, 
                          generation_mode::value mode = generation_mode::NON_LOCAL);
            void add_target(boost::intrusive_ptr<const build_node> node);
            void write() const;
            ~msvc_solution();
         
         protected:
            // used for testing now
            virtual boost::guid generate_id() const;

         private:
            impl_t* impl_;
      };
   }
}

#endif //h_01e69c62_3501_486a_b7d0_faccef6a4390
