
#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_DIP_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_DIP_RP_HH__

#include "base/templated_sat_counter.h"
#include "mem/cache/replacement_policies/lru_rp.hh"

struct DIPRPParams;

class DIPRP : public LRURP
{
  protected:
    /**
     * Bimodal throtle parameter. Value in the range [0,100] used to decide
     * if a new entry is inserted at the MRU or LRU position.
     */
    const unsigned btp;
    // no of sets in the cache
    const unsigned no_sets;
    // policy selector
    mutable SatCounter<uint16_t> psel;

    const unsigned no_offset_bits;

    /** DIP-specific implementation of replacement data. */
    struct DIPReplData : ReplacementData
    {
        /** Tick on which the entry was last touched. */
        Tick lastTouchTick;

        /**
         * Default constructor. Invalidate data.
         */
        DIPReplData() : lastTouchTick(0) {}
    };

  public:
    /** Convenience typedef. */
    typedef DIPRPParams Params;

    /**
     * Construct and initiliaze this replacement policy.
     */
    DIPRP(const Params *p);

    /**
     * Destructor.
     */
    ~DIPRP() {}

    /**
     * Reset replacement data for an entry. Used when an entry is inserted.
     * Uses the bimodal throtle parameter to decide whether the new entry
     * should be inserted as MRU, or LRU.
     *
     * @param replacement_data Replacement data to be reset.
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    std::shared_ptr<ReplacementData> instantiateEntry() override;

    bool is_policy1_set(uint32_t set) const;
    bool is_policy2_set(uint32_t set) const;
    void reset_policy1(const std::shared_ptr<DIPReplData>& replacement_data ) const;
    void reset_policy2(const std::shared_ptr<DIPReplData>& replacement_data ) const;
};
#endif // __MEM_CACHE_REPLACEMENT_POLICIES_DIP_RP_HH__
