#!/bin/bash

bin_dir=$(cd ../../../bin;pwd)

echo "### prefix=${bin_dir}/yaml"
./configure --prefix=${bin_dir}/yaml

make && make install
