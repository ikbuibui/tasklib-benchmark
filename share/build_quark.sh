#!/bin/sh

curl https://icl.utk.edu/projectsfiles/quark/pubs/quark-0.9.0.tgz | tar xfz -
cd quark-0.9.0
sed -i -e 's/CC=.*/CC=gcc/' make.inc
sed -i -e 's/CFLAGS=-O2/CFLAGS=-O3/' make.inc
make -j
prefix=${BENCHMARK_DIR}/thirdparty/.install make install

