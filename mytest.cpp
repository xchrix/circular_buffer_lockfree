#include "circular_buffer_lockfree.hpp"

#include <iostream>

int main()
{
    chrix::circular_buffer_lockfree<size_t> dat(128);
#if 0
    for (size_t i = 1; i < sizeof(char) * 8; i = i * 2)
    {
        std::cout << i << '\t';
    }
    std::cout << std::endl;
#endif
}