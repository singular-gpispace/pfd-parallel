#include <Singular/libsingular.h>

#include <interface/singular_pnet-interface.hpp>

#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include <boost/filesystem.hpp>

#include "singular_functions.hpp"

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
  if (!register_struct (reg_struct_name, reg_struct_desc))
  {
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
  if (a != b)
  {
    throw std::runtime_error ("( " + err_message + ") " +
                              std::to_string(a) +
                              " is not equal to " +
                              std::to_string(b));
  }
  return TRUE;
}


/*** Interface functions (declared in interface/singular_pnet-interface.hpp) ***/

NO_NAME_MANGLING
void singular_parallel_compute ( std::string const& path_to_libsingular
                               , std::string const& base_filename
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
  //std::cout << ids << " in singular_..._compute" << std::endl;
  init_singular (path_to_libsingular);

  safely_register_sing_struct(in_struct_name, in_struct_desc, ids);
  safely_register_sing_struct(out_struct_name, out_struct_desc, ids);

  int in_type, out_type;
  blackboxIsCmd (in_struct_name.c_str(), in_type);
  blackboxIsCmd (out_struct_name.c_str(), out_type);

  lists in_lst = ssi_read_newstruct(base_filename + ".i" + std::to_string (id),
                     in_struct_name);

  load_singular_library (needed_library);
  std::pair<int, lists> out = call_user_proc (function_name,
                                              needed_library,
                                              in_type,
                                              in_lst);
  check_integers_equal(out.first, out_type, ids +
                       " - singular_parallel_compute: incorrect return types" );


  ssi_write_newstruct ( get_out_struct_filename(base_filename, id)
                      , out_struct_name
                      , out.second);
  //std::cout << ids << ": A" << std::endl;
  //in_lst->Clean(); // TODO needs repairing
  //std::cout << ids << ": B" << std::endl;
  //out.second->Clean(); // TODO needs repairing
  //std::cout << ids << ": end of singular_..._compute" << std::endl;
}

NO_NAME_MANGLING
void pfd_parallel_compute ( std::string const& path_to_libsingular
                               , std::string const& base_filename
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
  //std::cout << ids << " in singular_..._compute" << std::endl;
  init_singular (path_to_libsingular);

  safely_register_sing_struct(in_struct_name, in_struct_desc, ids);
  safely_register_sing_struct(out_struct_name, out_struct_desc, ids);

  int in_type, out_type;
  blackboxIsCmd (in_struct_name.c_str(), in_type);
  blackboxIsCmd (out_struct_name.c_str(), out_type);

  lists in_lst = ssi_read_newstruct(base_filename + ".i" + std::to_string (id),
                     in_struct_name);

  load_singular_library (needed_library);
  std::pair<int, lists> out = call_user_proc (function_name,
                                              needed_library,
                                              in_type,
                                              in_lst);
  check_integers_equal(out.first, out_type, ids +
                       " - singular_parallel_compute: incorrect return types" );


  ssi_write_newstruct ( get_out_struct_filename(base_filename, id)
                      , out_struct_name
                      , out.second);
  //std::cout << ids << ": A" << std::endl;
  //in_lst->Clean(); // TODO needs repairing
  //std::cout << ids << ": B" << std::endl;
  //out.second->Clean(); // TODO needs repairing
  //std::cout << ids << ": end of singular_..._compute" << std::endl;
}

NO_NAME_MANGLING
singular_parallel::pnet_list sp_extract_neighbours
                               ( std::string const& path_to_libsingular
                               , std::string const& base_filename
                               , std::string const& in_struct_name
                               , std::string const& in_struct_desc
                               , std::string const& needed_library
                               , std::string const& expand_function
                               , unsigned long const& id
                               , unsigned int const& written_count
                               )
{
  std::string ids = get_id_string();

  /* set up singular instance */
  init_singular (path_to_libsingular);
  safely_register_sing_struct(in_struct_name, in_struct_desc, ids);

  int in_type;
  blackboxIsCmd (in_struct_name.c_str(), in_type);

  lists in_lst = ssi_read_newstruct(base_filename + ".i" + std::to_string (id),
                     in_struct_name);

  load_singular_library (needed_library);

  /* get neighbours of curren node */
  std::pair<int, lists> out = call_user_proc (expand_function,
                                              needed_library,
                                              in_type,
                                              in_lst);

  check_integers_equal(out.first, LIST_CMD, ids +
                       " - singular_parallel_compute: incorrect return types" );

  lists neigh_list = out.second;

  write_temp_structs_to_file( neigh_list
                            , base_filename
                            , in_type
                            );

  singular_parallel::pnet_list list;
  for (unsigned long i = 0; i < (unsigned long)neigh_list->nr + 1; i++) {
    boost::filesystem::rename( get_temp_struct_filename(base_filename, i)
                             , get_in_struct_filename( base_filename
                                                     , i + written_count
                                                     )
                             );
    list.push_back(written_count + i);
  }

  return list;
}

NO_NAME_MANGLING
singular_parallel::pnet_list sp_graph_extract_neighbours
                               ( std::string const& path_to_libsingular
                               , std::string const& base_filename
                               , std::string const& in_struct_name
                               , std::string const& in_struct_desc
                               , std::string const& needed_library
                               , std::string const& expand_function
                               , unsigned long const& id
                               , unsigned int const& written_count
                               , singular_parallel::pnet_set &hash_set // not constant!!
                               )
{
  std::string ids = get_id_string();

  /* set up singular instance */
  init_singular (path_to_libsingular);
  safely_register_sing_struct(in_struct_name, in_struct_desc, ids);

  int in_type;
  blackboxIsCmd (in_struct_name.c_str(), in_type);

  lists in_lst = ssi_read_newstruct(base_filename + ".i" + std::to_string (id),
                     in_struct_name);

  load_singular_library (needed_library);

  /* get neighbours of curren node */
  std::pair<int, lists> out = call_user_proc (expand_function,
                                              needed_library,
                                              in_type,
                                              in_lst);

  check_integers_equal(out.first, LIST_CMD, ids +
                       " - singular_parallel_compute: incorrect return types" );

  lists neigh_list = out.second;

  write_temp_structs_to_file( neigh_list
                            , base_filename
                            , in_type
                            );

  singular_parallel::pnet_list list;
  std::string temp_i_name;
  std::string temp_i_hash;
  unsigned long neigh_count = 0;
  for (int i = 0; i < neigh_list->nr + 1; i++) {
    temp_i_name = get_temp_struct_filename(base_filename, i);
    temp_i_hash = calculate_file_sha1_hash(temp_i_name);
    if (hash_set.find(temp_i_hash) == hash_set.end()) {
      std::cout << "not found in hashset: " 
                << std::to_string(written_count + neigh_count) 
                << " with hash "
                << temp_i_hash
                << "\n";
      std::cout << "renaming\n";
      boost::filesystem::rename( get_temp_struct_filename(base_filename, i)
                               , get_in_struct_filename( base_filename
                                                       , neigh_count + written_count
                                                       )
                               );
      std::cout << "inserting "
                << temp_i_hash
                << " into hashset\n";
      hash_set.insert(temp_i_hash);
      list.push_back(written_count + neigh_count);
      neigh_count++;
    } else {
      std::cout << "Already in hashset: "  
                << temp_i_hash
                << " "
                << temp_i_name
                << "\n";
    }
  }
  return list;
}

NO_NAME_MANGLING
singular_parallel::pnet_set create_initial_hashset( singular_parallel::pnet_list init_list
                                                  , std::string base_filename)
{
  unsigned long id;
  singular_parallel::pnet_set hash_set;
  std::string in_filename;
  std::string in_hash;

  for (singular_parallel::pnet_value v: init_list) { 
    id = boost::get<unsigned long>(v); 
    in_filename = get_in_struct_filename( base_filename, id);
    in_hash = calculate_file_sha1_hash(in_filename);
    hash_set.insert(in_hash);
    std::cout << "inserted initial "
              << in_hash
              << " for id "
              << std::to_string(id)
              << "\n";
  }

  return hash_set;
}
