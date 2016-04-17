#!/bin/bash

bin_dir=$(cd ../../../bin;pwd)

echo "### prefix=${bin_dir}/readline"
./configure --prefix=${bin_dir}/readline

make && make install
