#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include "myAllocator.h"

const int numberOfIterations = 10000;
const int randomRange = 1000;

/*   Test Generator
    *   Create a random list of sizes (random but keeps constant during the entire runtime).
    *   In each iteration, start from an empty vector.
    *   Push back elements one by one according to the random size, then pop back all.
*/
template <class Container>
class TestBed {
public:
    /*
     * No seed provided for the random engine so that
     * random distribution keeps constant for std/custom allocators
    */
    TestBed() : randGen(0), randDist(0, randomRange) {}
    virtual double run() {
        auto from = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < numberOfIterations; i++)
            testIteration(randDist(randGen));
        auto to = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::duration<double>>(to - from).count();
    }
    virtual void testIteration(int newSize) {
        int size = 0;
        while (size < newSize)
            this->container.push_back(size++);
        for (; size > newSize; size--)
            this->container.pop_back();
    }
    virtual ~TestBed() {}
private:
    Container container;
    std::default_random_engine randGen;
    std::uniform_int_distribution<int> randDist;
};

template <class StlContainer, class FastContainer>
void printTestStatus(const char *name, StlContainer &stlContainer, FastContainer &fastContainer) {
    std::cout << std::fixed;
    std::cout << name << " - Default STL Allocator : " << stlContainer.run() << " seconds." << std::endl;
    std::cout << name << " - Memory Pool Allocator : " << fastContainer.run() << " seconds." << std::endl;
    std::cout << std::endl;
}

int main() {
    typedef int64_t DataType;
    TestBed<std::vector<DataType>> std_alloc;
    TestBed<std::vector<DataType, myAlloc::allocator<DataType>>> my_alloc;
    printTestStatus("Vector push_back",std_alloc, my_alloc);
    return 0;
}