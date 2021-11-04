
#include <interface/pnet_logging.hpp>
#include <Singular/libsingular.h>
#include "singular_functions.hpp"

#include <boost/format.hpp>

#include <iostream>
#include <fstream>
#include <config.hpp>
#include <string>

#include <chrono>

namespace singular_parallel
{
  namespace interface
  {

    NO_NAME_MANGLING
      long get_current_time_milli()
      {
        return std::chrono::duration_cast
                  <std::chrono::milliseconds>
                  (std::chrono::system_clock::now()
                                .time_since_epoch())
                                .count();

      }

    NO_NAME_MANGLING
      std::string get_problem_time_path( const int& id
                                    , std::string measure_name
                                    , std::string tmpdir
                                    )
    {
      return tmpdir +
             "/" + measure_name +
             "_" + std::to_string(id) +
             ".time";
    }

    NO_NAME_MANGLING
      std::string get_term_time_path( const int& id
                                    , const int& term_id
                                    , std::string measure_name
                                    , std::string tmpdir
                                    )
    {
      return tmpdir +
             "/" + measure_name +
             "_" + std::to_string(id) +
             "_" + std::to_string(term_id) +
             ".time";
    }

    NO_NAME_MANGLING
      void write_current_time
      ( const std::string& path )
      {
        long t_now = get_current_time_milli();
        std::ofstream time_file;
        time_file.open(path, std::ofstream::trunc);
        time_file << t_now <<  "\n";
        time_file.close();
      }

    NO_NAME_MANGLING
      long read_written_time (const std::string& path)
      {
        std::ifstream time_file(path);
        std::string line;
        std::getline(time_file, line);
        long time_val = std::stol(line);
        time_file.close();

        return time_val;
      }

    NO_NAME_MANGLING
      long get_written_time (const std::string& path)
      {
        long time_val = read_written_time(path);

        remove(path.c_str());

        return time_val;
      }

    NO_NAME_MANGLING
      long get_duration_time
      ( const std::string& path )
      {
        long t_now = get_current_time_milli();
        long start_time = get_written_time(path);

        return t_now - start_time;
      }

    NO_NAME_MANGLING
      void write_duration_time
      ( const long& duration
      , const std::string& path )
      {
        std::ofstream time_file;
        time_file.open(path, std::ofstream::trunc);
        time_file << duration <<  "\n";
        time_file.close();
      }

    NO_NAME_MANGLING
      void log_duration
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& measure_name
      , const long& duration
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);
        boost::format command =
              boost::format("pfd_singular_log_duration(%1%, %2%, %3%, %4%);")
                            % id
                            % duration
                            % ("\"" + measure_name + "\"")
                            % ("\"" + options.tmpdir + "\"");
        singular::call_and_discard(command.str());
      }




    NO_NAME_MANGLING
      void init_logging_for_step(
                                 unsigned int const& id,
                                 const std::string& step,
                                 const pnet_options& options
                                )
      {
            write_duration_time ( 0, get_problem_time_path ( id
                                                           , "init_" + step
                                                           , options.tmpdir
                                                           ));
            write_duration_time ( 0, get_problem_time_path ( id
                                                           , "compute_" + step
                                                           , options.tmpdir
                                                           ));
            write_duration_time ( 0, get_problem_time_path ( id
                                                           , "merge_" + step
                                                           , options.tmpdir
                                                           ));
            write_duration_time ( 0, get_problem_time_path ( id
                                                           , "finish_" + step
                                                           , options.tmpdir
                                                           ));
      }
  }
}
