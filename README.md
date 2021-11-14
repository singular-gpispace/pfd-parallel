# PFD - Partial Fraction Decomposition

We provide a massively parallel framework for partial fraction decomposition of
rational functions based on the [Singular/GPI-Space framework](https://www.mathematik.uni-kl.de/~boehm/singulargpispace/).

Our implementation is based on the approach described in the paper

Janko, Boehm, Marcel Wittmann, Zihao Wu, Yingxuan Xu, and Yang Zhang:
IBP reduction coefficients made simple, JHEP 12 (2020) 054,

which has been implemened in Singular in the library
[pfd.lib](https://github.com/Singular/Singular/blob/spielwiese/Singular/LIB/pfd.lib).

Although applicable in general, it is aimed at the partial fraction
decomposition of integration-by-parts coefficients in high energy physics.

Most of the parallelization code is an adapted version of the
[wait-all-first](https://github.com/singular-gpispace/wait-all-first)
repository, implemented primarily by Lukas Ristau.

This project provides a function for applying the partial fraction decoposition
function to a matrix of rational functions.

To get this project up and running, you need to compile Singular, GPI-Space,
some of their dependencies and the project code itself.

For the various dependencies, it is recommended to create a file that exports
the various install locations as environment variables. For this purpose, the
following command may be run at a convenient location, after which the resulting
file should be edited to specify two directory roots, one for purposes of
compilation and one for installation of the code. While the first should
typically be a fast local file system, the second must be accessible from all
computation nodes to be used for running the system.

```bash
cat > env_vars_pfd.txt << "EOF"
export SOFTWARE_ROOT=<software-root>
# Some fast location in local system for hosting build directories,
# for example, something like /tmpbig/$USER/pfd or just $HOME/pfd if the user has a
# fast home directory.

export INSTALL_ROOT=<install-root>
# The install root is recommended to be some network (nfs) mountpoint, where
# each node of the cluster should be able to read from, for example
# something like /scratch/$USER/pfd

export COMPILE_ROOT=$SOFTWARE_ROOT
# Optionally, this might be set to something like /dev/shm/$USER/pfd that
# stores files purely in memory, thus the contents of this
# location will be lost after reboot.  It can speed up the computation times, as
# all disk io becomes memory io.

# GPI-Space dependencies:
export BOOST_ROOT=$INSTALL_ROOT/boost
export Libssh2_ROOT=$INSTALL_ROOT/libssh
export Libssh2_BUILD_DIR=$COMPILE_ROOT/libssh/build
export LD_LIBRARY_PATH="${Libssh2_ROOT}/lib"${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
export GASPI_ROOT=$INSTALL_ROOT/gpi2
export cpu_arch=$(getconf LONG_BIT)
export PKG_CONFIG_PATH="${GASPI_ROOT}/lib${cpu_arch}/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}
export PKG_CONFIG_PATH="${Libssh2_ROOT}/lib/pkgconfig:${Libssh2_ROOT}/lib64/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}


# GPI-Space:
export GPI_ROOT_DIR=$SOFTWARE_ROOT
export GPISPACE_REPO=$GPI_ROOT_DIR/gpispace/gpispace
export GPISPACE_BUILD_DIR=$COMPILE_ROOT/gpispace/build
export GPISPACE_INSTALL_DIR=$INSTALL_ROOT/gpispace
export GPISpace_ROOT=$GPISPACE_INSTALL_DIR # expected by cmake
export GSPC_HOME=$GPISPACE_INSTALL_DIR # Set mostly for legacy reasons
export SHARED_DIRECTORY_FOR_TESTS=$GPISPACE_BUILD_DIR/tests

# Singular:
export SING_ROOT=$SOFTWARE_ROOT/Singular
export DEP_LIBS=$INSTALL_ROOT/sing_dep_libs
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DEP_LIBS/lib
export SINGULAR_INSTALL_DIR=$INSTALL_ROOT/Singular
export SINGULAR_BUILD_DIR=$COMPILE_ROOT/Singular/build

# PFD:
export PFD_ROOT=$SOFTWARE_ROOT/pfd
export PFD_REPO=$PFD_ROOT/pfd
export PFD_INSTALL_DIR=$INSTALL_ROOT/pfd
export PFD_BUILD_DIR=$PFD_ROOT/build
export PFD_INPUT_DIR=$PFD_ROOT/input
export PFD_OUTPUT_DIR=$PFD_ROOT/output
EOF

```

As it is currently structured, the user needs only fill in the `<software-root>`
and the `<install-root>`, then all other veriables are set relative to these.
The above structure can of course be altered to suite the compiler's setup,
as long as the scripts in this REAME are altered accordingly, since they assume
the directory structure.
Once all the locations have been set correctly, the variables can easily be
exported with the following call:
```bash
source env_vars_pfd.txt

```

Ensure that the compile and install roots exist:
```bash
mkdir -p $SOFTWARE_ROOT
mkdir -p $INSTALL_ROOT
mkdir -p $COMPILE_ROOT

```
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

Ensure that the `GPI_ROOT_DIR` exists:
```bash
mkdir -p $GPI_ROOT_DIR

```

### Boost

| Website | Supported Versions |
| :-: | :-: |
| [Boost](https://boost.org) | >= 1.61, <= 1.63 |

Note, that Boost 1.61 is not compatible with OpenSSL >= 1.1, so it is
recommended to use Boost 1.63, as follows:

```bash
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
      -B $Libssh2_BUILD_DIR                                       \
      -S libssh2

cmake --build $Libssh2_BUILD_DIR                                  \
      --target install                                            \
      -j $(nproc)

```

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
cd $GPI_ROOT_DIR
mkdir gpi2 && cd gpi2

gpi2_version=1.3.2                                                \
 && git clone                                                     \
        --depth 1                                                 \
        --branch v${gpi2_version}                                 \
        https://github.com/cc-hpc-itwm/GPI-2.git                  \
        GPI-2                                                     \
 && cd GPI-2                                                      \
 && grep "^CC\s*=\s*gcc$" . -lR                                   \
    | xargs sed -i'' -e '/^CC\s*=\s*gcc$/d'                       \
 && ./install.sh -p "${GASPI_ROOT}"                               \
                 --with-fortran=false                             \
                 --with-ethernet

```
> ---
> **NOTE:**
>
> Note that `<install-prefix>` should be set to the correct path in the
> script above.
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
git clone                                                         \
    --depth 1                                                     \
    --branch v21.03                                               \
    https://github.com/cc-hpc-itwm/gpispace.git                   \
    gpispace

```

Should you want to build the tests, run the following command.  Note, the tests
significantly increase build time, so an experienced user who has built
gpi-space on a given machine before might opt not to run to define the
build_tests variable.

```bash
export build_tests="-DBUILD_TESTING=on
-DSHARED_DIRECTORY_FOR_TESTS=$SHARED_DIRECTORY_FOR_TESTS"
mkdir -p $SHARED_DIRECTORY_FOR_TESTS
```

To build GPI-Space, run
```bash
mkdir -p "${GPISPACE_BUILD_DIR}"

cmake -D CMAKE_INSTALL_PREFIX=${GPISPACE_INSTALL_DIR}             \
      -D CMAKE_BUILD_TYPE=Release                                 \
      -B ${GPISPACE_BUILD_DIR}                                    \
      -S ${GPISPACE_REPO}                                         \
      ${build_tests:-}

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

> ---
> **NOTE:**
>
> GPI-Space requires a working SSH environment with a password-less
> SSH-key when using the SSH RIF strategy, the default for most
> applications.
>
> By default, `${HOME}/.ssh/id_rsa` is used for authentication. If no
> such key exists,
>
> ```bash
> ssh-keygen -t rsa -b 4096 -N '' -f "${HOME}/.ssh/id_rsa"
> ssh-copy-id -f -i "${HOME}/.ssh/id_rsa" "${HOSTNAME}"
> ```
> can be used to create and register one.
>
> ---

The following is a simple self test, that should be sufficient for most users.
```bash
# to test with multiple nodes, set GSPC_NODEFILE_FOR_TESTS
#   Slurm: export GSPC_NODEFILE_FOR_TESTS="$(generate_pbs_nodefile)"
#   PBS/Torque: export GSPC_NODEFILE_FOR_TESTS="${PBS_NODEFILE}"
# and SWH_INSTALL_DIR:
#   export SWH_INSTALL_DIR=<a-shared-directory-visible-on-all-nodes>

"${GPISpace_ROOT}/share/gspc/example/stochastic_with_heureka/selftest"
```

If GPI-Space has been built with testing enabled, then `ctest` can be
used to execute the unit- and system tests. It can be ommitted, but is
recommended for first time installations.

```bash
cd "${GPISPACE_BUILD_DIR}"

export GPISPACE_TEST_DIR=$SHARED_DIRECTORY_FOR_TESTS # any empty test directoy should be good

hostname > nodefile
export GSPC_NODEFILE_FOR_TESTS="${PWD}/nodefile"
# or to test in a cluster allocation:
# Slurm: export GSPC_NODEFILE_FOR_TESTS="$(generate_pbs_nodefile)"
# PBS/Torque: export GSPC_NODEFILE_FOR_TESTS="${PBS_NODEFILE}"

ctest --output-on-failure                                         \
      -j $(nproc)

```

Some of these tests take a long time (811 seconds on Intel i7), and there are
294 tests in the suite at the time of writing this document.

## Singular

It is recommended to install the current version of Singular, which will be required by our
framework. The version of Singular found in package manager does *not* generally
work with the PFD project.

Besides flint, Singular has various more standard dependencies, which are
usually available through the package manager of your distribution. Feel free to
refer to the
[step-by-step instructions to build Singular](https://github.com/Singular/Singular/wiki/Step-by-Step-Installation-Instructions-for-Singular)
for more details, taking note of the tools necessary to compile Singular under
point 1.b.

This document gives a thorough guide for building the various dependencies and
then finally Singular itself, all the while assuming the user does *not* have
`sudo`/`root` privileges. This guide assumes that `mpfr` and `gmp` are installed
by the package manager to `/usr`

Start by choosing a location where Singular can be cloned. This will be
indicated by `$SING_ROOT` and compile the various dependencies

```bash
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
mkdir -p $COMPILE_ROOT/4ti2/build

cd $SING_ROOT
mkdir 4ti2 && cd 4ti2
wget http://www.4ti2.de/version_1.6/4ti2-1.6.tar.gz
tar xvfz 4ti2-1.6.tar.gz

pushd $COMPILE_ROOT/4ti2/build

$SING_ROOT/4ti2/4ti2-1.6/configure --prefix=$DEP_LIBS
make -j $(nproc)
make install

popd

```

### cddlib

```bash
mkdir -p $COMPILE_ROOT/cddlib/build

cd $SING_ROOT
mkdir cddlib && cd cddlib
wget https://github.com/cddlib/cddlib/releases/download/0.94j/cddlib-0.94j.tar.gz
tar -xvf cddlib-0.94j.tar.gz

pushd $COMPILE_ROOT/cddlib/build

$SING_ROOT/cddlib/cddlib-0.94j/configure --prefix=$DEP_LIBS
make -j $(nproc)
make install

popd

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
git clone                                                         \
    --depth 1                                                     \
    https://github.com/Singular/Singular.git                      \
    Sources

cd Sources
./autogen.sh

mkdir -p $SINGULAR_BUILD_DIR && pushd $SINGULAR_BUILD_DIR

CPPFLAGS="-I$DEP_LIBS/include"                                    \
LDFLAGS="-L$DEP_LIBS/lib"                                         \
${SING_ROOT}/Sources/configure                                    \
    --prefix=${SINGULAR_INSTALL_DIR}                              \
    --with-flint=$DEP_LIBS                                        \
    --with-ntl=$DEP_LIBS                                          \
    --enable-gfanlib
make -j $(nproc)
make install

popd

```
## Compile PFD
The PFD project can now be compiled and installed.

The following environment variables must be set:
- `${GPISPACE_REPO}` The path to the repository cloned from Github. This is
  needed for some cmake scripts, amongst other reasons.
- `${GPISPACE_INSTALL_DIR}` The install prefix used when compiling and
  installing gpi-space above.
- `${SINGULAR_INSTALL_DIR}` The install prefix used when compiling and
  installing Singular above.
- `${PFD_REPO}` The root of the cloned PFD project.
- `${PFD_BUILD_DIR}` The path of the build directory.  It is recommended to
  build in a separate directory to the source code, preferably starting with an
  empty  build directory.
- `${PFD_INSTALL_DIR}` The path to where the PFD project should be installed.

```bash
mkdir -p $PFD_ROOT && cd $PFD_ROOT

git clone                                                         \
    --depth 1                                                     \
    https://github.com/singular-gpispace/PFD.git                  \
    pfd

mkdir -p $PFD_BUILD_DIR
cmake -D CMAKE_INSTALL_PREFIX=$PFD_INSTALL_DIR   \
      -D CMAKE_BUILD_TYPE=Release                \
      -D GSPC_HOME=$GPISPACE_INSTALL_DIR         \
      -D GPISPACE_REPO=$GPISPACE_REPO            \
      -D SINGULAR_HOME=$SINGULAR_INSTALL_DIR     \
      -D FLINT_HOME=$DEP_LIBS                    \
      -B ${PFD_BUILD_DIR}                        \
      -S ${PFD_REPO}

cmake --build ${PFD_BUILD_DIR}                   \
      --target install                           \
      -j $(nproc)

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

An example script `test_parallel_pfd.sing` for a 4 by 4 matrix might be

```bash
mkdir -p $PFD_ROOT/tmpdir
echo $(hostname) > $PFD_ROOT/nodefile

cat > test_parallel_pfd.sing.temp << "EOF"
LIB "pfd_gspc.lib";

configToken gc = configure_gspc();

gc.options.tmpdir = "$PFD_ROOT/tmpdir";
gc.options.nodefile = "$PFD_ROOT/nodefile";
gc.options.procspernode = 8;
gc.options.loghost = "$(hostname)";
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
pfd_internal_parallel( "fraction"
                     , l
                     , "$PFD_INPUT_DIR"
                     , gc
                     , "$PFD_OUTPUT_DIR" // optional, only necessary if diff from input dir
                     );
exit;
EOF

```
We want to expand the environment variables in the script above, so create a
hacky script for this purpose:
```bash
cat > shell_expand_script.sh << "EOF"
echo 'cat <<END_OF_TEXT' >  temp.sh
cat "$1"                 >> temp.sh
echo 'END_OF_TEXT'       >> temp.sh
bash temp.sh >> "$2"
rm temp.sh
EOF

chmod a+x shell_expand_script.sh
./shell_expand_script.sh test_parallel_pfd.sing.temp test_parallel_pfd.sing

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
mkdir -p $PFD_INPUT_DIR
mkdir -p $PFD_OUTPUT_DIR
pushd $PFD_INPUT_DIR
for r in {1..4}
do
  for c in {1..4}
  do
    echo "x/(x*(x+1))" > fraction_"$r"_"$c".txt
  done
done
popd

```

Finally, the test may be started with
```bash
cat > run_pfd_example.sh << "EOF"
SINGULARPATH="$PFD_INSTALL_DIR/LIB"                               \
        $SINGULAR_INSTALL_DIR/bin/Singular                        \
        test_parallel_pfd.sing
EOF
chmod a+x run_pfd_example.sh
./run_pfd_example.sh

```



## Appendix: Standard packages required to build the framework

Assuming that we are installing on a Ubuntu system (analogous packages exist in other distributions), we give installation instructions for standard packages which are required by the framework and may not be included in your of-the-shelf installation.

Note that the following requires root privileges. If you do not have root access, ask your administator to install these packages. You may want to check with `dpkg -l <package name>` whether the package is installed already.

* Version control system Git used for downloading sources:
  ```bash
  sudo apt-get install git
  ```

* Tools necessary for compiling the packages:
  ```bash
  sudo apt-get install build-essential
  sudo apt-get install autoconf
  sudo apt-get install autogen
  sudo apt-get install libtool
  sudo apt-get install libreadline6-dev
  sudo apt-get install libglpk-dev
  sudo apt-get install cmake
  sudo apt-get install gawk
   ```

  Or everything in one command:
  ```bash
  sudo apt-get install build-essential autoconf autogen libtool libreadline6-dev libglpk-dev cmake gawk
  ```

* Scientific libraries used by Singular:
  ```bash
  sudo apt-get install libgmp-dev
  sudo apt-get install libmpfr-dev
  sudo apt-get install libcdd-dev
  sudo apt-get install libntl-dev
  ```

  Or everything in one command:
  ```bash
  sudo apt-get install libgmp-dev libmpfr-dev libcdd-dev libntl-dev
  ```
  Note, in the above guide, we only assume libmpfr and libgmp is installed, and
  libcdd and libntl is built locally from sources.

* Library required by to build libssh:
  ```bash
  sudo apt-get install libssl-dev
  ```

* Libraries required by GPI-Space
  ```bash
  sudo apt-get install openssh-server
  sudo apt-get install hwloc
  sudo apt-get install libhwloc-dev
  sudo apt-get install libudev-dev
  sudo apt-get install qt5-default
  sudo apt-get install chrpath
  ```

  Or everything in one command:
  ```bash
  sudo apt-get install openssh-server hwloc libhwloc-dev libudev-dev qt5-default chrpath
  ```
