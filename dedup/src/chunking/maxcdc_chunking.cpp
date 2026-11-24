/**
 * @file maxcdc_chunking.cpp
 * @brief Implementation of MaxCDC chunking, ported from
 * simple_max_content_defined_chunker.go in github.com/buildbarn/go-cdc
 * (Apache-2.0)
 *
 */

#include "maxcdc_chunking.hpp"

#include <algorithm>
#include <iostream>

MaxCDC_Chunking::MaxCDC_Chunking(const Config& config) {
    min_block_size = config.get_maxcdc_min_block_size();
    max_block_size = config.get_maxcdc_max_block_size();
    technique_name = "MaxCDC";

    // The cut point depends on the next min + max bytes, so the buffer
    // must hold them all.
    if (min_block_size < 64 || max_block_size < min_block_size ||
        config.get_buffer_size() < min_block_size + max_block_size) {
        std::cerr << "MaxCDC needs 64 <= min <= max and buffer_size >= min + max"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
}

uint64_t MaxCDC_Chunking::find_cutpoint(char* data, uint64_t size) {
    const uint8_t* buff = (const uint8_t*)data;
    uint64_t len = std::min(size, min_block_size + max_block_size);

    // Near the end of the data, take all of it or leave at least min bytes.
    if (len < 2 * min_block_size) {
        return len;
    }
    len -= min_block_size;

    // Gear hash of the 64 bytes before the first possible cut point
    uint64_t hash = 0;
    for (uint64_t i = min_block_size - 64; i < min_block_size; i++) {
        hash = (hash << 1) + GEAR_TABLE[buff[i]];
    }

    // Cut where the hash is largest
    uint64_t best_hash = hash;
    uint64_t best_pos = min_block_size;
    for (uint64_t i = min_block_size; i < len; i++) {
        hash = (hash << 1) + GEAR_TABLE[buff[i]];
        if (hash > best_hash) {
            best_hash = hash;
            best_pos = i + 1;
        }
    }
    return best_pos;
}
