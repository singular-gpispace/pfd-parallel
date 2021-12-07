#pragma once

#define NO_NAME_MANGLING extern "C"

#include <string>
#include <interface/type_aliases.hpp>
#include <interface/pnet_logging.hpp>
#include <config.hpp>

#define RESOLVE_INTERFACE_FUNCTION(function) \
      (fhg::util::scoped_dlhandle \
      (config::parallelInterfaceLibrary(), \
      RTLD_GLOBAL | RTLD_NOW | RTLD_DEEPBIND) \
      .sym<decltype(::singular_parallel::interface::function)> \
      (BOOST_PP_STRINGIZE(function)))


#define NULLSTELL "NSSdecompStep"
#define SHORT_NUM  "shortNumeratorDecompStep"
#define ALG_DEPEND  "algDependDecompStep"
#define NUMERATOR  "numeratorDecompStep"

namespace singular_parallel
{
  namespace interface
  {

    NO_NAME_MANGLING
      std::string get_to_name_cpp
       ( const std::string& step_name
       );

    NO_NAME_MANGLING
      std::string get_from_name_cpp
       ( const std::string& step_name
       );

    NO_NAME_MANGLING
      long get_filesize
      ( std::string path );

    NO_NAME_MANGLING
      long get_input_file_size
      ( unsigned int id
      , const pnet_options& options
      , const std::string net_type
      );

    NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_sorted_input_by_size
      ( unsigned int count
      , const pnet_options& options
      , const std::string net_type
      );

    NO_NAME_MANGLING
      void singular_parallel_compute
       ( unsigned int const& id
       , const pnet_options& options
       );

    NO_NAME_MANGLING
      void pfd_serial_compute_pfd
       ( unsigned int const& id
       , const pnet_options& options
       );

    NO_NAME_MANGLING
      unsigned int pfd_general_prepare
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& first_step
      );

    NO_NAME_MANGLING
      unsigned int pfd_already_done
       ( unsigned int const& id
       , const pnet_options& options
       );

    NO_NAME_MANGLING
      void pfd_prepare_single_entry_fraction
      ( unsigned int const& id
      , const pnet_options& options
      );


    NO_NAME_MANGLING
      unsigned int pfd_is_trivial
      ( unsigned int const& id
      , const pnet_options& options
      );

    NO_NAME_MANGLING
      void pfd_prepare_input
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& first_step
      );

    NO_NAME_MANGLING
      void pfd_compute_step
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      );

    NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_loop_init
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      );

    NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_fork_init
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      );

    NO_NAME_MANGLING
      void pfd_hand_back
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      );

    NO_NAME_MANGLING
      void pfd_hand_forward
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      );

    NO_NAME_MANGLING
      void pfd_loop_compute_term
      ( const unsigned int& id
      , const unsigned int& term_id
      , const pnet_options& options
      , const std::string step
      );

    NO_NAME_MANGLING
      void pfd_fork_compute_term
      ( const unsigned int& id
      , const unsigned int& term_id
      , const pnet_options& options
      , const std::string step
      );

    NO_NAME_MANGLING
      int pfd_loop_merge
      ( unsigned int const& id
      , unsigned int const& term_count
      , const pnet_options& options
      , const std::string& step
      );

    NO_NAME_MANGLING
      unsigned int pfd_fork_merge_pair
      ( unsigned int const& id
      , unsigned int const& left
      , unsigned int const& right
      , const pnet_options& options
      , const std::string& step
      );


    NO_NAME_MANGLING
      void pfd_fork_merge
      ( unsigned int const& id
      , unsigned int const& term_count
      , const pnet_options& options
      , const std::string& step
      );

    NO_NAME_MANGLING
      singular_parallel::pnet_list  pfd_loop_cycle_terms
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      );

    NO_NAME_MANGLING
      void  pfd_loop_finish
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      );

    NO_NAME_MANGLING
      unsigned int pfd_fork_finish
      ( unsigned int const& id
      , unsigned int const& term_id
      , const pnet_options& options
      , const std::string& step
      );

    NO_NAME_MANGLING
      void pfd_write_result
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string last_step
      );
  }
}
