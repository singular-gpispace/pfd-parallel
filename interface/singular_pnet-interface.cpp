#include <Singular/libsingular.h>

#include <interface/singular_pnet-interface.hpp>

#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include <boost/filesystem.hpp>

#include "singular_functions.hpp"
#include <config.hpp>

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
                                    "pfd_gspc_is_already_computed" +
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
        singular::call_and_discard( std::string("pfd_gspc_prepareSingleEntryFraction")
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
                                   "pfd_gspc_is_trivial"
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
        singular::call_and_discard("pfd_gspc_prepare_input"
                                    "(input, "
                                    + std::to_string(id)
                                    + ",\"" + options.tmpdir + "\""
                                    + ",\"" + get_from_name(first_step) + "\""
                                    ");");
        singular::call_and_discard("kill input;");
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
        singular::call_and_discard( "pfd_gspc_compute_step"
                                    "("
                                    "\"" +  step + "\""
                                    ", " + std::to_string(id) +
                                    ", \""+ options.tmpdir + "\""
                                    ", \"" + get_from_name(step) + "\""
                                    ", \"" + get_to_name(step) + "\""
                                    ");");

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
        singular::call_and_discard( "int count = pfd_loop_split_terms"
                                    "("
                                    + std::to_string(id) +
                                    ", \"" + in_file + "\""
                                    ", \"" + out_file + "\""
                                    ", \"" + options.tmpdir + "\""
                                    ");");
        //remove((options.tmpdir + "/" + file + "_" + std::to_string(id) + ".ssi").c_str());
        int term_count = singular::getInt("count");
        singular::call_and_discard( "kill count;");

        singular_parallel::pnet_list indices;
        int i;
        for (i = 1; i <= term_count; i++) {
          unsigned int term = i;
          indices.push_back(term);
        }
        return indices;
      }

    NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_init_loop
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      )
      {
        std::string file(get_from_name(step));

        singular_parallel::pnet_list indices(pfd_split_terms(id, options, file, file));

        singular::call_and_discard( "write(\"ssi:w "
                                     + options.tmpdir
                                     + "/"
                                     + file + "_dec_"
                                     + std::to_string(id)
                                     + ".ssi\", list());"
                                  );

        singular::call_and_discard( "write(\"ssi:w "
                                     + options.tmpdir
                                     + "/"
                                     + file + "_new_terms_"
                                     + std::to_string(id)
                                     + ".ssi\", list());"
                                  );

        return indices;
      }

/*

    NO_NAME_MANGLING
      void pfd_compute_short_numerator_for_term
      ( pnet_term_type const& in
      , const pnet_options& options
      )
      {
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);
        singular::call_and_discard( "pfd_gspc_compute_short_num_on_term"
                                    "("
                                    + std::to_string(in.id)
                                    + ",\"" + options.tmpdir + "\");");
        remove((options.tmpdir + "/terms_1_" + std::to_string(in.id) + ".ssi").c_str());

      }
      */

    NO_NAME_MANGLING
      int pfd_loop_merge
      ( unsigned int const& id
      , unsigned int const& term_id
      , const pnet_options& options
      , const std::string& step
      )
      {
        std::string file = get_from_name(step);
        init_singular ();

        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        singular::register_struct(options.out_struct_name,
                                  options.out_struct_desc);
        singular::load_library (options.needed_library);


        singular::call_and_discard( "int new_count = pfd_gspc_loop_merge"
                                    "("
                                    + std::to_string(id) +
                                    ", " + std::to_string(term_id) +
                                    ", \"" + file + "\""
                                    ", \"" + options.tmpdir + "\""
                                    ", \"" + step + "\""
                                    ");");
        int new_count = singular::getInt("new_count");
        singular::call_and_discard("kill new_count;");

        remove((options.tmpdir + "/" + file + "_result_" +
                       std::to_string(id) + "_" + std::to_string(term_id) +
                       ".ssi").c_str());

        return new_count;
      }

    NO_NAME_MANGLING
      singular_parallel::pnet_list  pfd_loop_cycle_terms
      ( unsigned int const& id
      , const pnet_options& options
      , const std::string& step
      )
      {
        std::string file = get_from_name(step);

        singular_parallel::pnet_list indices(
                                pfd_split_terms(id
                              , options
                              , file + "_new_terms"
                              , file));
        singular::call_and_discard( "write(\"ssi:w "
                                     + options.tmpdir
                                     + "/"
                                     + file + "_new_terms_"
                                     + std::to_string(id)
                                     + ".ssi\", list());"
                                  );

        return indices;
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
        singular::call_and_discard("out_struct output = "
                                    "pfd_gspc_write_result"
                                    "("
                                    + std::to_string(id)
                                    + ",\"" + options.tmpdir + "\");");
        singular::write_ssi("output", get_out_struct_filename( options.tmpdir
                                            , config::parallel_pfd_base_name()
                                            , id)
                                            );
        singular::call_and_discard("kill output;");
        remove((options.tmpdir + "/" + get_to_name(NUMERATOR) +"_"
                                     + std::to_string(id) 
                                     + ".ssi").c_str()
              );
        remove((options.tmpdir + "/input_" + std::to_string(id) + ".ssi").c_str());
      }
  }
}
