cd sudokugenerator-0.10
./configure
make -j4
cd -
cd minisat/core
export MROOT=../
make -j4
cd -
cd lingeling-ala-b02aa1a-121013
./configure
make -j4
cd -
cd glucose2.2/core
make
cd -
cd clasp-2.1.3
./configure.sh
cd build/release
make -j4


