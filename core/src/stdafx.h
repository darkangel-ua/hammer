#pragma once

#if defined(_WIN32) && !defined(__MINGW32__)
#pragma warning(disable : 4996)
#endif

#define _WIN32_WINNT 0x0501

#if defined(HAMMER_ENABLE_PRECOMPILED_HEADER)

#include <boost/asio/io_service.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/guid.hpp>
#include <boost/unordered_map.hpp>

#endif
