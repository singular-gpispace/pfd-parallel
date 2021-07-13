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


    /*** Interface functions (declared in interface/singular_pnet-interface.hpp) ***/

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
        singular::call_and_discard("out_struct output = " +
                                    options.function_name +
                                    "(input);");
        singular::write_ssi("output", get_out_struct_filename( options.tmpdir
                                            , config::parallel_list_base_name()
                                            , id)
                                            );
      }

    NO_NAME_MANGLING
      void pfd_parallel_compute
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
        singular::call_and_discard("out_struct output = " +
                                    options.function_name +
                                    "(input);");
        singular::write_ssi("output", get_out_struct_filename( options.tmpdir
                                            , config::parallel_pfd_base_name()
                                            , id)
                                            );
      }

  }
}
