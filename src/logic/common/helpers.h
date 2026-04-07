#ifndef RDP_EXT_LOGIC_HELPERS_H
#define RDP_EXT_LOGIC_HELPERS_H

#include <random>

inline uint32_t generateRandomValue() {
    std::mt19937 generator{std::random_device{}()};
    std::uniform_int_distribution<uint32_t> distribution;
    return distribution(generator);
}

#endif //RDP_EXT_LOGIC_HELPERS_H