# PFD - Partial Fraction Decomposition

A partial Fraction Decomposition Framework for Singular has been implemented by
Marcel Wittman, at the Technical University Kaiserslautern (TU Kaiserslautern).
Most of the code is an adapted version of the
[wait-all-first](https://github.com/singular-gpispace/wait-all-first)
repository, implemented primarily by Lukas Ristau.

This project provides allows for applying the partial fraction decoposition
function in the massively parallel system GPI-space.

To get this project up and running, you need to compile GPI-Space and Singular.

For the various dependencies, it is recommended to create a file that exports
the various install locations as environment variables. For this purpose, the
following command may be run at a convenient location, after which the resultant
file should be edited, with the appropriate paths set for the various install
locations.

```bash
cat > env_vars_pfd.txt << "EOF"
export BOOST_ROOT=<boost-install-prefix>
export Libssh2_ROOT=<libssh2-install-prefix>
export LD_LIBRARY_PATH="${Libssh2_ROOT}/lib"${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
export GASPI_ROOT=<gpi-install-prefix>
export cpu_arch=$(getconf LONG_BIT)
export PKG_CONFIG_PATH="${GASPI_ROOT}/lib${cpu_arch}/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}
export GPI_ROOT_DIR=<gpi-root-dir>
export GPISPACE_REPO=<gpi-space-repo>
export GPISPACE_BUILD_DIR=<gpi-space-build-dir>
export GPISPACE_INSTALL_DIR=<gpi-space-install-prefix>
export GPISPACE_TEST_DIR=<test-directory>
export GSPC_NODEFILE_FOR_TESTS=<path-to-nodefile> # suggested: $HOME/nodefile
export SING_ROOT=<dir-for-sing-related-source-code>
export DEP_LIBS=<sing-dependencies-install-prefix>
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DEP_LIBS/lib
export SINGULAR_INSTALL_DIR=<singular-install-prefix>
export SINGULAR_BUILD_DIR=<singular-build-dir>
export PFD_REPO=<pfd-repo-clone>
export PFD_INSTALL_DIR=<pfd-install-prefix>
export PFD_BUILD_DIR=<pfd-build-dir>
EOF
```

The meaning of the exact paths are described in the course of the README.
Once all the locations have been set correctly, the variables can easily be
exported with the following call:
```bash
source env_vars_pfd.txt
```
This helps eliminate the chance of typos.

## GPI-Space

GPI-Space targets and has been successfully used on x86-64 Linux
systems.
Other architectures are not guaranteed to work properly at this point.

The virtual memory layer can be backed with either Ethernet,
Infiniband or BeeOND.

GPI-Space supports multiple Linux distributions:
* Centos 6
* Centos 7
* Centos 8
* Ubuntu 18.04 LTS
* Ubuntu 20.04 LTS

In this guide, all gpi-space related code will be placed in some `GPI_ROOT_DIR`.

### Boost

| Website | Supported Versions |
| :-: | :-: |
| [Boost](https://boost.org) | >= 1.61, <= 1.63 |

Note, that Boost 1.61 is not compatible with OpenSSL >= 1.1, so it is
recommended to use Boost 1.63, as follows:

```bash
export BOOST_ROOT=<install-prefix>

boost_version=1.63.0

cd $GPI_ROOT_DIR
mkdir boost && cd boost

git clone                                                         \
    --jobs $(nproc)                                               \
    --depth 1                                                     \
    --shallow-submodules                                          \
    --recursive                                                   \
    --branch boost-${boost_version}                               \
    https://github.com/boostorg/boost.git                         \
    boost

cd boost
./bootstrap.sh --prefix="${BOOST_ROOT}"
./b2                                                              \
  -j $(nproc)                                                     \
  headers
./b2                                                              \
  cflags="-fPIC -fno-gnu-unique"                                  \
  cxxflags="-fPIC -fno-gnu-unique"                                \
  link=static                                                     \
  variant=release                                                 \
  install
```
> ---
> **NOTE:**
>
> Take note to replace `<install-prefix>` with the appropriate path on your
> system where you need boost installed. Also, if you are using
> `env_vars_pfs.txt`, the export step can be omitted.
>
> ---

### libssh2

| Website | Supported Versions |
| :-: | :-: |
| [libssh2](https://www.libssh2.org/) | >= 1.7 |

`libssh2`  is not built with the OpenSSL backend on all systems. Additionally,
some versions available via package manager might not be compatible with
OpenSSH's default settings. For those reasons, it is highly recommended to build
`libssh2` 1.9 from scratch. Doing so is however straightforward thanks to CMake.
As additional dependencies `OpenSSL` and `Zlib` are required (for this any
package manager version should be sufficient). Also, unless `Libssh2_ROOT` is
set to `/usr`, the `LD_LIBRARY_PATH` needs to be set (as in lines 2 and 3 below)
in order for applications to find the correct one.

> ---
> **WARNING:**
> * libssh2 1.7 is not compatible with **OpenSSL >= 1.1**.
> * libssh2 <= 1.8 is incompatible with the new default SSH-key format in **OpenSSH >= 7.8**.
>
> ---

```bash
export Libssh2_ROOT=<install-prefix>
export LD_LIBRARY_PATH="${Libssh2_ROOT}/lib"${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}

cd $GPI_ROOT_DIR
mkdir libssh && cd libssh

libssh2_version=1.9.0

git clone --jobs $(nproc)                                         \
          --depth 1                                               \
          --shallow-submodules                                    \
          --recursive                                             \
          --branch libssh2-${libssh2_version}                     \
          https://github.com/libssh2/libssh2.git                  \
          libssh2

cmake -D CRYPTO_BACKEND=OpenSSL                                   \
      -D CMAKE_BUILD_TYPE=Release                                 \
      -D CMAKE_INSTALL_PREFIX="${Libssh2_ROOT}"                   \
      -D ENABLE_ZLIB_COMPRESSION=ON                               \
      -D BUILD_SHARED_LIBS=ON                                     \
      -B libssh2/build                                            \
      -S libssh2

cmake --build libssh2/build                                       \
      --target install                                            \
      -j $(nproc)

```
Note that `<install-prefix>` should be set to the correct path in the
script above.

### GPI-2

| Website | Supported Versions |
| :-: | :-: |
| [GPI-2](http://www.gpi-site.com) | 1.3.2 |

If Infiniband support is required, the `--with-ethernet` option can be omitted.

> ---
> **NOTE:**
>
> Compiling GPI2 requires gawk. Please install this with a package manager, if
> not already present on the system.
>
> ---

```bash
export cpu_arch=$(getconf LONG_BIT)
export GASPI_ROOT=<install-prefix>
export PKG_CONFIG_PATH="${GASPI_ROOT}/lib${cpu_arch}/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}

cd $GPI_ROOT_DIR
mkdir gpi2 && cd gpi2

gpi2_version=1.3.2                                                            \
 && git clone                                                                 \
        --depth 1                                                             \
        --branch v${gpi2_version}                                             \
        https://github.com/cc-hpc-itwm/GPI-2.git                              \
        GPI-2                                                                 \
 && cd GPI-2                                                                  \
 && grep "^CC\s*=\s*gcc$" . -lR                                               \
    | xargs sed -i'' -e '/^CC\s*=\s*gcc$/d'                                   \
 && ./install.sh -p "${GASPI_ROOT}"                                           \
                 --with-fortran=false                                         \
                 --with-ethernet
```
> ---
> **NOTE:**
>
> Note that `<install-prefix>` should be set to the correct path in the
> script above. If the `env_vars_pfd.txt` file is being used, the export steps
> may be omitted.
>
> ---

### GPI-Space

The code listings in this document assume

- `${GPISPACE_REPO}` to be the directory storing the GPI-Space
  sources.
- `${GPISPACE_BUILD_DIR}` to be an empty directory to be used for
  building GPI-Space.
- `${GPISPACE_INSTALL_DIR}` to be a directory to install GPI-Space
  to. It is suggested to use a previously empty directory on a shared
  filesystem.
- `${GPISPACE_TEST_DIR}` to be an empty directory on a shared
  filesystem, which used when running the system tests.

Start by cloning gpi-space:

```bash
cd $GPI_ROOT_DIR
mkdir gpispace && cd gpispace
git clone git@github.com:cc-hpc-itwm/gpispace.git
```

> ---
> **NOTE:**
>
> Until version 21.03 is realeased, the following patch is required:
>
> ---
```bash
cd $GPISPACE_REPO
sed -i 's/INSTALL_RPATH_USE_LINK_PATH false/INSTALL_RPATH_USE_LINK_PATH ${INSTALL_DO_NOT_BUNDLE}/' cmake/add_macros.cmake
```
> ---
>
> After version 21.03 and above, this `sed` step can be omitted.
>
> ---

```bash
mkdir -p "${GPISPACE_BUILD_DIR}" && cd "${GPISPACE_BUILD_DIR}"

cmake -C ${GPISPACE_REPO}/config.cmake                            \
      -D CMAKE_INSTALL_PREFIX=${GPISPACE_INSTALL_DIR}             \
      -B ${GPISPACE_BUILD_DIR}                                    \
      -S ${GPISPACE_REPO}

cmake --build ${GPISPACE_BUILD_DIR}                               \
      --target install                                            \
      -j $(nproc)
```

> ---
> **NOTE:**
>
> GPI-Space requires a working SSH environment with a password-less
> SSH-key when using the SSH RIF strategy. To ensure this, make sure when
> generating your ssh keypair to leave the password field empty.
>
> ---

### Test GPI-Space
This step may be omitted, but is recommended, especially for first time
installations.

```bash
cd "${GPISPACE_BUILD_DIR}"

hostname > nodefile
export GSPC_NODEFILE_FOR_TESTS="${PWD}/nodefile"
# or to test in a cluster allocation:
# Slurm: export GSPC_NODEFILE_FOR_TESTS="$(generate_pbs_nodefile)"
# PBS/Torque: export GSPC_NODEFILE_FOR_TESTS="${PBS_NODEFILE}"

ctest --output-on-failure                                         \
      --tests-regex share_selftest
```

Some of these tests take a long time, and there are 286 tests in the suite at
the time of writing this document.

## Singular

It is recommended to install the current version of Singular, which will be required by our
framework. The version of Singular found in package manager does *not* generally
work with the PFD project.

Note, various tools are required to be present on the system:

TODO:

Besides flint, Singular has various more standard dependencies, which are
usually available through the package manager of your distribution. Feel free to
refer to the
[step-by-step instructions to build Singular](https://github.com/Singular/Singular/wiki/Step-by-Step-Installation-Instructions-for-Singular)
for more details.

This document gives a thorough guide for building the various dependencies and
then finally Singular itself, all the while assuming the user does *not* have
`sudo`/`root` privileges. This guide assumes that `mpfr` and `gmp` are installed
by the package manager to `/usr`

Start by choosing a location where Singular can be cloned. This will be
indicated by `$SING_ROOT` and compile the various dependencies

```bash
export SING_ROOT=<sing-dependencies_root_dir>
mkdir -p $SING_ROOT
```

### flint
The official guides for singular clones the latest development branch of flint.
As flint is being actively developed and the APIs changed quite often, this has
led to issues in the past.  Therefore, it is rather recommended that a release
version be downloaded and compiled instead.  The newest release with which
Singular can be built (at the time of writing) is version 2.6.3.

```bash
cd $SING_ROOT
mkdir flint && cd flint
wget http://www.flintlib.org/flint-2.6.3.tar.gz
tar -xvf flint-2.6.3.tar.gz
cd flint-2.6.3
./configure --with-gmp=/usr --prefix=$DEP_LIBS --with-mpfr=/usr
make -j $(nproc)
make install
```

### 4ti2
```bash
cd $SING_ROOT
mkdir 4ti2 && cd 4ti2
wget http://www.4ti2.de/version_1.6/4ti2-1.6.tar.gz
tar xvfz 4ti2-1.6.tar.gz
cd 4ti2-1.6
./configure --prefix=$DEP_LIBS
make -j $(nproc)
make install
```

### cddlib

```bash
cd $SING_ROOT
mkdir cddlib && cd cddlib
wget https://github.com/cddlib/cddlib/releases/download/0.94j/cddlib-0.94j.tar.gz
tar -xvf cddlib-0.94j.tar.gz
cd cddlib-0.94j
./configure --prefix=$DEP_LIBS
make -j $(nproc)
make install
```

### ntl

```bash
cd $SING_ROOT
mkdir ntl && cd ntl
wget https://libntl.org/ntl-11.4.3.tar.gz
tar -xvf ntl-11.4.3.tar.gz
cd ntl-11.4.3/src # note the extra src
./configure PREFIX=$DEP_LIBS CXXFLAGS=-fPIC #notice PREFIX and CXXFLAGS is capitalized without dashes
make -j $(nproc)
make install
```

### Compile Singular

Singular may now be compiled against the libraries compiled and installed above.

```bash
cd $SING_ROOT
git clone                                                \
    --depth 1                                            \
    git@github.com:Singular/Singular.git                 \
    Sources

cd Sources
./autogen.sh

mkdir -p $SINGULAR_BUILD_DIR && cd $SINGULAR_BUILD_DIR

CPPFLAGS="-I/home/murray/fraunhofer/prog/tmp/include" \
LDFLAGS="-L/home/murray/fraunhofer/prog/tmp/lib" \
${SING_ROOT}/Sources/configure \
    --prefix=${SINGULAR_INSTALL_DIR} \
    --with-flint=$DEP_LIBS \
    --with-ntl=$DEP_LIBS \
    --enable-gfanlib
make -j $(nproc)
make install
```
## Compile PFD
The PFD project can now be compiled and installed.

The following environment variables must be set:
- `${GPISPACE_REPO}` as path to the repository cloned from Github. This is
  needed for some cmake scripts, amongst other reasons.
- `${GPISPACE_INSTALL_DIR}` The install prefix used when compiling and installing
  gpi-space above.
- `${SINGULAR_INSTALL_DIR}` The install prefix used when compiling and installing
  Singular above.
- `${PFD_REPO}` The root of the cloned PFD project.
- `${PFD_BUILD_DIR}` The path of the build directory.  It is recommended to build in
  a separate directory to the source code, preferably starting with an empty
  build directory.
- `${PFD_INSTALL_DIR}` The path to where the PFD project should be installed.

```bash
mkdir -p $PFD_BUILD_DIR && cd $PFD_BUILD_DIR
cmake -DCMAKE_INSTALL_PREFIX=$PFD_INSTALL_DIR   \
      -DCMAKE_BUILD_TYPE=Release                \
      -DGSPC_HOME=$GPISPACE_INSTALL_DIR         \
      -DALLOW_ANY_GPISPACE_VERSION=true         \
      -DGPISPACE_REPO=$GPISPACE_REPO            \
      -DSINGULAR_HOME=$SINGULAR_INSTALL_DIR     \
      $PFD_REPO

make -j $(nproc)
make -j $(nproc) install
```
## Example to run PFD
To run an example, we need a Singular script that loads the `pfd_gspc.lib`
library. A gpi-space configure token needs to be prepared, with some important
configuration:  The path of a temporary directory, where files will be stored
during the computation and handling of the various files at runtime, the
location of a nodefile, containing a location on then network where gpi-space is
installed, the number of processes each node should run in parallel, the address
of a running instance of the gpi-space monitoring tool, as well as the port on
which the monitoring tool is listening (Note, the program can be run without the
monitoring tool, in which case the last two options should remain unset). Next,
the ring in which the rational function's numerator and denominator is found is
declared.  The input of the system is in the form of files, identified by the
row and column in a matrix where it must be found, in the form
`<basename>_<row>_<col>.(txt|ssi)`, where the suffix is txt if the file is in
plain text format, and ssi if the input files are in this binary format
implemented by singular. To specify the input files to be calculated, put the
coordinates of the matrix entries to be calculated in a list of lists.

Finally, all this is provided to the `parallel_pfd` function as arguments,
preferably with the optional argument for the path to where the input files are
found.  The user may also provide in a separate argument the path of where the
output files should be written.

An example script `test_pfd.sing` for a 4 by 4 matrix might be

```cpp
LIB "pfd_gspc.lib";

configToken gc = configure_gspc();

gc.options.tmpdir = "<path-to-tmpdir>";
gc.options.nodefile = "<path-to-nodefile>";
gc.options.procspernode = 8;
gc.options.loghost = "<hostname>";
gc.options.logport = 6439;

ring r = 0, x, lp;

list l = list( list(1, 1)
             , list(1, 2)
             , list(1, 3)
             , list(1, 4)
             , list(2, 1)
             , list(2, 2)
             , list(2, 3)
             , list(2, 4)
             , list(3, 1)
             , list(3, 2)
             , list(3, 3)
             , list(3, 4)
             , list(4, 1)
             , list(4, 2)
             , list(4, 3)
             , list(4, 4)
             );
parallel_pfd( "fraction"
            , l
            , gc
            , "<path-to-input-files>"
            , "<path-to-output-files>");
exit;
```
Next, if you wish to start a monitor, this may be done as follows:
```bash
cat > start_monitor.sh << "EOF"
#!/usr/bin/bash

set -euo pipefail

# raster or native (native for X forwarding)
QT_DEBUG_PLUGINS=0                                                \
        QT_GRAPHICSSYSTEM=native                                  \
        $PFD_INSTALL_DIR/libexec/bundle/gpispace/bin/gspc-monitor \
        --port 6439 &

EOF
chmod a+x start_monitor.sh
./start_monitor.sh
```
Ensure that the `--port` number matches the one set in the singular script.
Also, if this is run over ssh on a remote machine, make sure that x forwarding
is enabled.

Create the input files:
```bash
pushd <input-dir>
for r in {1..4}
do
  for c in {1..4}
  do
    echo "x/(x*(x+1))" >> fraction_"$r"_"$c".txt
  done
done
popd
```

Finally, the test may be started with
```bash
SINGULARPATH="$PFD_INSTALL_DIR/LIB"                           \
        $SINGULAR_INSTALL_DIR/bin/Singular                    \
        test_pfd.sing
```
