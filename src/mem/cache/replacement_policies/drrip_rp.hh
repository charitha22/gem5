

#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_DRRIP_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_DRRIP_RP_HH__

// #include "base/sat_counter.hh"
#include "base/templated_sat_counter.h"
#include "mem/cache/replacement_policies/base.hh"

struct DRRIPRPParams;

class DRRIPRP : public BaseReplacementPolicy
{
  protected:
    /** BRRIP-specific implementation of replacement data. */
    struct DRRIPReplData : ReplacementData
    {
        /**
         * Re-Reference Interval Prediction Value.
         * Some values have specific names (according to the paper):
         * 0 -> near-immediate re-rereference interval
         * max_RRPV-1 -> long re-rereference interval
         * max_RRPV -> distant re-rereference interval
         */
        SatCounter<uint8_t> rrpv;

        /** Whether the entry is valid. */
        bool valid;

        /**
         * Default constructor. Invalidate data.
         */
        DRRIPReplData(const int num_bits)
            : rrpv(num_bits), valid(false)
        {
        }
    };

    /**
     * Number of RRPV bits. An entry that saturates its RRPV has the longest
     * possible re-reference interval, that is, it is likely not to be used
     * in the near future, and is among the best eviction candidates.
     * A maximum RRPV of 1 implies in a NRU.
     */
    const unsigned numRRPVBits;

    /**
     * The hit priority (HP) policy replaces entries that do not receive cache
     * hits over any cache entry that receives a hit, while the frequency
     * priority (FP) policy replaces infrequently re-referenced entries.
     */
    const bool hitPriority;

    /**
     * Bimodal throtle parameter. Value in the range [0,100] used to decide
     * if a new entry is inserted with long or distant re-reference.
     */
    const unsigned btp;

    const unsigned no_sets;
    // policy selector
    mutable SatCounter<uint16_t> psel;
    // no of bits assigned for members under one consitituent
    const unsigned no_offset_bits;

  public:
    /** Convenience typedef. */
    typedef DRRIPRPParams Params;

    /**
     * Construct and initiliaze this replacement policy.
     */
    DRRIPRP(const Params *p);

    /**
     * Destructor.
     */
    ~DRRIPRP() {}

    /**
     * Invalidate replacement data to set it as the next probable victim.
     * Set RRPV as the the most distant re-reference.
     *
     * @param replacement_data Replacement data to be invalidated.
     */
    void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
                                                              const override;

    /**
     * Touch an entry to update its replacement data.
     *
     * @param replacement_data Replacement data to be touched.
     */
    void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Reset replacement data. Used when an entry is inserted.
     * Set RRPV according to the insertion policy used.
     *
     * @param replacement_data Replacement data to be reset.
     */
    void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                     override;

    /**
     * Find replacement victim using rrpv.
     *
     * @param cands Replacement candidates, selected by indexing policy.
     * @return Replacement entry to be replaced.
     */
    ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const
                                                                     override;

    /**
     * Instantiate a replacement data entry.
     *
     * @return A shared pointer to the new replacement data.
     */
    std::shared_ptr<ReplacementData> instantiateEntry() override;
    
    bool is_policy1_set(uint32_t set) const;
    bool is_policy2_set(uint32_t set) const;
    void reset_policy1(const std::shared_ptr<DRRIPReplData>& replacement_data ) const;
    void reset_policy2(const std::shared_ptr<DRRIPReplData>& replacement_data ) const;
};

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_DRRIP_RP_HH__
