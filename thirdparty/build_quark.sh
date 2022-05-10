#!/bin/sh

curl http://icl.cs.utk.edu/projectsfiles/quark/pubs/quark-0.9.0.tgz | tar xfz -
cd quark-0.9.0
sed -i -e 's/CFLAGS=-O2/CFLAGS=-O3/' make.inc
make
prefix=install make install

