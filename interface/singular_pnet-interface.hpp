#pragma once

#define NO_NAME_MANGLING extern "C"

#include <string>
#include <interface/type_aliases.hpp>
#include <config.hpp>

#define RESOLVE_INTERFACE_FUNCTION(function) \
      (fhg::util::scoped_dlhandle \
      (config::parallelInterfaceLibrary(), \
      RTLD_GLOBAL | RTLD_NOW | RTLD_DEEPBIND) \
      .sym<decltype(::singular_parallel::interface::function)> \
      (BOOST_PP_STRINGIZE(function)))


namespace singular_parallel
{
  namespace interface
  {
    NO_NAME_MANGLING
      void singular_parallel_compute
       ( unsigned int const& id
       , const pnet_options& options
       );

    NO_NAME_MANGLING
      void pfd_parallel_compute
       ( unsigned int const& id
       , const pnet_options& options
       );
  }
}
