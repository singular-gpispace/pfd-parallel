#pragma once

#include <string>

#include <Singular/libsingular.h>
#include <Singular/links/ssiLink.h> // for ssiInfo etc.
#include <Singular/newstruct.h>

// Singular defines this in ssiLink.cc
#define SSI_VERSION 13

// these are from ssiLink.cc
char* ssiReadString(const ssiInfo *d);

// these are from newstruct.cc
BOOLEAN newstruct_deserialize(blackbox **, void **d, si_link f);
BOOLEAN newstruct_serialize(blackbox *b, void *d, si_link f);

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

std::pair<int, lists> call_user_proc
  (std::string const&, std::string const&, int, lists);


std::string get_in_struct_filename( std::string base_filename
                                  , unsigned long const& id);
std::string get_out_struct_filename( std::string base_filename
                                   , unsigned long const& id);
std::string get_temp_struct_filename( std::string base_filename
                                    , unsigned long const& id);

void write_struct_to_file ( sleftv const& m
                          , int const& in_token
                          , std::string const& filename);
void write_in_structs_to_file( lists const& struct_list
                             , std::string const& base_filename
                             , int const& in_token);
void write_in_structs_to_file_from_index( lists const& struct_list
                                        , int const& index
                                        , std::string const& base_filename
                                        , int const& in_token);
void write_temp_structs_to_file( lists const& struct_list
                               , std::string const& base_filename
                               , int const& in_token);

std::string calculate_file_sha1_hash(std::string const& disk_filename);
std::string calculate_file_sha1_hash(std::ifstream & ifs);

