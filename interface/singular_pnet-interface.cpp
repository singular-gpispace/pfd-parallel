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
      (
       std::string const& tmpdir
       , unsigned int const& id
       , std::string const& function_name
       , std::string const& needed_library
       , std::string const& in_struct_name
       , std::string const& in_struct_desc
       , std::string const& out_struct_name
       , std::string const& out_struct_desc
      )
      {
        std::string ids = get_id_string();

        init_singular ();

        safely_register_sing_struct(in_struct_name, in_struct_desc, ids);
        safely_register_sing_struct(out_struct_name, out_struct_desc, ids);

        int in_type, out_type;
        blackboxIsCmd (in_struct_name.c_str(), in_type);
        blackboxIsCmd (out_struct_name.c_str(), out_type);

        lists in_lst = ssi_read_newstruct(
            get_in_struct_filename( tmpdir
                                  , config::parallel_list_base_name()
                                  , id)
            , in_struct_name
            );

        load_singular_library (needed_library);
        std::pair<int, lists> out = call_user_proc (function_name,
            needed_library,
            in_type,
            in_lst);
        check_integers_equal(out.first, out_type, ids +
            " - singular_parallel_compute: incorrect return types" );


        ssi_write_newstruct
            ( get_out_struct_filename( tmpdir
                                     , config::parallel_list_base_name()
                                     , id)
            , out_struct_name
            , out.second);
      }

    NO_NAME_MANGLING
      void pfd_parallel_compute
      (  unsigned int const& id
       , const singular_parallel::pnet_options& options
      )
      {
        std::string ids = get_id_string();
        //std::cout << ids << " in singular_..._compute" << std::endl;
        init_singular ();

        safely_register_sing_struct(options.in_struct_name,
                                    options.in_struct_desc,
                                    ids);
        safely_register_sing_struct(options.out_struct_name,
                                    options.out_struct_desc,
                                    ids);

        int in_type, out_type;
        blackboxIsCmd (options.in_struct_name.c_str(), in_type);
        blackboxIsCmd (options.out_struct_name.c_str(), out_type);

        lists in_lst = ssi_read_newstruct
            ( get_in_struct_filename( options.tmpdir
                                    , config::parallel_pfd_base_name()
                                    , id)
            , options.in_struct_name
            );

        load_singular_library (options.needed_library);
        std::pair<int, lists> out = call_user_proc (options.function_name,
            options.needed_library,
            in_type,
            in_lst);
        check_integers_equal(out.first, out_type, ids +
            " - singular_parallel_compute: incorrect return types" );


        ssi_write_newstruct
            ( get_out_struct_filename( options.tmpdir
                                     , config::parallel_pfd_base_name()
                                     , id)
            , options.out_struct_name
            , out.second);
      }

  }
}
