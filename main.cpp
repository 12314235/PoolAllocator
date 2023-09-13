#include <iostream>
#include <deque>
#include <list>
#include <chrono>
#include <fstream>
#include <string>
#include "pool_allocator.h"

template<class... Args>
void print(Args... args)
{
    (std::cout << ... << args) << "\n";
}

int main() {
    pool_allocator<int> alloc_ = pool_allocator<int>({std::make_pair(100000000, 20)});
    std::list<int> v;

    std::ofstream output;
    output.open("D:\\labwork-9-12314235\\vector_std_allocator.csv", std::ios::binary);

    for(int i = 1000; i < 4000000; i += 10000){
        auto start_time = std::chrono::high_resolution_clock::now();
        for(int j = 0; j < i; j++){
            v.push_back(j);
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration_ms = duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        std::string b = std::to_string(i) + "," + std::to_string(duration_ms) + "\n";
        output.write(b.c_str(), b.size());
        v.clear();
    }

    output.close();

    return 0;
}
