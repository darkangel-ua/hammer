#pragma once
#include <vector>
#include <string>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature_registry.h>

struct build_request {
   hammer::feature_set* build_request_;
   std::vector<std::string> target_names_;
   std::vector<const hammer::basic_meta_target*> targets_;
   std::vector<std::string> unresolved_target_names_;
};

// build request examples:
//
// 1 foo variant=debug
// 2 /boost/thread                              - build all founded boost/thread targets, there can be multiple versions of boost/thread
// 3 /boost/thread variant=debug version=1.56.0 - build only 1.56.0 version of boost/thread lib in debug variant
// 4 /boost/thread variant=debug,release        - build all founded boost/thread targets in both release and debug variants

build_request
resolve_build_request(hammer::engine& e,
                      const std::vector<std::string>& build_request_args,
                      const hammer::project* current_project);

std::ostream&
operator << (std::ostream& s,
             const build_request& build_request);
