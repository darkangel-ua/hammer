#pragma once
#include <vector>
#include <string>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature_registry.h>

namespace hammer {
   class project;
   class engine;
}

struct build_request {
   hammer::feature_set* build_request_;
   std::vector<std::string> target_ids_;
//   std::vector<const hammer::basic_meta_target*> targets_;
//   std::vector<std::string> unresolved_target_names_;
};

struct resolved_targets {
   std::vector<const hammer::basic_meta_target*> targets_;
   std::vector<std::string> unresolved_target_ids_;
};

// build request examples:
//
// 1 foo variant=debug
// 2 /boost/thread                              - build all founded boost/thread targets, there can be multiple versions of boost/thread
// 3 /boost/thread variant=debug version=1.56.0 - build only 1.56.0 version of boost/thread lib in debug variant
// 4 /boost/thread variant=debug,release        - build all founded boost/thread targets in both release and debug variants - not handled right now

build_request
resolve_build_request(hammer::engine& e,
                      const std::vector<std::string>& build_request_args,
                      const hammer::project* current_project);

resolved_targets
resolve_target_ids(hammer::engine& e,
                   const hammer::project* project,
                   const std::vector<std::string>& target_ids,
                   const hammer::feature_set& build_request);

std::ostream&
operator << (std::ostream& s,
             const build_request& build_request);

std::ostream&
operator << (std::ostream& s,
             const resolved_targets& resolved_targets);
