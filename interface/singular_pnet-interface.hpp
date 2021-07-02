#pragma once

#define NO_NAME_MANGLING extern "C"

#include <string>
#include <interface/type_aliases.hpp>

NO_NAME_MANGLING
void singular_parallel_compute ( std::string const&
                               , std::string const&
                               , unsigned int const&
                               , std::string const&
                               , std::string const&
                               , std::string const&
                               , std::string const&
                               , std::string const&
                               , std::string const&
                               );

NO_NAME_MANGLING
singular_parallel::pnet_list sp_extract_neighbours
                               ( std::string const& path_to_libsingular
                               , std::string const& base_filename
                               , std::string const& in_struct_name
                               , std::string const& in_struct_desc
                               , std::string const& needed_library
                               , std::string const& expand_function
                               , unsigned long const& id
                               , unsigned int const& written_count);

NO_NAME_MANGLING
singular_parallel::pnet_list sp_graph_extract_neighbours
                               ( std::string const& path_to_libsingular
                               , std::string const& base_filename
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
