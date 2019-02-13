git clone https://github.com/miracl/amcl.git
cd amcl/version3/c
python3 ../../../config64_modified.py
mkdir /usr/local/include/amcl
cp *.h /usr/local/include/amcl
cp amcl.a /usr/local/lib/libamcl.a
cd ../../..
rm -rf amcl
