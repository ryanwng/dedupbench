/**
 * @file repmaxcdc_chunking.cpp
 * @brief Implementation of RepMaxCDC chunking, ported from
 * simple_rep_max_content_defined_chunker.go in github.com/buildbarn/go-cdc
 * (Apache-2.0)
 *
 */

#include "repmaxcdc_chunking.hpp"

#include <algorithm>
#include <iostream>

RepMaxCDC_Chunking::RepMaxCDC_Chunking(const Config& config) {
    min_block_size = config.get_repmaxcdc_min_block_size();
    horizon_size = config.get_repmaxcdc_horizon_size();
    technique_name = "RepMaxCDC";

    // The cut point depends on the next 2 * min + horizon bytes, so the
    // buffer must hold them all.
    if (min_block_size < 64 ||
        config.get_buffer_size() < 2 * min_block_size + horizon_size) {
        std::cerr << "RepMaxCDC needs min >= 64 and buffer_size >= 2 * min + horizon"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
}

uint64_t RepMaxCDC_Chunking::find_cutpoint(char* data, uint64_t size) {
    const uint64_t* table = MaxCDC_Chunking::GEAR_TABLE;
    const uint8_t* buff = (const uint8_t*)data;
    uint64_t len = std::min(size, 2 * min_block_size + horizon_size);

    // Near the end of the data, take all of it or leave at least min bytes.
    if (len < 2 * min_block_size) {
        return len;
    }
    uint64_t end = len - min_block_size;

    // Gear hash of the 64 bytes before the first possible cut point
    uint64_t start_hash = 0;
    for (uint64_t i = min_block_size - 64; i < min_block_size; i++) {
        start_hash = (start_hash << 1) + table[buff[i]];
    }

    while (true) {
        // Find the first position whose hash is the largest within min bytes
        // after it
        uint64_t hash = start_hash;
        uint64_t best_hash = hash;
        uint64_t best_pos = min_block_size;
        for (uint64_t i = min_block_size; i < end && i < best_pos + min_block_size; i++) {
            hash = (hash << 1) + table[buff[i]];
            if (hash > best_hash) {
                best_hash = hash;
                best_pos = i + 1;
            }
        }
        if (best_pos < 2 * min_block_size) {
            return best_pos;
        }

        // Cutting there would give a chunk of 2 * min or more. Search again,
        // stopping min bytes before that position.
        end = best_pos - min_block_size;
    }
}
