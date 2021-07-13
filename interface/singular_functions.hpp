#pragma once

#include <string>

#include <Singular/libsingular.h>
#include <Singular/links/ssiLink.h> // for ssiInfo etc.
#include <Singular/newstruct.h>

#include <interface/type_aliases.hpp>

// Singular defines this in ssiLink.cc
#define SSI_VERSION 13

// these are from ssiLink.cc
char* ssiReadString(const ssiInfo *d);

// these are from newstruct.cc
BOOLEAN newstruct_deserialize(blackbox **, void **d, si_link f);
BOOLEAN newstruct_serialize(blackbox *b, void *d, si_link f);

//structs
class ScopedLeftv
{
  public:
    ScopedLeftv (int c, void* data);
    ScopedLeftv (ScopedLeftv& parent, int c, void* data);
    ~ScopedLeftv();
    ScopedLeftv (ScopedLeftv const&) = delete;
    ScopedLeftv (ScopedLeftv&&) = delete;
    ScopedLeftv& operator= (ScopedLeftv const&) = delete;
    ScopedLeftv& operator= (ScopedLeftv&&) = delete;
    leftv leftV() const;
  private:
    leftv _;
    bool chained = false;
};


//bool init_singular (std::string const&);
bool init_singular ();
void load_singular_library (std::string const&);
bool register_struct(std::string const&, std::string const&);
si_link ssi_open_for_read (std::string const&);
si_link ssi_open_for_write (std::string const&);
void ssi_close_and_remove (si_link);
void ssi_write_newstruct (si_link, std::string const&, lists);
void ssi_write_newstruct (si_link, int, lists);
void ssi_write_newstruct ( std::string filename
                         , int const& struct_token
                         , lists lst);
void ssi_write_newstruct ( std::string filename, std::string const& struct_name
                         , lists lst);
void ssi_write_newstruct ( std::string const& filename, int const& in_token
                         , sleftv const& m);
void ssi_write_newstruct ( std::string const& filename
                         , std::string const& struct_name
                         , sleftv const& m);

void double_ssi_write_newstruct ( std::string const& filename
                                , int const& in_token
                                , sleftv const& m
                                );

lists ssi_read_newstruct (si_link, std::string const&);
lists ssi_read_newstruct(std::string const& file_name,
                        std::string const& struct_name);

std::pair<int, lists> call_user_proc (std::string const&,
                                      std::string const&,
                                      int,
                                      lists);

std::pair<int, lists> call_user_proc (std::string const&,
                                      std::string const&,
                                      ScopedLeftv&);




void write_struct_to_file ( sleftv const& m
                          , int const& in_token
                          , std::string const& filename);
void write_in_structs_to_file( lists const& struct_list
                             , std::string const& root
                             , std::string const& basename
                             , int const& in_token);
void write_in_structs_to_file_from_index( lists const& struct_list
                                        , int const& index
                                        , std::string const& root
                                        , std::string const& basename
                                        , int const& in_token);
void write_temp_structs_to_file( lists const& struct_list
                               , std::string const& root
                               , std::string const& basename
                               , int const& in_token);


/*** direct singular call based function ***/

namespace singular {

  void call (std::string const& command);

  void call_and_discard (std::string const& command);

  std::string get_result (std::string const& command);

  void register_struct(std::string struct_name, std::string struct_desc);

  void load_library
    ( std::string const& library_name
    , bool enforce_reload = false
    );


  void load_ssi
    ( const std::string& symbol_name
    , const ::singular_parallel::pnet_value& files
    );
   void load_ssi
    ( const std::string& symbol_name
      , const std::string& file
    );

  void load_ssi_if_not_defined
  ( const std::string& symbol_name
  , const ::singular_parallel::pnet_value& files
  );

  void write_ssi (const std::string& symbol_name, std::string const& file_name);

  bool symbol_exists(const std::string& symbol_name);

  intvec* getIntvec(const std::string& symbol);

  lists getList(const std::string& symbol);

  void put(const std::string& symbol, lists list);

  void put(const std::string& symbol, intvec* iv);

  template<typename T, typename... Args>
  void kill(const T& t, Args&&... args);


}






/*** user functions TODO: these should be split out ***/

std::string get_in_struct_filename( std::string const& root
                                  , std::string const& basename
                                  , unsigned long const& id);
std::string get_out_struct_filename( std::string const& root
                                   , std::string const& basename
                                   , unsigned long const& id);
std::string get_temp_struct_filename( std::string const& root
                                    , std::string const& basename
                                    , unsigned long const& id);

std::string calculate_file_sha1_hash(std::string const& disk_filename);
std::string calculate_file_sha1_hash(std::ifstream & ifs);

