rm -rf Hayha/

mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX=Hayha ..
make -j8 && make install
mv Hayha/ ..
cd ..
