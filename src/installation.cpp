#include <installation.hpp>

#include <util-generic/executable_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <stdexcept>

namespace singular_parallel
{
  namespace
  {
    void check ( boost::filesystem::path const& path
               , bool okay
               , std::string const& message
               )
    {
      if (!okay)
      {
        throw std::logic_error
          ( ( boost::format ("%1% %2%: Installation incomplete!?")
            % path
            % message
            ).str()
          );
      }
    }

    void check_is_directory (boost::filesystem::path const& path)
    {
      check ( path
            , boost::filesystem::is_directory (path)
            , "is not a directory"
            );
    }
    void check_is_file (boost::filesystem::path const& path)
    {
      check ( path
            , boost::filesystem::exists (path)
            , "does not exist"
            );
      check ( path
            , boost::filesystem::is_regular_file (path)
            , "is not a regular file"
            );
    }

    //! \todo configure
    boost::filesystem::path gspc_home
      (boost::filesystem::path const& gspc_path)
    {
      return gspc_path;
    }
    boost::filesystem::path workflow_path
      (boost::filesystem::path const& installation_path)
    {
      return installation_path / "libexec" / "workflow";
    }
    boost::filesystem::path workflow_pfd_file
      (boost::filesystem::path const& installation_path)
    {
      // TODO: to sth. about several nets
      return workflow_path (installation_path) / "parallel_allpfd.pnet";
    }
  }

  installation::installation()
    : installation
        (SP_INSTALL_PATH)
  {}

  installation::installation (boost::filesystem::path const& ip)
    : installation ( ip
                   , fhg::util::executable_path
                       (gspc::set_gspc_home).parent_path().parent_path()
                   )
  {}

  installation::installation (boost::filesystem::path const& ip,
    boost::filesystem::path const& gp)
    : _path (ip), _gspc_path (gp)
  {
    //! \todo more detailed tests!?
    check_is_directory (gspc_home (_gspc_path));
    check_is_directory (workflow_path (_path));
    check_is_file (workflow_pfd());
  }


  boost::filesystem::path installation::workflow_pfd() const
  {
    return workflow_pfd_file (_path);
  }

  boost::filesystem::path installation::workflow_dir() const
  {
    return workflow_path (_path);
  }
  gspc::installation installation::gspc_installation
    (boost::program_options::variables_map& vm) const
  {
    gspc::set_gspc_home (vm, gspc_home (_gspc_path));
    gspc::set_application_search_path (vm, workflow_path (_path));

    return {vm};
  }
}
