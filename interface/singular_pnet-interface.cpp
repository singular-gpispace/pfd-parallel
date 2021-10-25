#include <Singular/libsingular.h>

#include <interface/singular_pnet-interface.hpp>

#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "singular_functions.hpp"
#include <config.hpp>
#include <chrono>
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
        std::string time_path(get_problem_time_path( id
                                                , "compute_" + step
                                                , options.tmpdir));
        write_current_time(time_path);

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
        long duration( get_duration_time(time_path) );
        write_duration_time ( duration , time_path);
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
        for (i = 0; i < term_count; i++) {
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
        long prev_init_time(get_written_time(time_path));
        write_current_time(time_path);

        // Function Content
        std::string file(get_from_name(step));

        singular_parallel::pnet_list indices(pfd_split_terms(id, options, file, file));

        int i;
        for (i = 0; i < (int)indices.size(); i++) {
          write_duration_time(0, get_term_time_path( id
                                                   , i
                                                   , "merge_" + step
                                                   , options.tmpdir) );
        }
        //
        long duration(get_duration_time (time_path));
        write_duration_time(duration + prev_init_time, time_path);

        return indices;
      }

    NO_NAME_MANGLING
      void pfd_hand_back
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      )
      {
        std::string to_name( get_to_name(step) );
        std::string from_name( get_from_name(step) );

        std::string to_path( options.tmpdir +
                             "/" + to_name + "_" + std::to_string(id) +
                             ".ssi");
        std::string from_path( options.tmpdir +
                             "/" + from_name + "_" + std::to_string(id) +
                             ".ssi");

        if (rename(to_path.c_str(), from_path.c_str()))
        {
          throw("Could not rename " + to_path + " to " + from_path);
        }
      }


      long log_step_component( unsigned int const& id
                             , const pnet_options& options
                             , std::string const& step
                             , std::string const& measure
                             )
      {
        std::string time_path(
            get_problem_time_path( id
                                 , measure + step
                                 , options.tmpdir) );
        long measure_time( get_written_time(time_path) );

        log_duration( id
                    , options
                    , measure + step
                    , measure_time);
        return measure_time;
      }

    NO_NAME_MANGLING
      void pfd_hand_forward
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      )
      {
        std::string to_name( get_to_name(step) );

        std::string from_path( options.tmpdir +
                             "/" + to_name + "_" + std::to_string(id) +
                             ".ssi");
        std::string to_path( options.tmpdir +
                             "/" + to_name + "_" + std::to_string(id) +
                             ".ssi");

        init_singular ();
        singular::load_library (options.needed_library);
        boost::format command =
              boost::format("pfd_singular_hand_forward(%1%, %2%);")
                            % ("\"" + from_path + "\"")
                            % ("\"" + to_path + "\"");

        singular::call_and_discard(command.str());

        std::string step_time_path(
            get_problem_time_path( id
                                 , step
                                 , options.tmpdir) );
        long init_time( log_step_component(id, options, step, "init_") );
        long compute_time( log_step_component(id, options, step, "compute_") );
        long merge_time( log_step_component(id, options, step, "merge_") );
        long finish_time( log_step_component(id, options, step, "finish_") );
        write_duration_time( init_time
                              + compute_time
                              + merge_time
                              + finish_time
                           , step_time_path);
        log_duration( id
                    , options
                    , step
                    , init_time +
                    compute_time +
                    merge_time +
                    finish_time);
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
                                                , "compute_" + step
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
              boost::format("pfd_fork_compute_term(%1%, %2%, %3%, %4%, %5%, %6%, %7%);")
                            % id
                            % term_id
                            % ("\"" + step + "\"")
                            % ("\"" + from_file + "\"")
                            % ("\"" + to_file + "\"")
                            % ("\"" + options.tmpdir + "\"")
                            % options.loop_max;

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

        // time merging step
        std::string left_merge_time_path( get_term_time_path( id
                                                      , left
                                                      , "merge_" + step
                                                      , options.tmpdir) );
        std::string right_merge_time_path( get_term_time_path( id
                                                      , right
                                                      , "merge_" + step
                                                      , options.tmpdir) );
        long left_merge_time( get_written_time (left_merge_time_path) );
        long right_merge_time( get_written_time (right_merge_time_path) );

        //std::cout << "path: "
        //          << left_merge_time
        //          << " and \n"
        //          << right_merge_time
        //          << "\n";

        write_current_time(left_merge_time_path);

        // merge times logged
        std::string left_compute_time_path( get_term_time_path( id
                                                      , left
                                                      , "compute_" + step
                                                      , options.tmpdir) );
        std::string right_compute_time_path( get_term_time_path( id
                                                       , right
                                                       , "compute_" + step
                                                       , options.tmpdir) );
        long left_time( get_written_time (left_compute_time_path) );
        long right_time( get_written_time (right_compute_time_path) );
        write_duration_time(left_time + right_time, left_compute_time_path);

        // merge the lists of terms
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




        long merge_time ( get_duration_time(left_merge_time_path) );

        write_duration_time(left_merge_time + right_merge_time + merge_time, left_merge_time_path);

        return left;
      }

    void update_summed_time( unsigned int const& id
                           , unsigned int const& term_id
                           , std::string const& step
                           , std::string const& measure
                           , const pnet_options& options
                           )
    {
      std::string term_time_path(
            get_term_time_path( id
                              , term_id
                              , measure + step
                              , options.tmpdir) );
        long term_time(get_written_time(term_time_path));

        std::string prev_time_path(
            get_problem_time_path( id
                              , measure + step
                              , options.tmpdir) );
        long previous_time(get_written_time(prev_time_path));
        write_duration_time(previous_time + term_time, prev_time_path);
    }


    NO_NAME_MANGLING
      unsigned int pfd_fork_finish
      ( unsigned int const& id
      , unsigned int const& term_id
      , const pnet_options& options
      , const std::string& step
      )
      {
        //log computed time
        update_summed_time (id, term_id, step, "compute_", options);
        update_summed_time (id, term_id, step, "merge_", options);

        // measure finishing step
        std::string finish_time_path(
            get_problem_time_path( id
                                 , "finish_" + step
                                 , options.tmpdir) );
        long previous_finish_time( get_written_time(finish_time_path) );
        write_current_time(finish_time_path);

        // function contents
        std::string from_name(get_from_name(step));
        std::string to_name(get_to_name(step));

        std::string dec_temp_file(options.tmpdir + "/" + from_name + "_dectemp_" +
                       std::to_string(id) +
                       ".ssi");
        std::string from_file(options.tmpdir + "/" + from_name + "_result_" +
                       std::to_string(id) + "_" + std::to_string(term_id) +
                       ".ssi");

        std::string to_file(options.tmpdir + "/" + to_name +
                                               "_" + std::to_string(id) +
                                               ".ssi");

        init_singular ();
        singular::load_library (options.needed_library);
        boost::format command =
            boost::format("pfd_merge_decs_and_write(%1%, %2%, %3%);")
                            % ("\"" + dec_temp_file + "\"")
                            % ("\"" + from_file + "\"")
                            % ("\"" + to_file + "\"");
        singular::call_and_discard(command.str());

        remove(from_file.c_str());
        remove(dec_temp_file.c_str());

        command = boost::format("int i = pfd_singular_terms_left(%1%);")
                            % ("\"" + to_file + "\"");
        singular::call_and_discard(command.str());

        unsigned int terms_left = singular::getInt("i");
        singular::call_and_discard("kill i;");


        // get function time
        long finish_time( get_duration_time(finish_time_path) );
        write_duration_time( finish_time + previous_finish_time
                           , finish_time_path);

        return terms_left;
      }

    NO_NAME_MANGLING
      void pfd_write_result
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string last_step
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

        std::string step_time_path(
            get_problem_time_path( id
                                 , last_step
                                 , options.tmpdir) );
        long init_time( log_step_component(id, options, last_step, "init_") );
        long compute_time( log_step_component(id, options, last_step, "compute_") );
        long merge_time( log_step_component(id, options, last_step, "merge_") );
        long finish_time( log_step_component(id, options, last_step, "finish_") );
        write_duration_time( init_time
                              + compute_time
                              + merge_time
                              + finish_time
                           , step_time_path);
        log_duration( id
                    , options
                    , last_step
                    , init_time +
                    compute_time +
                    merge_time +
                    finish_time);


        long step1( get_written_time(get_problem_time_path( id
                                                          , "NSSdecompStep"
                                                          , options.tmpdir)) );
        long step2( get_written_time(get_problem_time_path( id
                                                          , "shortNumeratorDecompStep"
                                                          , options.tmpdir)) );
        long step3( get_written_time(get_problem_time_path( id
                                                          , "algDependDecompStep"
                                                          , options.tmpdir)) );
        long step4(
                    init_time +
                    compute_time +
                    //merge_time +
                    finish_time
                  );
        log_duration(id, options, "total cpu", step1 + step2 + step3 + step4);

        remove((options.tmpdir + "/" + get_to_name(last_step) +"_"
                                     + std::to_string(id)
                                     + ".ssi").c_str()
              );
        remove((options.tmpdir + "/input_" + std::to_string(id) + ".ssi").c_str());
      }

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
      singular_parallel::pnet_list pfd_fork_get_tdegrees_before
      ( unsigned int const& id
      , const std::string& step
      , const pnet_options& options
      )
      {
        // Compute for input file of fork compute
        std::string from_file(get_from_name(step));

        return pfd_fork_get_tdegrees_file(id, from_file, options);
      }

    NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_fork_get_tdegrees_after
      ( unsigned int const& id
      , const std::string& step
      , const pnet_options& options
      )
      {
        // fork compute outputs a "to" name
        std::string to_file(get_to_name(step));

        return pfd_fork_get_tdegrees_file(id, to_file, options);
      }

    NO_NAME_MANGLING
      int pfd_fork_compare_tdegrees
      ( singular_parallel::pnet_list left
      , singular_parallel::pnet_list right
      )
      {
        int i, l, r;

        if (left.size() != right.size()) {
          if (left.size() < right.size()) {
            return -1;
          }
          if (left.size() > right.size()) {
            return 1;
          }
        }

        for (i = 0; (unsigned long)i < left.size(); i++) {
          l = boost::get<int> (left.front());
          left.pop_front();
          r = boost::get<int> (right.front());
          right.pop_front();
          if (l < r) {
            return 1;
          }
          if (l > r) {
            return -1;
          }
        }

        return 0;

      }

      NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_fork_get_tdegrees_file
      ( unsigned int const& id
      , const std::string& file
      , const pnet_options& options
      )
      {
        // Fire up and compute in singular
        init_singular ();
        singular::load_library (options.needed_library);
        boost::format command =
              boost::format("list l = pfd_singular_get_tdegree_vector(%1%, %2%, %3%);")
                            % id
                            % ("\"" + file + "\"")
                            % ("\"" + options.tmpdir + "\"");
        singular::call_and_discard(command.str());

        // Extract into CPP data  structures
        int n = singular::getList("l")->nr + 1;
        std::list<int> ret_list;
        int i;
        singular::call_and_discard("int i;");
        for (i = 0; i < n; i++) {
          singular::call_and_discard(("i = l[" + std::to_string(i + 1) + "];")
                                      .c_str());
          ret_list.push_front(singular::getInt("i"));
        }

        // Clean up Singular
        singular::call_and_discard("kill l;");
        singular::call_and_discard("kill i;");

        // We assume a sorted list, for comparisons
        ret_list.sort(std::greater<int>());

        // repackage for pnet
        singular_parallel::pnet_list ret_value_list;
        for (int i : ret_list) {
          ret_value_list.push_back(i);
        }

        return ret_value_list;
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
