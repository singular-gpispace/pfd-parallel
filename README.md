# PFD - Partial Fraction Decomposition

A partial Fraction Decomposition Framework for Singular has been implemented by
Marcel Wittman, at the Technical University Kaiserslautern (TU Kaiserslautern).
This project provides allows for applying this function in a massively parallel
system using GPI-space.

To get this project up and running, you need to compile GPI-Space and Singular.

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

For the various dependencies, it is recommended to create a file that exports
the various install locations as environment variables.

```bash
cat > env_vars_pfd.txt << "EOF"
export BOOST_ROOT=<boost-install-prefix>
export Libssh2_ROOT=<libssh2-install-prefix>
export LD_LIBRARY_PATH="${Libssh2_ROOT}/lib"${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
export GASPI_ROOT=<gpi-install-prefix>
export PKG_CONFIG_PATH="${GASPI_ROOT}/lib${arch}/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}
export GPISPACE_REPO=<gpi-space-repo>
export GPISPACE_BUILD_DIR=<gpi-space-build-dir>
export GPISPACE_INSTALL_DIR=<gpi-space-install-prefix>
export GPISPACE_TEST_DIR=<test-directory>
export GSPC_NODEFILE_FOR_TESTS="${PWD}/nodefile"
export SING_ROOT=<dir-for-sing-related-source-code>
export DEP_LIBS=<sing-dependencies-install-prefix>
export SINGULAR_INSTALL_DIR=<singular-install-prefix>
EOF
```

### Boost

| Website | Supported Versions |
| :-: | :-: |
| [Boost](https://boost.org) | >= 1.61, <= 1.63 |

Note, that Boost 1.61 is not compatible with OpenSSL >= 1.1, so we recommend
using Boost 1.63 as follows:

```bash
boost_version=1.63.0
export BOOST_ROOT=<install-prefix>

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
Take note to replace `<install-prefix>` with the appropriate path on your
system where you need boost installed. Also, if you are using
`env_vars_pfs.txt`, the export step can be ommited.

### libssh2

| Website | Supported Versions |
| :-: | :-: |
| [libssh2](https://www.libssh2.org/) | >= 1.7 |

`libssh2`  is not built with the OpenSSL backend on all systems. Additionally,
some versions available via package manager might not be compatible with
OpenSSH's default settings. For those reasons, we highly recommend building
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
libssh2_version=1.9.0
export Libssh2_ROOT=<install-prefix>
export LD_LIBRARY_PATH="${Libssh2_ROOT}/lib"${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}

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

```bash
arch=$(getconf LONG_BIT)
export GASPI_ROOT=<install-prefix>
export PKG_CONFIG_PATH="${GASPI_ROOT}/lib${arch}/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}

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
Note that `<install-prefix>` should be set to the correct path in the
script above.

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

```bash
cd "${GPISPACE_REPO}"

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
> SSH-key when using the SSH RIF strategy.
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

## Singular

We install the current version of Singular, which will be required by our framework.

Besides flint, Singular has various more standard dependencies, which are
usually available through the package manager of your distribution. Please refer
to the [step-by-step instructions to build Singular](https://github.com/Singular/Singular/wiki/Step-by-Step-Installation-Instructions-for-Singular) for more details.

Here we give a thorough guide for building the various dependencies and then
finally Singular itself, all the while assuming the user does *not* have `sudo`
privileges. We assume that mpfr and gmp are installed by the package manager
to `/usr`

Start by choosing a location where Singular can be cloned. We will indicate this
by `$SING_ROOT`. Clone Singular

```bash
cd $SING_ROOT
git clone git@github.com:Singular/Singular.git Sources
```

Now, we need to compile the various dependencies first.

### flint
The official guides for singular clones the latest development branch of flint.
As flint is being actively developed and the APIs changed quite often, this has
led to issues in the past.  Therefore, we rather recommend that a release
version be downloaded and compiled instead.

```bash
cd $SING_ROOT
wget http://www.flintlib.org/flint-2.7.1.tar.gz
tar -xvf flint-2.7.1.tar.gz
cd flint-2.7.1
./configure --with-gmp=/usr --prefix=$DEP_LIBS --with-mpfr=/usr
make -j $(nproc)
make install
```

### 4ti2
```bash
cd $SING_ROOT
mkdir 4ti2
cd 4ti2
wget http://www.4ti2.de/version_1.6/4ti2-1.6.tar.gz
tar xvfz 4ti2-1.6.tar.gz
cd 4ti2-1.6
./configure --prefix=$DEP_LIBS
make -j $(nproc)
```

### cddlib

```bash
cd $SING_ROOT
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
wget https://libntl.org/ntl-11.4.3.tar.gz
tar -xvf ntl-11.4.3.tar.gz
cd ntl-11.4.3/src # note the extra src
./configure PREFIX=$DEP_LIBS -fPIC #notice PREFIX is capitalized without dashes
make -j $(nproc)
make install
```

### Compile Singular

We are finally ready to compile Singular itself against the libraries we just
installed.
```bash
cd $SING_ROOT
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
We are finally in a position to compile the PFD project.

We expect the following variables to be set:
- `${GPISPACE_REPO}` as path to the repository cloned from Github.  We need this
  for some cmake scripts, amongst other reasons.
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
cd $PFD_BUILD_DIR
cmake -DCMAKE_INSTALL_PREFIX=$PFD_INSTALL_DIR   \
      -DCMAKE_BUILD_TYPE=Release            \
      -DGSPC_HOME=$GPISPACE_INSTALL_DIR     \
      -DALLOW_ANY_GPISPACE_VERSION=true     \
      -DGPISPACE_REPO=$GPISPACE_REPO        \
      -DSINGULAR_HOME=$SINGULAR_INSTALL_DIR \
      $PFD_REPO

make -j $(nproc)
make -j $(nproc) install
```

