mkdir -p build
cd build
cmake ..
make all
echo "run correctness test #1 ..."
./correctness_test1
echo "run correctness test #2 ..."
./correctness_test2
echo "run speed test ..."
./speedtest