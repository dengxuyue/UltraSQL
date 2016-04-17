#!/bin/bash

bin_dir=$(cd ../../bin;pwd)

echo "### prefix=${bin_dir}/cunit"
./configure --prefix=${bin_dir}/cunit

make && make install
