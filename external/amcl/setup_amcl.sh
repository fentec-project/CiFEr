#!/bin/bash

git clone https://github.com/miracl/amcl.git
cd amcl/version3/c
git reset --hard e39201e3d34f4406530c103bb01f50fd84253b48
python ../../../config64_modified.py
mkdir /usr/local/include/amcl
cp *.h /usr/local/include/amcl
cp amcl.a /usr/local/lib/libamcl.a
cd ../../..
rm -rf amcl
