# PFD-Parallel - Partial Fraction Decomposition in Parallel

This package provide a massively parallel framework for partial fraction decomposition of
rational functions based on the [Singular/GPI-Space framework](https://www.mathematik.uni-kl.de/~boehm/singulargpispace/).

Our implementation is based on a combination of the following two algorithms:

1) the approach described in the paper

Janko Boehm, Marcel Wittmann, Zihao Wu, Yingxuan Xu, and Yang Zhang:
IBP reduction coefficients made simple, JHEP 12 (2020) 054,

which has been implemened in [Singular](https://www.singular.uni-kl.de) in the library
[pfd.lib](https://github.com/Singular/Singular/blob/spielwiese/Singular/LIB/pfd.lib).

2) MultivariateApart algorithm

Matthias Heller, Andreas von Manteuffel, Comput.Phys.Commun. 271 (2022) 108174 

Although applicable in general, its primary aim is the partial fraction
decomposition of integration-by-parts coefficients in high energy physics.

Our package relies on code developed in the  repository
[framework](https://github.com/singular-gpispace/framework)
implemented primarily by Lukas Ristau.

Since most useful in applications in high energy physics, the main function of our framework applies the partial fraction decoposition
function to a specified subset of entries of a two-dimensional array of rational functions.

To use the framework, it is required to install Singular, GPI-Space,
some of their dependencies and the project code itself. In the following we provide two different
ways of installation. The preferable way is the use the supercomputing package manager Spack, which will take
care of all dependencies automatically. 
The second way is a manual installation of a components, which can be used in case the installation with Spack is not possible on the target system.

# Installation using Spack
Spack is a package manager specifically aimed at handling software installations in supercomputing environments, but
usable on anything from a personal computer to an HPC cluster. It supports Linux and macOS (note that the Singular/GPI-Space
framework and hence our package requires Linux).

We will assume that the user has some directory path with read and
write access. In the following, we assume this path is set as the environment variable
`software_ROOT`, as detailed in the following example:

```bash
export software_ROOT=~/singular-gpispace

```
Note, this needs to be set again if you open a new terminal session (preferably set it automatically by adding the line to your .profile file).

## Install Spack
If Spack is not already present in the above directory, clone Spack from Github:
```bash
git clone https://github.com/spack/spack.git $software_ROOT/spack

```
We check out verison v0.19.0 of Spack (the current version):
```bash
cd $software_ROOT/spack
git checkout releases/v0.19
cd $software_ROOT

```
Spack requires a couple of standard system packages to be present. For example, on an Ubuntu machines they can be installed by the following commands (which typically require sudo privilege)

```bash
sudo apt update

```
```bash
sudo apt install build-essential ca-certificates coreutils curl environment-modules gfortran git gpg lsb-release python3 python3-distutils python3-venv unzip zip

```

To be able to use spack from the command line, run the setup script:
```bash
. $software_ROOT/spack/share/spack/setup-env.sh

```
Note, this script needs to be executed again if you open a new terminal session (preferably set it automatically by adding the line to your .profile file).

Finally, Spack needs to boostrap clingo.  This can be done by concretizing any
spec, for example
```bash
spack spec zlib

```

Note: If you experience connection timeouts due to a slow internet connection you can set in the following file the variable `connect_timeout` to a larger value.
```bash
vim $software_ROOT/spack/etc/spack/defaults/config.yaml

```

### How to uninstall Spack
Note that Spack can be uninstalled by just deleting its directory and its configuration files. Be CAREFUL to do that, since it will delete your Spack setup. Typically you do NOT want to do that now, so the code is commented out. It can be useful if your Spack installation is broken:

```bash
#cd
#rm -rf $software_ROOT/spack/
#rm -rf .spack

```

## Install pfd-parallel

Once you have installed Spack, our package can be installed with just three lines of code.

Clone the Singular/GPI-Space package repository into this directory:
```bash
git clone https://github.com/singular-gpispace/spack-packages.git $software_ROOT/spack-packages

```

Add the Singular/GPI-Space package repository to the Spack installation:
```bash
spack repo add $software_ROOT/spack-packages

```

Finally, install pfd-parallel:
```bash
spack install pfd-parallel
```
Note, this may take quite a bit of time, when doing the initial installation, as it needs to build GPI-Space and Singular
including dependencies. Installing further components of the framework or updating is then typically quick.

## Load pfd-parallel

Once pfd-parallel is installed, to use pfd-parallel load the package via:
```bash
spack load pfd-parallel

```
Note, that this command needs to be executed again if you open a new terminal session. In particular, the environment variable `PFD_INSTALL_DIR` is available only after executing this command.

## Set up ssh

GPI-Space requires a working SSH environment with a password-less
SSH-key when using the SSH RIF strategy. To ensure this, 
leave the password field empty when generating your ssh keypair.

By default, `${HOME}/.ssh/id_rsa` is used for authentication. If no such key exists,
```bash
ssh-keygen -t rsa -b 4096 -N '' -f "${HOME}/.ssh/id_rsa"

```
can be used to create one. 

If you compute on your personal machine, there must run an ssh server. On an Ubuntu machine, the respective package can be installed by:

```bash
sudo apt install openssh-server

```

Your key has to be registered with the machine you want to compute on. On a cluster with shared home directory, this only has to be done on one machine. For example, if you compute on your personal machine, you can register the key with:
```bash
ssh-copy-id -f -i "${HOME}/.ssh/id_rsa" "${HOSTNAME}"

```
# Example how to use pfd-parallel

## Setup Spack and load pfd-parallel

If you start in a new terminal session (and did not configure your terminal to do this automatically) make sure to set `software_ROOT` and run the `setup-env.sh` script. Make also sure to load the pfd-parallel package in Spack. As discussed above this can be done by:

```bash
export software_ROOT=~/singular-gpispace
. $software_ROOT/spack/share/spack/setup-env.sh
spack load pfd-parallel

```

# Setup directories and example files

We create directories which will used for input and output: 

```bash
mkdir -p $software_ROOT/input
mkdir -p $software_ROOT/results

```
To execute an example, we will use rational function data, which is provided with the installation and is copied by the following command to in the input directory.

```bash
cp $PFD_INSTALL_DIR/example_data/* $software_ROOT/input

```


We create a nodefile, which contains the names of the nodes used for computations with our framework. In our example, it just contains the result of hostname.

```bash
hostname > $software_ROOT/nodefile

```
Moreover, we need a directory for temporary files, which should be accessible from all machines involved in the computation:

```bash
mkdir -p $software_ROOT/tempdir

```

# Start the monitor

Optionally, but recommended: We start the GPI-Space Monitor to display computations in form of a Gantt diagram (to do so, you need an X-Server running).
In case you do not want to use the monitor, you should not set in Singular the fields options.loghostfile and options.logport of the GPI-Space configuration token (see below). In order to use the GPI-Space Monitor, we need a loghostfile with the name of the machine running the monitor.

```bash
hostname > $software_ROOT/loghostfile

```

On this machine, start the monitor, specifying a port number where the monitor will receive information from GPI-Space. The same port has to be specified in Singular in the field options.logport.

```bash
$PFD_INSTALL_DIR/libexec/bundle/gpispace/bin/gspc-monitor --port 6439 &

```

# Start Singular

We start Singular in the directory where it will have direct access to all relevant directories we just created, telling it also where to find the libraries for our framework:

```bash
cd $software_ROOT
SINGULARPATH="$PFD_INSTALL_DIR/LIB"  $SINGULAR_INSTALL_DIR/bin/Singular

```

# Configure and run computation in Singular

In Singular, now do what follows below.

This
* loads the library giving access to pfd-parallel,
* creates a configuration token for the Singular/GPI-Space framework,
  * adds information where to store temporary data (in the field options.tmpdir),
  * where to find the nodefile (in the field options.nodefile), and
  * sets how many processes per node should be started (in the field options.procspernode, usually one process per core, not taking hyper-threading into account; you may have to adjust according to your hardware),
* creates a configuration token for pfd-parallel, adds information on
  * the base file name (in the field pfdconfig.filename),
  * the input directory (in the field pfdconfig.inputdir)
  * the output directory (in the field pfdconfig.outputdir)
  * the output format(s) (in the field pfdconfig.outputformat)
  * the choice of parallelization strategy (in the field pfdconfig.parallelism)
  * the choice of algorithmic strategy (in the field pfdconfig.algorithm). 

* creates a polynomial ring containing the numerators and denominators
* creates a list with the indices of the array entries to be decomposed (referencing the base file name)
* and starts the computation.

Note that in more fancy environments like a cluster, one should specify absolute paths to the nodefile and the temp directory.


```bash
LIB "pfd_gspc.lib";

// configuration for gpispace
configToken gspcconfig = configure_gspc();

gspcconfig.options.tempdir = "tempdir";
gspcconfig.options.nodefile = "nodefile";
gspcconfig.options.procspernode = 8;

// Should the user want to run withouth the gspc-monitor
// the following two lines may be commented out.
gspcconfig.options.loghostfile = "loghostfile";
gspcconfig.options.logport = 6439;

// configuration for the problem to be computed.
configToken pfdconfig = configure_pfd();
pfdconfig.options.filename = "xb_deg5";
pfdconfig.options.inputdir = "input";
pfdconfig.options.outputdir = "results";
pfdconfig.options.outputformat = "ssi,cleartext,listnumden,indexed_denominator";
pfdconfig.options.parallelism = "intertwined";
pfdconfig.options.algorithm = "Leinartas";

ring r = 0, x, lp;

list entries = list( list(1, 1), list(1, 2), list(1, 3), list(1, 4)
                   , list(1, 5), list(1, 6), list(1, 7), list(1, 8)
                   , list(1, 9), list(1, 10)
                   );

parallel_pfd( entries
            , gspcconfig
            , pfdconfig
            );

```

The results can then be found in the directory `$software_ROOT/results`. Note that if an output file already exist, the respective computation will not be repeated to allow for restartability. The Singular command returns a list of strings providing information about whether the computation was successful, or was skipped since the result file is already there. After the above run, the output directory contains for each individual computation output files in the requested formats. 

## Configuration options for pfd_parallel

* Output formats (in the field pfdconfig.outputformat):

outputformat=ssi,cleartext,listnumden,indexed_denominator

a file with the indexing of the denominators (in human readable form), a file with the actual partial fraction decomposition (in human readable form, referencing the denominator file), a text file containing the result without indexing (in human readable form), an ssi file in the binary Singular serialization format (consistent between input and output), and a resources file giving information about the time and memory used by the individual steps of the algorithm.

* parallelization strategy (in the field pfdconfig.parallelism)

parallelization strategy (in the field pfdconfig.parallelism). Note, to run the same computation, but without the internal parallelism on each entry, the `parallelism` field in the `pfdconfig` tokens `options` field may be changed to
`waitAll`.

* algorithmic strategy (in the field pfdconfig.algorithm)


# Appendix: Convenient scripts to run an example in pfd-parallel
To run examples repeatedly, it can be useful to create a Singular script and a 
couple of shell scripts. This is described in the following, again at the above 
example.

We start out by creating a Singular script. It will loads the `pfd_gspc.lib`
library. A GPI-Space configure token is prepared, with the
configuration:  The path of a temporary directory, where files will be stored
during the computation and handling of the various files at runtime, the
location of a nodefile, containing a location on then network where GPI-Space is
installed, the number of processes each node should run in parallel, the address
of a running instance of the GPI-Space monitoring tool, as well as the port on
which the monitoring tool is listening (note, the framework can be run without the
monitoring tool, in which case the corresponding entries should remain unset). Next,
the ring in which the numerators and denominators of the rational functions are contained is
declared.  The input of the computation is specified in terms of files, referenced by the
row and column index in a matrix where it must be found, in the form
`<basename>_<row>_<col>.(txt|ssi)`, where the suffix is txt if the file is in
plain text format, and ssi if the input files are in this high-performence binary format
used by Singular. In the text format input files, the
coordinates of the matrix entries to be calculated are given in a list of lists.

Finally, all the tokens and index pairs are provided to the `parallel_pfd` function as arguments,
preferably with an optional argument for the path to where the input files are
found.  The user may also provide in a separate argument the path of where the
output files should be written.

For the examples below, the user needs to set the following environment
variables providing the software root and input and output directories:
```bash
export PFD_ROOT=$software_ROOT
export PFD_INPUT_DIR=$software_ROOT/input
export PFD_OUTPUT_DIR=$software_ROOT/results

```

An example script `test_parallel_pfd.sing` in Singular for a 1 by 10 matrix is created as follows:

```bash
mkdir -p $PFD_ROOT/tempdir
hostname > $PFD_ROOT/nodefile
hostname > $PFD_ROOT/loghostfile

cat > test_parallel_pfd.sing.temp << "EOF"
LIB "pfd_gspc.lib";

// configuration for gpispace
configToken gspcconfig = configure_gspc();

gspcconfig.options.tempdir = "$PFD_ROOT/tempdir";
gspcconfig.options.nodefile = "$PFD_ROOT/nodefile";
gspcconfig.options.procspernode = 8;

// Should the user want to run withouth the gspc-monitor
// the following two lines may be commented out.
gspcconfig.options.loghostfile = "$PFD_ROOT/loghostfile";
gspcconfig.options.logport = 6439;

// configuration for the problem to be computed.
configToken pfdconfig = configure_pfd();
pfdconfig.options.filename = "xb_deg5";
pfdconfig.options.inputdir = "$PFD_INPUT_DIR";
pfdconfig.options.outputdir = "$PFD_OUTPUT_DIR";
pfdconfig.options.parallelism = "intertwined";

ring r = 0, x, lp;

list entries = list( list(1, 1), list(1, 2), list(1, 3), list(1, 4)
                   , list(1, 5), list(1, 6), list(1, 7), list(1, 8)
                   , list(1, 9), list(1, 10)
                   );

parallel_pfd( entries
            , gspcconfig
            , pfdconfig
            );
exit;
EOF

```
We expand the environment variables in the script created above using the following script:
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

Next, we create a script to start the graphical GPI-Space monitor displaying a Gant diagram of the computation. 
Note that if you do not want to use the monitor, you can skip these steps, but have to comment out the respective lines in the script `test_parallel_pfd.sing`.
```bash
cat > start_monitor.sh << "EOF"
#!/bin/bash

set -euo pipefail

# raster or native (native for X forwarding)
QT_DEBUG_PLUGINS=0                                                \
        QT_GRAPHICSSYSTEM=native                                  \
        $PFD_INSTALL_DIR/libexec/bundle/gpispace/bin/gspc-monitor \
        --port 6439 &

EOF
chmod a+x start_monitor.sh

```
It can be run as
```
./start_monitor.sh

```

Make sure that the `--port` number matches the one set in the singular script.
Also, if this is run over ssh on a remote machine, make sure that x forwarding
is enabled.

Create the input files:
```bash
mkdir -p $PFD_INPUT_DIR
mkdir -p $PFD_OUTPUT_DIR
cp -v $PFD_INSTALL_DIR/example_data/* $PFD_INPUT_DIR

```

Finally, the test may be run with the script
```bash
cat > run_pfd_example.sh << "EOF"
SINGULARPATH="$PFD_INSTALL_DIR/LIB"                               \
        $SINGULAR_INSTALL_DIR/bin/Singular                        \
        test_parallel_pfd.sing
EOF
chmod a+x run_pfd_example.sh
```
which can be started with
```
./run_pfd_example.sh

```

Note, to run the same computation, but without the internal parallelism on each
entry, the `parallelism` field in the `pfdconfig` tokens `options` field may be changed to
`waitAll`.

# Appendix: Manual installation from sources

Note: This section is not necessary if the installation via Spack has been completed. It contains
everything what Spack does automatically for you.

As an alternative to Spack, pfd-parallel and its dependencies can also be compiled
directly.  For most users, a Spack installation should suffice, in which case
this section should be skipped.

For the various dependencies, it is recommended to create a file that exports
the various install locations as environment variables. For this purpose, the
following command may be run at a convenient location, after which the resulting
file should be edited to specify two directory roots, one for purposes of
compilation and one for installation of the code. While the first should
typically be a fast local file system, the second must be accessible from all
computation nodes to be used for running the system.

```bash
cat > env_vars_pfd.txt << "EOF"
export software_ROOT=<software-root>
# Some fast location in local system for hosting build directories,
# for example, something like /tmpbig/$USER/pfd-parallel or just $HOME/pfd-parallel if the user has a
# fast home directory.

export install_ROOT=<install-root>
# The install root is recommended to be some network (nfs) mountpoint, where
# each node of the cluster should be able to read from, for example
# something like /scratch/$USER/pfd-parallel

export compile_ROOT=$software_ROOT
# Optionally, this might be set to something like /dev/shm/$USER/pfd-parallel that
# stores files purely in memory, thus the contents of this
# location will be lost after reboot.  It can speed up the computation times, as
# all disk io becomes memory io.

# GPI-Space dependencies:
export BOOST_ROOT=$install_ROOT/boost
export Libssh2_ROOT=$install_ROOT/libssh
export Libssh2_BUILD_DIR=$compile_ROOT/libssh/build
export LD_LIBRARY_PATH="${Libssh2_ROOT}/lib"${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}
export GASPI_ROOT=$install_ROOT/gpi2
export cpu_arch=$(getconf LONG_BIT)
export PKG_CONFIG_PATH="${GASPI_ROOT}/lib${cpu_arch}/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}
export PKG_CONFIG_PATH="${Libssh2_ROOT}/lib/pkgconfig:${Libssh2_ROOT}/lib64/pkgconfig"${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}


# GPI-Space:
export GPI_ROOT_DIR=$software_ROOT
export GPISPACE_REPO=$GPI_ROOT_DIR/gpispace/gpispace
export GPISPACE_BUILD_DIR=$compile_ROOT/gpispace/build
export GPISPACE_INSTALL_DIR=$install_ROOT/gpispace
export GPISpace_ROOT=$GPISPACE_INSTALL_DIR # expected by cmake
export GSPC_HOME=$GPISPACE_INSTALL_DIR # Set mostly for legacy reasons
export SHARED_DIRECTORY_FOR_TESTS=$GPISPACE_BUILD_DIR/tests

# Singular:
export SING_ROOT=$software_ROOT/Singular
export DEP_LIBS=$install_ROOT/sing_dep_libs
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$DEP_LIBS/lib
export SINGULAR_INSTALL_DIR=$install_ROOT/Singular
export SINGULAR_BUILD_DIR=$compile_ROOT/Singular/build

# PFD:
export PFD_ROOT=$software_ROOT/pfd-parallel
export PFD_REPO=$PFD_ROOT/pfd-parallel
export PFD_INSTALL_DIR=$install_ROOT/pfd-parallel
export PFD_BUILD_DIR=$compile_ROOT/pfd-parallel/build
export PFD_INPUT_DIR=$software_ROOT/input
export PFD_OUTPUT_DIR=$software_ROOT/results
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
mkdir -p $software_ROOT
mkdir -p $install_ROOT
mkdir -p $compile_ROOT

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
    --branch v22.03                                               \
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
mkdir -p $compile_ROOT/4ti2/build

cd $SING_ROOT
mkdir 4ti2 && cd 4ti2
wget http://www.4ti2.de/version_1.6/4ti2-1.6.tar.gz
tar xvfz 4ti2-1.6.tar.gz

pushd $compile_ROOT/4ti2/build

$SING_ROOT/4ti2/4ti2-1.6/configure --prefix=$DEP_LIBS
make -j $(nproc)
make install

popd

```

### cddlib

```bash
mkdir -p $compile_ROOT/cddlib/build

cd $SING_ROOT
mkdir cddlib && cd cddlib
wget https://github.com/cddlib/cddlib/releases/download/0.94j/cddlib-0.94j.tar.gz
tar -xvf cddlib-0.94j.tar.gz

pushd $compile_ROOT/cddlib/build

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
    https://github.com/singular-gpispace/pfd-parallel.git         \
    pfd-parallel

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


## Standard packages required to build the framework via the manual installation

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
