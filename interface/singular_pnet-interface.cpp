#include <Singular/libsingular.h>

#include <interface/singular_pnet-interface.hpp>
#include <interface/pnet_logging.hpp>

#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "singular_functions.hpp"
#include <config.hpp>
#include <string>
#include <pnetc/type/term/op.hpp>

namespace singular_parallel
{
  namespace interface
  {

    /*** local function declarations ***/

    std::string get_id_string();
    BOOLEAN safely_register_sing_struct(std::string const& reg_struct_name,
        std::string const& reg_struct_desc,
        std::string const& ids);
    BOOLEAN check_integers_equal(int const& a,
        int const& b,
        std::string const& err_message);

    NO_NAME_MANGLING
      void pfd_fork_compute_term_
      ( const unsigned int& id
      , const unsigned int& term_id
      , const pnet_options& options
      , const std::string step
      , const std::string from_file
      , const std::string to_file
      );

    /*** local function impementations ***/

    std::string get_id_string()
    {
      char hstn[65];
      gethostname (hstn, 64);
      hstn[64] = '\0';
      std::string ids (hstn);
      ids = ids + " " + std::to_string (getpid());

      return ids;
    }

    BOOLEAN safely_register_sing_struct(std::string const& reg_struct_name,
        std::string const& reg_struct_desc,
        std::string const& ids)
    {
      if (!register_struct (reg_struct_name, reg_struct_desc)) {
        throw std::runtime_error(ids +
            ": singular_parallel_compute: could not register " +
            reg_struct_name +
            " with description " + reg_struct_desc);
      }

      return TRUE;
    }

    BOOLEAN check_integers_equal(int const& a,
        int const& b,
        std::string const& err_message)
    {
      if (a != b) {
        throw std::runtime_error ("( " + err_message + ") " +
            std::to_string(a) +
            " is not equal to " +
            std::to_string(b));
      }
      return TRUE;
    }

    template <typename T>
      std::string get_from_name
      ( const T& step_name
      )
      {
        std::string step(step_name);
        if (step == NULLSTELL) {
          return "terms";
        } else if (step == SHORT_NUM) {
          return "terms_1";
        } else if (step == ALG_DEPEND) {
          return "terms_2";
        } else if (step == NUMERATOR) {
          return "terms_3";
        } else {
          throw std::runtime_error ("Invalid step name: " + step);
          return "invalid_step";
        }
      }

    template <typename T>
      std::string get_to_name
      ( const T& step_name
      )
      {
        std::string step(step_name);
        if (step == NULLSTELL) {
          return "terms_1";
        } else if (step == SHORT_NUM) {
          return "terms_2";
        } else if (step == ALG_DEPEND) {
          return "terms_3";
        } else if (step == NUMERATOR) {
          return "terms_4";
        } else {
          throw std::runtime_error ("Invalid step name: " + step);
          return "invalid_step";
        }
      }

    /*** Interface functions (declared in interface/singular_pnet-interface.hpp) ***/


    NO_NAME_MANGLING
      std::string get_from_name_cpp
       ( const std::string& step_name
       )
       {
         return get_from_name(step_name);
       }

    NO_NAME_MANGLING
      std::string get_to_name_cpp
       ( const std::string& step_name
       )
       {
         return get_to_name(step_name);
       }



    NO_NAME_MANGLING
      void singular_parallel_compute
      ( unsigned int const& id
      , const pnet_options& options
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);
        singular::load_ssi("input", get_in_struct_filename( options.tmpdir
                                        , config::parallel_list_base_name()
                                        , id));
        singular::call_and_discard(options.out_struct_name + " output = " +
                                    options.function_name +
                                    "(input);");
        singular::write_ssi("output", get_out_struct_filename( options.tmpdir
                                            , config::parallel_list_base_name()
                                            , id)
                                            );
        singular::call_and_discard("kill output;");
      }

    NO_NAME_MANGLING
      unsigned int pfd_already_done
      (  unsigned int const& id
      , const singular_parallel::pnet_options& options
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);

        singular::load_library (options.needed_library);
        singular::load_ssi("input", get_in_struct_filename( options.tmpdir
                                        , config::parallel_pfd_base_name()
                                        , id));
        singular::call_and_discard(std::string("def already_done = ") +
                                    "pfd_singular_is_already_computed" +
                                    "( input );");

        unsigned int done_already = singular::getInt("already_done");
        singular::call_and_discard("kill already_done;");
        if (done_already) {
          singular::call_and_discard( options.out_struct_name +
                                      " internal_temp_o;" +
                                      " internal_temp_o.result = \"Already done!\";");
          singular::write_ssi( "internal_temp_o"
                             , get_out_struct_filename( options.tmpdir
                                              , config::parallel_pfd_base_name()
                                              , id)
                             );
          singular::call_and_discard("kill internal_temp_o;");
        }

        return done_already;
      }


    NO_NAME_MANGLING
      void pfd_prepare_single_entry_fraction
      ( unsigned int const& id
      , const pnet_options& options
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);
        singular::load_ssi("input", get_in_struct_filename( options.tmpdir
                                        , config::parallel_pfd_base_name()
                                        , id));
        singular::call_and_discard( std::string("pfd_singular_prepareSingleEntryFraction")
                                    + "( input );");
        singular::call_and_discard("kill input;");
      }

    NO_NAME_MANGLING
      unsigned int pfd_is_trivial
      ( unsigned int const& id
      , const pnet_options& options
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);
        singular::load_ssi("input", get_in_struct_filename( options.tmpdir
                                        , config::parallel_pfd_base_name()
                                        , id));
        singular::call_and_discard("def output = "
                                   "pfd_singular_is_trivial"
                                   "(input);");
        singular::call_and_discard("kill input;");
        singular::call_and_discard("if (typeof(output) == \"out_struct\")"
                                   " { int trivial_pfd = 1; }"
                                   " else { int trivial_pfd = 0; }");
        if (singular::getInt("trivial_pfd")) {
          singular::write_ssi("output", get_out_struct_filename( options.tmpdir
                                            , config::parallel_pfd_base_name()
                                            , id)
                                            );
          singular::call_and_discard("kill output;");
          singular::call_and_discard("kill trivial_pfd;");
          return 1;
        } else {
          singular::call_and_discard("kill output;");
          singular::call_and_discard("kill trivial_pfd;");
          return 0;
        }

      }

    NO_NAME_MANGLING
      void pfd_prepare_input
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& first_step
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);
        singular::load_ssi("input", get_in_struct_filename( options.tmpdir
                                        , config::parallel_pfd_base_name()
                                        , id));
        boost::format command =
              boost::format("pfd_singular_prepare_input(%1%, %2%, %3%, %4%);")
                            % "input"
                            % id
                            % ("\"" + options.tmpdir + "\"")
                            % ("\"" + get_from_name(first_step) + "\"");

        singular::call_and_discard(command.str());

        singular::call_and_discard("kill input;");
      }

    NO_NAME_MANGLING
      unsigned int pfd_general_prepare
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& first_step
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);
        singular::load_ssi("input", get_in_struct_filename( options.tmpdir
                                        , config::parallel_pfd_base_name()
                                        , id));
        boost::format command =
              boost::format("int prepstat = pfd_singular_general_prepare(%1%, %2%, %3%, %4%);")
                            % "input"
                            % id
                            % ("\"" + options.tmpdir + "\"")
                            % ("\"" + get_from_name(first_step) + "\"");

        singular::call_and_discard(command.str());
        unsigned int prepstat = singular::getInt("prepstat");
        singular::call_and_discard("kill prepstat;");

        if (prepstat) {
          singular::call_and_discard( options.out_struct_name +
                                      " internal_temp_o;");
          if (prepstat == 1) {
            singular::call_and_discard(" internal_temp_o.result = \"Already done!\";");
          } else { if (prepstat == 2) {
            singular::call_and_discard(" internal_temp_o.result = \"Trivially done!\";");
          } else {
            throw("Unrecognised return value from general prepare");
          }}
          singular::write_ssi( "internal_temp_o"
                             , get_out_struct_filename( options.tmpdir
                                              , config::parallel_pfd_base_name()
                                              , id)
                             );
          singular::call_and_discard("kill internal_temp_o;");
        }
        singular::call_and_discard("kill input;");

        return prepstat;
      }



    NO_NAME_MANGLING
      void pfd_compute_step
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);
        boost::format command =
              boost::format("pfd_singular_compute_step(%1%, %2%, %3%, %4%, %5%);")
                            % id
                            % ("\"" + step + "\"")
                            % ("\"" + get_from_name(step) + "\"")
                            % ("\"" + get_to_name(step) + "\"")
                            % ("\"" + options.tmpdir + "\"");

        singular::call_and_discard(command.str());

        remove((options.tmpdir + "/" + get_from_name(step) +"_"
                                     + std::to_string(id)
                                     + ".ssi").c_str()
              );
      }

    NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_split_terms
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& in_file
      , const std::string& out_file
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);

        boost::format command =
              boost::format("int count = pfd_split_terms(%1%, %2%, %3%, %4%, %5%);")
                            % id
                            % ("\"" + in_file + "\"")
                            % ("\"" + out_file + "\"")
                            % ("\"" + options.tmpdir + "\"")
                            % options.split_max;

        singular::call_and_discard(command.str());



        remove((options.tmpdir + "/" + in_file + "_" + std::to_string(id) + ".ssi").c_str());
        int term_count = singular::getInt("count");
        singular::call_and_discard( "kill count;");

        singular_parallel::pnet_list indices;
        int i;
        for (i = 1; i <= term_count; i++) {
          singular_parallel::pnet_term term;
          term.id = id;
          term.term_id = i;
          indices.push_back(pnetc::type::term::to_value(term));
        }
        return indices;
      }


    NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_fork_init
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      )
      {
        std::string time_path(
            get_problem_time_path( id
                                 , "init_" + step
                                 , options.tmpdir
                                 ));
        write_current_time(time_path);

        std::string file(get_from_name(step));

        singular_parallel::pnet_list indices(pfd_split_terms(id, options, file, file));

        long duration(get_duration_time (time_path));
        write_duration_time(duration, time_path);

        return indices;
      }

    NO_NAME_MANGLING
      void pfd_fork_compute_term
      ( const unsigned int& id
      , const unsigned int& term_id
      , const pnet_options& options
      , const std::string step
      )
    {
        std::string time_path(get_term_time_path( id
                                                , term_id
                                                , "fork_compute_" + step
                                                , options.tmpdir));
        write_current_time(time_path);

        std::string file(get_from_name(step));
        pfd_fork_compute_term_( id
                             , term_id
                             , options
                             , step
                             , file
                             , file + "_result"
                             );
        remove((options.tmpdir + "/" + file + "_" + std::to_string(id) +
                        "_" + std::to_string(term_id) + ".ssi").c_str());

        long duration( get_duration_time(time_path) );
        write_duration_time ( duration , time_path);

    }


    NO_NAME_MANGLING
      void pfd_fork_compute_term_
      ( const unsigned int& id
      , const unsigned int& term_id
      , const pnet_options& options
      , const std::string step
      , const std::string from_file
      , const std::string to_file
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);
        boost::format command =
              boost::format("pfd_fork_compute_term(%1%, %2%, %3%, %4%, %5%, %6%);")
                            % id
                            % term_id
                            % ("\"" + step + "\"")
                            % ("\"" + from_file + "\"")
                            % ("\"" + to_file + "\"")
                            % ("\"" + options.tmpdir + "\"");

        singular::call_and_discard(command.str());
      }


    NO_NAME_MANGLING
      void pfd_fork_merge
      ( unsigned int const& id
      , unsigned int const& term_count
      , const pnet_options& options
      , const std::string& step
      )
      {
        unsigned int i;
        std::string file = get_from_name(step);
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);

        boost::format command =
              boost::format("pfd_fork_merge(%1%, %2%, %3%, %4%, %5%);")
                            % id
                            % term_count
                            % ("\"" + step + "\"")
                            % ("\"" + file + "\"")
                            % ("\"" + options.tmpdir + "\"");

        singular::call_and_discard(command.str());

        for (i = 1; i <= term_count; i++) {
          remove((options.tmpdir + "/" + file + "_result_" +
                       std::to_string(id) + "_" + std::to_string(i) +
                       ".ssi").c_str());
        }
      }

    NO_NAME_MANGLING
      unsigned int pfd_fork_merge_pair
      ( unsigned int const& id
      , unsigned int const& left
      , unsigned int const& right
      , const pnet_options& options
      , const std::string& step
      )
      {
        std::string left_time_path( get_term_time_path( id
                                                      , left
                                                      , "fork_compute_" + step
                                                      , options.tmpdir) );
        std::string right_time_path( get_term_time_path( id
                                                       , right
                                                       , "fork_compute_" + step
                                                       , options.tmpdir) );
        long left_time( get_written_time (left_time_path) );
        long right_time( get_written_time (right_time_path) );

        write_current_time(left_time_path);


        std::string file = get_from_name(step);
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);

        boost::format command =
             boost::format("pfd_fork_merge_pair(%1%, %2%, %3%, %4%, %5%);")
                           % id
                           % left
                           % right
                           % ("\"" + file + "\"")
                           % ("\"" + options.tmpdir + "\"");

        singular::call_and_discard(command.str());

        remove((options.tmpdir + "/" + file + "_result_" +
                       std::to_string(id) + "_" + std::to_string(right) +
                       ".ssi").c_str());




        long merge_time ( get_duration_time(left_time_path) );
        write_duration_time(left_time + right_time + merge_time, left_time_path);

        return left;
      }


    NO_NAME_MANGLING
      void pfd_fork_finish
      ( unsigned int const& id
      , unsigned int const& term_id
      , const pnet_options& options
      , const std::string& step
      )
      {
        //get times until now
        std::string computed_time_path(
            get_term_time_path( id
                              , term_id
                              , "fork_compute_" + step
                              , options.tmpdir) );
        std::string init_time_path(
            get_problem_time_path( id
                              , "init_" + step
                              , options.tmpdir) );
        long computed_time(get_written_time(computed_time_path));
        long init_time(get_written_time(init_time_path));

        // prepare to measure finishing step
        std::string step_time_path(
            get_problem_time_path( id
                                 , step
                                 , options.tmpdir) );
        write_current_time(step_time_path);

        // function contents
        std::string from_name(get_from_name(step));
        std::string to_name(get_to_name(step));

        /*
        */
        pfd_fork_compute_term_( id
                              , term_id
                              , options
                              , step
                              , from_name + "_result"
                              , from_name + "_result"
                              );

        std::string from_file(options.tmpdir + "/" + from_name + "_result_" +
                       std::to_string(id) + "_" + std::to_string(term_id) +
                       ".ssi");

        std::string to_file(options.tmpdir + "/" + to_name +
                                               "_" + std::to_string(id) +
                                               ".ssi");

        if (rename(from_file.c_str(), to_file.c_str()))
        {
          throw("Could not rename " + from_file + " to " + to_file);
        }


        // get function time
        long finish_time( get_duration_time(step_time_path) );
        write_duration_time( finish_time + computed_time + init_time
                           , step_time_path);
        log_duration( id
                    , options
                    , step
                    , finish_time + computed_time + init_time);

      }

    NO_NAME_MANGLING
      void pfd_write_result
      ( unsigned int const& id
      , const pnet_options& options
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);

        boost::format command =
              boost::format("out_struct output = pfd_singular_write_result(%1%, %2%);")
                            % id
                            % ("\"" + options.tmpdir + "\"");
        singular::call_and_discard(command.str());

        singular::write_ssi("output", get_out_struct_filename( options.tmpdir
                                            , config::parallel_pfd_base_name()
                                            , id)
                                            );
        singular::call_and_discard("kill output;");

        long step1( get_written_time(get_problem_time_path( id
                                                          , "NSSdecompStep"
                                                          , options.tmpdir)) );
        long step2( get_written_time(get_problem_time_path( id
                                                          , "shortNumeratorDecompStep"
                                                          , options.tmpdir)) );
        long step3( get_written_time(get_problem_time_path( id
                                                          , "algDependDecompStep"
                                                          , options.tmpdir)) );
        long step4( get_written_time(get_problem_time_path( id
                                                          , "numeratorDecompStep"
                                                          , options.tmpdir)) );
        log_duration(id, options, "total cpu", step1 + step2 + step3 + step4);

        remove((options.tmpdir + "/" + get_to_name(NUMERATOR) +"_"
                                     + std::to_string(id)
                                     + ".ssi").c_str()
              );
        remove((options.tmpdir + "/input_" + std::to_string(id) + ".ssi").c_str());
      }
  }
}
