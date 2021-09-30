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
      void singular_parallel_compute
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
      );

    NO_NAME_MANGLING
      long get_current_time_milli();

    NO_NAME_MANGLING
      std::string get_term_time_path( const int& id
                                    , const int& term_id
                                    , std::string measure_name
                                    , std::string tmpdir
                                    );
    NO_NAME_MANGLING
      std::string get_problem_time_path( const int& id
                                    , std::string measure_name
                                    , std::string tmpdir
                                    );
    

    NO_NAME_MANGLING
      void write_current_time
      ( const std::string& path );
      
    NO_NAME_MANGLING
      long get_written_time (const std::string& path);

    NO_NAME_MANGLING
      long get_duration_time
      ( const std::string& path );

    NO_NAME_MANGLING
      void write_duration_time
      ( const long& duration
      , const std::string& path 
      );

    NO_NAME_MANGLING
      void log_duration
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& measure_name
      , const long& duration
      );

    NO_NAME_MANGLING
      int pfd_fork_compare_tdegrees
      ( singular_parallel::pnet_list left
      , singular_parallel::pnet_list right
      );

    NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_fork_get_tdegrees_before
      ( unsigned int const& id
      , const std::string& step
      , const pnet_options& options
      );

    NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_fork_get_tdegrees_after
      ( unsigned int const& id
      , const std::string& step
      , const pnet_options& options
      );

    NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_fork_get_tdegrees_file
      ( unsigned int const& id
      , const std::string& file
      , const pnet_options& options
      );
  }
}
