#include "singular_functions.hpp"

#include <stdexcept>

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <boost/uuid/sha1.hpp>
#include <boost/filesystem.hpp>
#include <src/config.hpp>
#include <util-generic/finally.hpp>

/*** Private function declarations ***/

std::string hash_to_string(const unsigned int *hash, const int len);
std::ifstream open_file (std::string const& fname);

/*** Private functions ***/

std::ifstream open_file (std::string const& fname)
{
  std::ifstream ifs(fname,std::ios::binary);

  if(!ifs.good()) {
    throw std::runtime_error("bad bile name\n");
  }
  return ifs;
}

std::string hash_to_string( const unsigned int *hash, const int len)
{
  std::stringstream strstr;
  strstr <<  std::hex<<std::setfill('0')
         <<  std::setw(sizeof(int)*2);
  for (int i=0; i < len; i++) {
    strstr << hash[i];
  }

  return strstr.str();
}

/*** Public functions ***/

idhdl symbol (std::string const& lib, std::string const& fun)
{
  load_singular_library (lib);
  idhdl h = ggetid (fun.c_str());
  if (h == NULL)
  {
    throw std::runtime_error ("procedure " + fun + " not available in " + lib);
  }
  return h;
}


ScopedLeftv::ScopedLeftv (int c, void* data)
: _ (static_cast<leftv> (omAlloc0Bin (sleftv_bin))) // TODO check result
{
  _->rtyp = c;
  _->data = data;
}
  ScopedLeftv::ScopedLeftv (ScopedLeftv& parent, int c, void* data)
: ScopedLeftv (c, data)
{
  chained = true;
  parent._->next = _;
}
ScopedLeftv::~ScopedLeftv()
{
  if (!chained) {
    _->CleanUp();
    omFreeBin (_, sleftv_bin);
  }
};

leftv ScopedLeftv::leftV() const
{
  return _;
}

template<typename R> std::pair<int, R> proc (idhdl h, ScopedLeftv const& arg)
{
  BOOLEAN const res (iiMake_proc (h, NULL, arg.leftV()));

  if (res)
  {
    throw std::runtime_error ("call to procedure " + std::string (h->id)
                              + " failed");
  }

  R const r = static_cast<R> (iiRETURNEXPR.Data());
  int const i = iiRETURNEXPR.Typ();

  iiRETURNEXPR.data = NULL;
  iiRETURNEXPR.CleanUp();

  return std::make_pair (i, r);
}

static void error_callback(const char* msg)
{
  throw std::runtime_error("Singular error: " + std::string(msg));
}

bool init_singular ()
{
  if (currPack == NULL) // use this to check if this instance has already been
                        // initializied
  {
    mp_set_memory_functions (omMallocFunc, omReallocSizeFunc, omFreeSizeFunc);
    siInit (const_cast<char*> (config::singularLibrary().string().c_str()));
    currentVoice = feInitStdin (NULL);
    errorreported = 0;
    myynest = 1;
    WerrorS_callback=error_callback;

    return true;
  }
  return false;
}

void load_singular_library (std::string const& library_name)
{
  char *s = omStrDup (library_name.c_str());
  BOOLEAN res = iiGetLibStatus (s);
  if (res == TRUE) // already loaded
  {
    omFree (s);
    return;
  }
  res = iiLibCmd (s, TRUE, FALSE, FALSE); // also frees s
  if (res)
  {
    throw std::runtime_error ("could not load library " + library_name);
  }
}

bool register_struct(std::string const& name, std::string const& desc)
{
  int tok;
  int cmd_result = blackboxIsCmd (name.c_str(), tok);
  if (cmd_result == ROOT_DECL)
  {
    return true; // already exists
  }

  newstruct_desc d = newstructFromString (desc.c_str());
  if (d == NULL)
  {
    return false;
  }
  newstruct_setup (name.c_str(), d); // is void function
  return true;
}

si_link ssi_open_for_read (std::string const& ssi_filename)
{
  si_link l = static_cast<si_link> (omAlloc0Bin (sip_link_bin));
  l->name = omStrDup (ssi_filename.c_str());
  l->mode = omStrDup ("r");
  l->ref = 1;
  si_link_extension ns = static_cast<si_link_extension>
    (omAlloc0Bin (s_si_link_extension_bin));
  ns = slInitSsiExtension (ns);
  l->m = ns; // originally, Singular has a global list of "extensions"
             // we use a private copy for now

  BOOLEAN res = l->m->Open (l, SI_LINK_READ, NULL);
  if (res)
  {
    throw std::runtime_error ("could not open ssi file " + ssi_filename);
  }

  ssiInfo *d = static_cast<ssiInfo*> (l->data);
  int t = s_readint (d->f_read); // SSI version info in first line
  if (t != 98)
  {
    std::string error_msg = "wrong token, got " + std::to_string (t);
    throw std::runtime_error (error_msg);
  }
  int n98_v, n98_m;
  BITSET n98_o1, n98_o2;
  n98_v = s_readint (d->f_read);
  n98_m = s_readint (d->f_read);
  n98_o1 = s_readint (d->f_read);
  n98_o2 = s_readint (d->f_read);
  if ((n98_v != SSI_VERSION) || (n98_m != MAX_TOK))
  {
    std::string error_msg = "incompatible versions of ssi: expected "
      + std::to_string (SSI_VERSION) + '/' + std::to_string (MAX_TOK)
      + " got " + std::to_string (n98_v) + '/' + std::to_string (n98_m);
    throw std::runtime_error (error_msg);
  }
  si_opt_1 = n98_o1; // do I want to set these global options?
  si_opt_2 = n98_o2;

  return l;
}

si_link ssi_open_for_write (std::string const& ssi_filename)
{
  si_link l = static_cast<si_link> (omAlloc0Bin (sip_link_bin));
  l->name = omStrDup (ssi_filename.c_str());
  l->mode = omStrDup ("w");
  l->ref = 1;
  si_link_extension ns = static_cast<si_link_extension>
    (omAlloc0Bin (s_si_link_extension_bin));
  ns = slInitSsiExtension (ns);
  l->m = ns;

  BOOLEAN res = l->m->Open (l, SI_LINK_WRITE, NULL);
  // this already writes the parameter line "98 ..."

  if (res)
  {
    throw std::runtime_error ("could not open ssi file " + ssi_filename
                              + " for write");
  }

  return l;
}

void ssi_close_and_remove (si_link l)
{
  // it seems that PrepClose is not used (not set) for ssi links
  BOOLEAN res = l->m->Close (l); // this frees l->d
  if (res)
  {
    throw std::runtime_error ("closing ssi link failed");
  }
  omFree (static_cast<ADDRESS> (l->name));
  omFree (static_cast<ADDRESS> (l->mode));
  omFreeBin (l->m, s_si_link_extension_bin);
  omFreeBin (l, sip_link_bin);
}

void ssi_write_newstruct ( std::string const& filename
                         , std::string const& struct_name
                         , sleftv const& m
                         )
{
  int type;
  blackboxIsCmd (struct_name.c_str(), type);
  ssi_write_newstruct ( filename
                      , type
                      , m
                      );
}

void ssi_write_newstruct ( std::string const& filename
                         , int const& in_token
                         , sleftv const& m
                         )
{
  int task_type = m.rtyp;
  if (task_type != in_token) {
    throw std::invalid_argument ( "wrong type for argument, expected "
                                + std::to_string (in_token)
                                + " got "
                                + std::to_string (task_type));
  }

  si_link l = ssi_open_for_write(filename);
  ssi_write_newstruct (l,
                       in_token,
                       static_cast<lists> (m.data));
  ssi_close_and_remove (l);
}

void ssi_write_newstruct ( std::string filename
                         , int const& struct_token
                         , lists lst)
{
  si_link l = ssi_open_for_write(filename);
  ssi_write_newstruct (l, struct_token, lst);
  ssi_close_and_remove (l);
}

void ssi_write_newstruct ( std::string filename
                         , std::string const& struct_name
                         , lists lst)
{
  si_link l = ssi_open_for_write(filename);
  ssi_write_newstruct (l, struct_name, lst);
  ssi_close_and_remove (l);
}

void ssi_write_newstruct (si_link l, std::string const& struct_name, lists lst)
{
  int type;
  blackboxIsCmd (struct_name.c_str(), type); // TODO check for success
  ssi_write_newstruct (l, type, lst);
}

// NOTE, this is where the actual writing happens in all cases
void ssi_write_newstruct (si_link l, int type, lists lst)
{
  ssiInfo *d = static_cast<ssiInfo*> (l->data);
  blackbox *b = getBlackboxStuff (type); // TODO check for success
  fputs ("20 ", d->f_write);
  newstruct_serialize (b, lst, l);
}

/*
 * Why? Ensures in some edge cases that files on disk for that "same" struct is
 * directly comparable on byte level.  
 */
void double_ssi_write_newstruct ( std::string const& filename
                                , int const& in_token
                                , sleftv const& m
                                )
{
  std::string tmp_filename(filename + "_temp");
  ssi_write_newstruct ( tmp_filename
                      , in_token
                      , m
                      );

  std::string struct_name(getBlackboxName(in_token));
  lists l = ssi_read_newstruct(tmp_filename, struct_name);

  ssi_write_newstruct ( filename
                      , in_token
                      , l
                      );
  boost::filesystem::remove(tmp_filename);
  
}

lists ssi_read_newstruct(std::string const& file_name,
                        std::string const& struct_name)
{
  si_link l = ssi_open_for_read (file_name);
  lists in_lst = ssi_read_newstruct (l, struct_name);
  ssi_close_and_remove (l);

  return in_lst;
}

// The work is done here.
lists ssi_read_newstruct (si_link l, std::string const& struct_name)
{
  ssiInfo *d = static_cast<ssiInfo*> (l->data);
  int t = s_readint (d->f_read);
  if (t != 20)
  {
    std::string error_msg = "wrong token, expected 20 got "
      + std::to_string (t);
    throw std::runtime_error (error_msg);
  }
  /*int ignore =*/ s_readint (d->f_read);
  char* name = ssiReadString (d);
  if (struct_name.compare (name) != 0)
  {
    std::string error_msg = "wrong blackbox name, expected " + struct_name
      + " got " + std::string (name);
    throw std::runtime_error (error_msg);
  }
  int tok;
  blackboxIsCmd (name, tok);
  omFree(name);
  if (tok <= MAX_TOK)
  {
    std::string error_msg = "token " + std::to_string (tok)
      + " is not larger than MAX_TOX " + std::to_string (MAX_TOK);
    throw std::runtime_error (error_msg);
  }
  lists li;
  newstruct_deserialize (NULL, reinterpret_cast<void**> (&li), l);
  return li;
}

std::pair<int, lists> call_user_proc (std::string const& function_name,
  std::string const& needed_library, int in_type, lists in_lst)
{
  ScopedLeftv arg (in_type, lCopy (in_lst));
  return proc<lists> (symbol (needed_library, function_name), arg);
}

std::pair<int, lists> call_user_proc (std::string const& function_name,
  std::string const& needed_library, ScopedLeftv& u_arg)
{
  return proc<lists> (symbol (needed_library, function_name), u_arg);
}



namespace singular {

  void call (std::string const& command)
  {
    int err = iiAllStart
      (NULL, const_cast<char*> ((command + " return();").c_str()), BT_proc, 0);
      if (err) {
        errorreported = 0;
        throw std::runtime_error (
          "Singular returned an error at \"" + command + "\"");
      }
  }

  void call_and_discard (std::string const& command)
  {
    SPrintStart();
    call (command);
    char* result_ptr = SPrintEnd();
    omFree (result_ptr);
  }

  std::string get_result (std::string const& command)
  {
    SPrintStart();
    call (command);
    char* result_ptr = SPrintEnd();
    std::string result (result_ptr);
    omFree (result_ptr);
    if (result.size() > 0)
    {
      result.pop_back();
    }
    return result;
  }

  void register_struct(std::string struct_name, std::string struct_desc)
  {
    call_and_discard("newstruct(\"" +
                     struct_name +
                     "\", \"" +
                     struct_desc +
                     "\");");
  }



  void load_library
    ( std::string const& library_name
    , bool enforce_reload
    )
  {
    bool load = enforce_reload;
    if (!load) {
      char *plib = iiConvName(library_name.c_str());
      FHG_UTIL_FINALLY([=] { omFree((ADDRESS)plib); });
      idhdl pl = basePack->idroot->get(plib,0);
      load = !((pl!=NULL)
          && IDTYP(pl)==PACKAGE_CMD
          && IDPACKAGE(pl)->language == LANG_SINGULAR);
    }
    if (load){
        call_and_discard ("LIB \"" + library_name + "\";");
    }
  }


  void load_ssi_no_def( const std::string& symbol_name
      , const std::string& file)
  {
      call_and_discard(symbol_name + " = read(\"ssi:r " +
          file + "\");");
  }

  void load_ssi_no_def( const std::string& symbol_name
      , const singular_parallel::pnet_value& files)
  {
    if (files.type() == typeid(singular_parallel::pnet_list))
    {
      call_and_discard(symbol_name + " = list();");
      int i = 1;
      for ( const singular_parallel::pnet_value& elem
          : singular_parallel::as<singular_parallel::pnet_list>(files)
          )
      {
        load_ssi_no_def(symbol_name + "[" + std::to_string(i) + "]", elem);
        i++;
      }
    }
    /*
    else if (files.type() == typeid(pnet::type::value::structured_type))
    {
      const singular_parallel::pnet_file file = pnetc::type::file::from_value(files);
      call_and_discard(symbol_name + " = read(\"ssi:r " + file.path + "\");");
    }
    */
    else if (files.type() == typeid(std::string))
    {
      //call_and_discard(symbol_name + " = read(\"ssi:r "
      //    + singular_parallel::as<std::string>(files) + "\");");
      load_ssi_no_def(symbol_name, singular_parallel::as<std::string>(files));
    } else {
      throw std::runtime_error("Loading ssi files from nested pnet_lists "
          "containing other leaves than pnet_file or string is not supported.");
    }
  }


  void load_ssi
    ( const std::string& symbol_name
      , const std::string& file
    )
  {
    call_and_discard("def " + symbol_name + ";");
    load_ssi_no_def(symbol_name, file);
  }

  void load_ssi
    ( const std::string& symbol_name
      , const ::singular_parallel::pnet_value& files
    )
  {
    call_and_discard("def " + symbol_name + ";");
    load_ssi_no_def(symbol_name, files);
  }

  void load_ssi_if_not_defined
    ( const std::string& symbol_name
    , const ::singular_parallel::pnet_value& files
    )
  {
    if (!symbol_exists(symbol_name))
    {
      load_ssi(symbol_name, files);
    }

  }

  void write_ssi (const std::string& symbol_name, std::string const& file_name)
  {
    call_and_discard ("link l = \"ssi:w " + file_name + "\"; write(l,"
      + symbol_name + "); close(l); kill l;");
  }

  bool symbol_exists(const std::string& symbol_name)
  {
    return ggetid(symbol_name.c_str()) != NULL;
  }

  intvec* getIntvec(const std::string& symbol)
  {
    idhdl h = ggetid(symbol.c_str());
    if (IDTYP(h) == INTVEC_CMD)
    {
      return IDINTVEC(h);
    }
    else
    {
      throw std::runtime_error(
          "Symbol " + symbol + " does not represent an intvec.");
    }
  }

  lists getList(const std::string& symbol)
  {
    idhdl h = ggetid(symbol.c_str());
    if (IDTYP(h) != LIST_CMD)
    {
      throw std::runtime_error(
          "Symbol " + symbol + " does not represent a list.");
    }
    else
    {
      return IDLIST(h);
    }

  }

  int getInt(const std::string& symbol)
  {
    idhdl h = ggetid(symbol.c_str());
    if (IDTYP(h) != INT_CMD)
    {
      throw std::runtime_error(
          "Symbol " + symbol + " does not represent an int.");
    }
    else
    {
      return IDINT(h);
    }

  }

  void put(const std::string& symbol, lists list)
  {
    idhdl handle = enterid(symbol.c_str(), 1, LIST_CMD, &IDROOT, FALSE);
    IDLIST(handle) = list;
  }

  void put(const std::string& symbol, intvec* iv)
  {
    idhdl handle = enterid(symbol.c_str(), 1, INTVEC_CMD, &IDROOT, FALSE);
    IDINTVEC(handle) = iv;
  }

  template<typename T, typename... Args>
    void kill(const T& t, Args&&... args);


}



std::string get_struct_filename( std::string const& root
                               , std::string const& basename
                               , std::string const& postfix
                               , unsigned long const& id);


std::string get_struct_filename( std::string const& root
                               , std::string const& basename
                               , std::string const& postfix
                               , unsigned long const& id)
{
  return root + "/" + basename + "." + postfix + std::to_string(id);
}

std::string get_in_struct_filename( std::string const& root
                                  , std::string const& basename
                                  , unsigned long const& id)
{
  return get_struct_filename( root
                            , basename
                            , "i"
                            , id);
}

std::string get_out_struct_filename( std::string const& root
                                   , std::string const& basename
                                   , unsigned long const& id)
{
  return get_struct_filename( root
                            , basename
                            , "o"
                            , id);
}

std::string get_temp_struct_filename( std::string const& root
                                    , std::string const& basename
                                    , unsigned long const& id)
{
  return get_struct_filename( root
                            , basename
                            , "t"
                            , id);
}


void write_in_structs_to_file( lists const& struct_list
                             , std::string const& root
                             , std::string const& basename
                             , int const& in_token)
{
  write_in_structs_to_file_from_index( struct_list
                                     , 0
                                     , root
                                     ,basename
                                     , in_token);
}

void write_in_structs_to_file_from_index( lists const& struct_list
                                        , int const& index
                                        , std::string const& root
                                        , std::string const& basename
                                        , int const& in_token)
{
  for (std::size_t i = 0; i < (std::size_t)struct_list->nr + 1; i++) {
    double_ssi_write_newstruct ( get_in_struct_filename( root
                                                       , basename
                                                       , i + index)
                               , in_token
                               , struct_list->m[i]);
  }
}

void write_temp_structs_to_file( lists const& struct_list
                               , std::string const& root
                               , std::string const& basename
                               , int const& in_token)
{
  for (std::size_t i = 0; i < (std::size_t)struct_list->nr + 1; i++) {
    double_ssi_write_newstruct ( get_temp_struct_filename(root, basename, i)
                        , in_token
                        , struct_list->m[i]);
  }
}

std::string calculate_file_sha1_hash(std::ifstream & ifs)
{
  boost::uuids::detail::sha1 sha1;
  unsigned int hash[5];

  /* sha1 loop */
  char buf[1024];
  while(ifs.good()) {
    ifs.read(buf,sizeof(buf));
    sha1.process_bytes(buf,ifs.gcount());
  }

  if(!ifs.eof()) {
    throw std::runtime_error("not at eof\n");
  }

  sha1.get_digest(hash);
  std::stringstream strstr;
  strstr << hash_to_string(hash, (int)sizeof(hash)/sizeof(unsigned int));

  return strstr.str();
}

std::string calculate_file_sha1_hash(std::string const& disk_filename)
{
  std::ifstream ifs = open_file(disk_filename.c_str());
  std::string s_hash = calculate_file_sha1_hash(ifs);
  ifs.close();

  return s_hash;
}
