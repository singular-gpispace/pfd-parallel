#!/bin/bash
#this script is for demonstration with command line environment: no gspc-monitor
#basic set up for pfd-parallel
export software_ROOT=~/singular-gpispace
##one may change this to their own $software_ROOT
. $software_ROOT/spack/share/spack/setup-env.sh
spack load pfd-parallel
#set up ssh-server
#ssh-keygen -t rsa -b 4096 -N '' -f "${HOME}/.ssh/id_rsa"
#ssh-copy-id -f -i "${HOME}/.ssh/id_rsa" "${HOSTNAME}"
#set up folders for computation nodes, input and output
rm -rf $outputFolder
mkdir -p $outputFolder
##set output folder, one should substitute $outputFolder to their own ones 
#change the raw data into the form of listnumden form via mathematica
#math -script listnumden-converter.wl $software_ROOT/factored $software_ROOT/factoredinputs
hostname > $software_ROOT/nodefile
mkdir -p $software_ROOT/tempdir
#start Singular embedded in pfd-parallel and run the test script
cd $software_ROOT
SINGULARPATH="$PFD_INSTALL_DIR/LIB"  $SINGULAR_INSTALL_DIR/bin/Singular Singular_script_template.sing
