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
          ( std::string const& base_filename
          , unsigned int const& id
          , std::string const& function_name
          , std::string const& needed_library
          , std::string const& in_struct_name
          , std::string const& in_struct_desc
          , std::string const& out_struct_name
          , std::string const& out_struct_desc
          );

    NO_NAME_MANGLING
      void pfd_parallel_compute
      (
       std::string const& base_filename
       , unsigned int const& id
       , const pnet_options& options
       /*
       , std::string const& function_name
       , std::string const& needed_library
       , std::string const& in_struct_name
       , std::string const& in_struct_desc
       , std::string const& out_struct_name
       , std::string const& out_struct_desc
       */
       );

    NO_NAME_MANGLING
      singular_parallel::pnet_list sp_extract_neighbours
      ( std::string const& base_filename
        , std::string const& in_struct_name
        , std::string const& in_struct_desc
        , std::string const& needed_library
        , std::string const& expand_function
        , unsigned long const& id
        , unsigned int const& written_count);

    NO_NAME_MANGLING
      singular_parallel::pnet_list sp_graph_extract_neighbours
      ( std::string const& base_filename
        , std::string const& in_struct_name
        , std::string const& in_struct_desc
        , std::string const& needed_library
        , std::string const& expand_function
        , unsigned long const& id
        , unsigned int const& written_count
        , singular_parallel::pnet_set &hash_set // not constant!!
      );

    NO_NAME_MANGLING
      singular_parallel::pnet_set create_initial_hashset(singular_parallel::pnet_list init_list
          , std::string base_filename);
  }
}
