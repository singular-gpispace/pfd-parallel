#include <Singular/libsingular.h>

#include <interface/singular_pnet-interface.hpp>
#include <interface/pnet_logging.hpp>

#include <sys/stat.h>
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

    NO_NAME_MANGLING
      void pfd_merge_sort
      ( unsigned int *input
      , long *sizes
      , unsigned int low
      , unsigned int high
      , unsigned int *buffer
      );

    NO_NAME_MANGLING
      void pfd_merge_lists
      ( unsigned int *input
      , long *sizes
      , unsigned int low
      , unsigned int mid
      , unsigned int high
      , unsigned int *buffer
      );

    NO_NAME_MANGLING
      void pfd_quick_sort
      ( unsigned int *input
      , long *sizes
      , int low
      , int high
      );

    NO_NAME_MANGLING
      int pfd_quick_partition
      ( unsigned int *input
      , long *sizes
      , int low
      , int high
      );



    NO_NAME_MANGLING
      void sort_input_files_by_size
      ( unsigned int *input
      , unsigned int count
      , const pnet_options& options
      , const std::string net_type
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

    NO_NAME_MANGLING
      void pfd_swap
      ( unsigned int *input
      , int index1
      , int index2
      )
      {
        unsigned int temp = input[index1];
        input[index1] = input[index2];
        input[index2] = temp;
      }

    NO_NAME_MANGLING
      void pfd_merge_lists
      ( unsigned int *input
      , long *sizes
      , unsigned int low
      , unsigned int mid
      , unsigned int high
      , unsigned int *buffer
      )
      {
        unsigned int i, l, r, count;
        if (mid < low) {
          throw std::runtime_error("merge needs low <= mid");
        }
        if (high < mid) {
          throw std::runtime_error("merge needs mid <= high");
        }
        if (low == high) {
          throw std::runtime_error("low == high: This should never happen in merging");
        }
        count = high - low + 1;
        l = low;
        r = mid + 1;

        if (buffer == NULL) {
          throw std::runtime_error("malloc fail in sorting's merge step");
        }
        for (i = 0; i < count ; i++) {
          if (l > mid) {
              buffer[i] = input[r++];
          } else if (r > high) {
              buffer[i] = input[l++];
          } else {
            if (sizes[input[l]] > sizes[input[r]]) {
              buffer[i] = input[r++];
            } else {
              buffer[i] = input[l++];
            }
          }
        }
        for (i = 0; i < count; i++) {
          input[low + i] = buffer[i];
          buffer[i] = 0;
        }
      }

    NO_NAME_MANGLING
      void pfd_merge_sort
      ( unsigned int *input
      , long * sizes
      , unsigned int low
      , unsigned int high
      , unsigned int *buffer
      )
      {
        unsigned int mid;

        if ( high > low) {
          mid = (unsigned int)((low + high) / 2);
          pfd_merge_sort(input, sizes, low, mid, buffer);
          pfd_merge_sort(input, sizes, mid + 1, high, buffer);
          pfd_merge_lists(input, sizes, low, mid, high, buffer);

        } else if (high == low) {
          return;
        } else {
          std::cout << "weird in sorting\n";
          throw std::runtime_error ("This should never happen when sorting");
        }
      }

    NO_NAME_MANGLING
      void pfd_quick_sort
      ( unsigned int *input
      , long *sizes
      , int low
      , int high
      )
      {
        if (low >= high) {
          return;
        }
        if ((low < 0) or (high < 0)) {
          // this should never happen
          throw std::runtime_error("Bad indices, should have been caught");
        }
        unsigned int pivot = pfd_quick_partition(input, sizes, low, high);
        pfd_quick_sort(input, sizes, low, pivot - 1);
        pfd_quick_sort(input, sizes, pivot + 1, high);

      }

    NO_NAME_MANGLING
      unsigned int pfd_quick_pivot
      ( unsigned int *input
      , long *sizes
      , int low
      , int high
      )
      {
        // local function
        int mid = (int)((low + high) / 2);

        if(sizes[input[mid]] < sizes[input[low]]) {
          pfd_swap(input, low, mid);
        }

        if(sizes[input[high]] < sizes[input[low]]) {
          pfd_swap(input, low, high);
        }

        if(sizes[input[mid]] < sizes[input[high]]) {
          // make sure the "median of three" is in position of high
          pfd_swap(input, low, high);
        }

        return input[high];
      }

    NO_NAME_MANGLING
      int pfd_quick_partition
      ( unsigned int *input
      , long *sizes
      , int low
      , int high
      )
      {
        //local function
        // the real workhorse
        int i, j;
        unsigned int pivot = pfd_quick_pivot(input, sizes, low, high);
        long pivot_size = sizes[pivot];

        i = low;
        j = high - 1;

        while (i < j) {
          while (i < high and sizes[input[i]] < pivot_size) {
            i++;
          }
          while (j > low and sizes[input[j]] >= pivot_size) {
            j--;
          }
          if (i < j) {
            pfd_swap(input, i, j);
          }
        }

        if (sizes[input[i]] > pivot_size) {
            pfd_swap(input, i, high);
        }

        return i;
      }



    NO_NAME_MANGLING
      void sort_input_files_by_size
      ( unsigned int *input
      , unsigned int count
      , const pnet_options& options
      , const std::string net_type
      )
      {
        unsigned int i;
        long *sizes =
          (long *)malloc(sizeof(long) * count);
        for (i = 0; i < count; i++) {
          sizes[i] = get_input_file_size(i, options, net_type);
        }
        pfd_quick_sort(input, sizes, 0, count - 1);

        for (i = 0; i < count; i++) {
          sizes[i] = 0;
        }
        free(sizes);
        sizes = NULL;
      }

    NO_NAME_MANGLING
      void sort_term_files_by_size
      ( unsigned int *input
      , unsigned int count
      , const pnet_options& options
      , const std::string net_type
      )
      {
        unsigned int i;
        long *sizes =
          (long *)malloc(sizeof(long) * count);
        unsigned int *buffer =
          (unsigned int *)malloc(sizeof(unsigned int) * count);
        for (i = 0; i < count; i++) {
          sizes[i] = get_input_file_size(i, options, net_type);
          buffer[i] = 0;
        }
        pfd_merge_sort(input, sizes, 0, count - 1, buffer);
        free(sizes);
        free(buffer);
        sizes = NULL;
        buffer = NULL;
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
      long get_filesize
      ( std::string path )
      {
        struct stat stat_buf;
        int rc = stat(path.c_str(), &stat_buf);
        return rc == 0 ? stat_buf.st_size : -1;
      }

    NO_NAME_MANGLING
      long get_input_file_size
      ( unsigned int id
      , const pnet_options& options
      , const std::string net_type
      )
      {
        int row, col;
        std::string matrix_name;
        init_singular ();
        singular::register_struct(options.in_struct_name,
                                  options.in_struct_desc);
        if (net_type == "pfd") {
          singular::load_ssi("input", get_in_struct_filename( options.tempdir
                                        , config::parallel_pfd_base_name()
                                        , id));
        } else if (net_type == "list") {
          singular::load_ssi("input", get_in_struct_filename( options.tempdir
                                        , config::parallel_list_base_name()
                                        , id));
        }

        singular::call_and_discard("int r = input.row;");
        singular::call_and_discard("int c = input.col;");
        singular::call_and_discard("string matrixname = input.matrixname;");
        row = singular::getInt("r");
        col = singular::getInt("c");
        matrix_name = singular::getString("matrixname");

        singular::call_and_discard("kill r;");
        singular::call_and_discard("kill c;");
        singular::call_and_discard("kill input;");

        return get_filesize(options.from_dir + "/" +
                            matrix_name + "_" +
                            std::to_string(row) + "_" +
                            std::to_string(col) + ".ssi"
                            );
      }

    NO_NAME_MANGLING
      singular_parallel::pnet_list pfd_sorted_input_by_size
      ( unsigned int count
      , const pnet_options& options
      , const std::string net_type
      )
      {
        unsigned int id;
        singular_parallel::pnet_list sorted_list;
        unsigned int *input =
          (unsigned int *)malloc(sizeof(unsigned int) * count);
        for (id = 0; id < count; id++) {
          input[id] = id;
        }

        // sort
        sort_input_files_by_size(input, count, options, net_type);
        // sorted

        for (id = 0; id < count; id++) {
          sorted_list.push_back(input[id]);
          input[id] = 0;
        }

        free(input);
        input = NULL;

        return sorted_list;
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
        singular::load_ssi("input", get_in_struct_filename( options.tempdir
                                        , config::parallel_list_base_name()
                                        , id));
        singular::call_and_discard(options.out_struct_name + " output = " +
                                    options.function_name +
                                    "(input);");
        singular::write_ssi("output", get_out_struct_filename( options.tempdir
                                            , config::parallel_list_base_name()
                                            , id)
                                            );
        singular::call_and_discard("kill output;");
      }

    NO_NAME_MANGLING
      void pfd_serial_compute_pfd
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
        singular::load_ssi("input", get_in_struct_filename( options.tempdir
                                        , config::parallel_pfd_base_name()
                                        , id));
        singular::call_and_discard(options.out_struct_name + " output = " +
                                    options.function_name +
                                    "(input);");
        singular::write_ssi("output", get_out_struct_filename( options.tempdir
                                            , config::parallel_pfd_base_name()
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
        singular::load_ssi("input", get_in_struct_filename( options.tempdir
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
                             , get_out_struct_filename( options.tempdir
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
        singular::load_ssi("input", get_in_struct_filename( options.tempdir
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
        singular::load_ssi("input", get_in_struct_filename( options.tempdir
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
          singular::write_ssi("output", get_out_struct_filename( options.tempdir
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
        singular::load_ssi("input", get_in_struct_filename( options.tempdir
                                        , config::parallel_pfd_base_name()
                                        , id));
        boost::format command =
              boost::format("pfd_singular_prepare_input(%1%, %2%, %3%, %4%);")
                            % "input"
                            % id
                            % ("\"" + options.tempdir + "\"")
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
        singular::load_ssi("input", get_in_struct_filename( options.tempdir
                                        , config::parallel_pfd_base_name()
                                        , id));
        boost::format command =
              boost::format("int prepstat = pfd_singular_general_prepare(%1%, %2%, %3%, %4%);")
                            % "input"
                            % id
                            % ("\"" + options.tempdir + "\"")
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
            throw std::runtime_error("Unrecognised return value from general prepare");
          }}
          singular::write_ssi( "internal_temp_o"
                             , get_out_struct_filename( options.tempdir
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
                                                , options.tempdir));
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
                            % ("\"" + options.tempdir + "\"");

        singular::call_and_discard(command.str());

        remove((options.tempdir + "/" + get_from_name(step) +"_"
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
                            % ("\"" + options.tempdir + "\"")
                            % options.split_max;

        singular::call_and_discard(command.str());

        remove((options.tempdir + "/" + in_file + "_" + std::to_string(id) + ".ssi").c_str());
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
                                 , options.tempdir
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
                                                   , options.tempdir) );
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

        std::string to_path( options.tempdir +
                             "/" + to_name + "_" + std::to_string(id) +
                             ".ssi");
        std::string from_path( options.tempdir +
                             "/" + from_name + "_" + std::to_string(id) +
                             ".ssi");

        if (rename(to_path.c_str(), from_path.c_str()))
        {
          throw std::runtime_error("Could not rename " + to_path + " to " + from_path);
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
                                 , options.tempdir) );
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

        std::string from_path( options.tempdir +
                             "/" + to_name + "_" + std::to_string(id) +
                             ".ssi");
        std::string to_path( options.tempdir +
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
                                 , options.tempdir) );
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
                                                , options.tempdir));
        write_current_time(time_path);

        std::string file(get_from_name(step));
        pfd_fork_compute_term_( id
                             , term_id
                             , options
                             , step
                             , file
                             , file + "_result"
                             );
        remove((options.tempdir + "/" + file + "_" + std::to_string(id) +
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
                            % ("\"" + options.tempdir + "\"")
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
                            % ("\"" + options.tempdir + "\"");

        singular::call_and_discard(command.str());

        for (i = 1; i <= term_count; i++) {
          remove((options.tempdir + "/" + file + "_result_" +
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
                                                      , options.tempdir) );
        std::string right_merge_time_path( get_term_time_path( id
                                                      , right
                                                      , "merge_" + step
                                                      , options.tempdir) );
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
                                                      , options.tempdir) );
        std::string right_compute_time_path( get_term_time_path( id
                                                       , right
                                                       , "compute_" + step
                                                       , options.tempdir) );
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
                           % ("\"" + options.tempdir + "\"");

        singular::call_and_discard(command.str());

        remove((options.tempdir + "/" + file + "_result_" +
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
                              , options.tempdir) );
        long term_time(get_written_time(term_time_path));

        std::string prev_time_path(
            get_problem_time_path( id
                              , measure + step
                              , options.tempdir) );
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
                                 , options.tempdir) );
        long previous_finish_time( get_written_time(finish_time_path) );
        write_current_time(finish_time_path);

        // function contents
        std::string from_name(get_from_name(step));
        std::string to_name(get_to_name(step));

        std::string dec_temp_file(options.tempdir + "/" + from_name + "_dectemp_" +
                       std::to_string(id) +
                       ".ssi");
        std::string from_file(options.tempdir + "/" + from_name + "_result_" +
                       std::to_string(id) + "_" + std::to_string(term_id) +
                       ".ssi");

        std::string to_file(options.tempdir + "/" + to_name +
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
                            % ("\"" + options.tempdir + "\"");
        singular::call_and_discard(command.str());

        singular::write_ssi("output", get_out_struct_filename( options.tempdir
                                            , config::parallel_pfd_base_name()
                                            , id)
                                            );
        singular::call_and_discard("kill output;");

        std::string step_time_path(
            get_problem_time_path( id
                                 , last_step
                                 , options.tempdir) );
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
                    , init_time
                      + compute_time
                      + merge_time
                      + finish_time
                    );


        long step1( get_written_time(get_problem_time_path( id
                                                          , "NSSdecompStep"
                                                          , options.tempdir)) );
        long step2( get_written_time(get_problem_time_path( id
                                                          , "shortNumeratorDecompStep"
                                                          , options.tempdir)) );
        long step3( get_written_time(get_problem_time_path( id
                                                          , "algDependDecompStep"
                                                          , options.tempdir)) );
        long step4( get_written_time(get_problem_time_path( id
                                                          , "numeratorDecompStep"
                                                          , options.tempdir)) );
        log_duration(id, options, "total cpu", step1 + step2 + step3 + step4);

        remove((options.tempdir + "/" + get_to_name(last_step) +"_"
                                     + std::to_string(id)
                                     + ".ssi").c_str()
              );
        remove((options.tempdir + "/input_" + std::to_string(id) + ".ssi").c_str());
      }
  }
}
