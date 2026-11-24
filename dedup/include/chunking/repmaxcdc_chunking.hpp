#ifndef _REPMAXCDC_CHUNKING_
#define _REPMAXCDC_CHUNKING_

#include "chunking_common.hpp"
#include "config.hpp"
#include "maxcdc_chunking.hpp"

class RepMaxCDC_Chunking : public virtual Chunking_Technique {
    /**
     * @brief Class implementing RepMaxCDC (Schouten, "Content-Defined
     * Chunking with tight chunk size bounds", 2026). Ported from
     * github.com/buildbarn/go-cdc (Apache-2.0). RecMaxCDC is RepMaxCDC with
     * horizon_size = buffer_size - 2 * min_block_size.
     *
     */

   private:
    uint64_t min_block_size;
    uint64_t horizon_size;

    /**
     * @brief finds the next cut point in an array of bytes
     * @param buff: the buff to find the cutpoint in.
     * @param size: the size of the buffer
     * @return: cutpoint position in the buffer
     */
    uint64_t find_cutpoint(char* buff, uint64_t size) override;

   public:
    /**
     * @brief Defines all parameters based on values from the config file
     * @return: void
     */
    RepMaxCDC_Chunking(const Config& config);
};

#endif
