#!/bin/bash -e

VENDOR_PATH=vendor/amcl/version3/c

# Fetch source
git submodule init
git submodule update

# Compile
(
  cd $VENDOR_PATH ;
  rm * ;
  git checkout . ;
  sed -ie 's/ -O3/ -O3 -fPIC/g' config64.py ;
  echo -e "18\n0" | python config64.py
)

# "Install"
cp $VENDOR_PATH/*.h $VENDOR_PATH/amcl.a external/amcl
mv external/amcl/amcl.a external/amcl/libamcl.a

# Clean up
(
  cd $VENDOR_PATH ;
  rm * ;
  git checkout . ;
)
