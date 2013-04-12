cd sudokugenerator-0.10
./configure
make -j4
cd -
cd minisat/core
export MROOT=../
make -j4
cd -

