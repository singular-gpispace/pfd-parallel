#ifndef PNET_LOGGING_HPP
#define PNET_LOGGING_HPP

#define NO_NAME_MANGLING extern "C"

#include <string>
#include <interface/type_aliases.hpp>
#include <config.hpp>

namespace singular_parallel
{
  namespace interface
  {

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
      void init_logging_for_step
      ( unsigned int const& id
      , const std::string& step
      , const pnet_options& options
      );
  }
}

#endif /* ifndef pnet_logging_hpp */
