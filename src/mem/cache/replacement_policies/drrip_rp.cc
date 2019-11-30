
#include "mem/cache/replacement_policies/drrip_rp.hh"

#include <cassert>
#include <memory>

#include "base/logging.hh" // For fatal_if
#include "base/random.hh"
#include "params/DRRIPRP.hh"
#include "debug/CacheReplPolicy.hh"

DRRIPRP::DRRIPRP(const Params *p)
    : BaseReplacementPolicy(p),
      numRRPVBits(p->num_bits), hitPriority(p->hit_priority), btp(p->btp),
    no_sets((p->size) / (p->assoc * p->cache_line_size)),
    psel(p->psel_bits,  1 << (p->psel_bits-1)),
    no_offset_bits((uint)std::log2(no_sets / p->sample_size))
{
    DPRINTF(CacheReplPolicy, "psel init value : %d \n", 1 << (p->psel_bits-1));
    fatal_if(numRRPVBits <= 0, "There should be at least one bit per RRPV.\n");
}

void
DRRIPRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
    std::shared_ptr<DRRIPReplData> casted_replacement_data =
        std::static_pointer_cast<DRRIPReplData>(replacement_data);

    // Invalidate entry
    casted_replacement_data->valid = false;
}

void
DRRIPRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<DRRIPReplData> casted_replacement_data =
        std::static_pointer_cast<DRRIPReplData>(replacement_data);

    // Update RRPV if not 0 yet
    // Every hit in HP mode makes the entry the last to be evicted, while
    // in FP mode a hit makes the entry less likely to be evicted
    if (hitPriority) {
        casted_replacement_data->rrpv.reset();
    } else {
        casted_replacement_data->rrpv--;
    }
}

void
DRRIPRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<DRRIPReplData> casted_replacement_data =
        std::static_pointer_cast<DRRIPReplData>(replacement_data);
    uint32_t set = casted_replacement_data->set;


    if (is_policy1_set(set)){
        psel++;
        reset_policy1(casted_replacement_data);
    }
    else if (is_policy2_set(set)){
        psel--;
        reset_policy2(casted_replacement_data);
    }
    else{
        if (psel.getMSB() == 0) reset_policy1(casted_replacement_data);
        else  reset_policy2(casted_replacement_data);
    }
    
    Tick t = curTick();
    if(t%1000000000 == 0){
        DPRINTF(CacheReplPolicy, "value of psel counter at tick %d : %d\n",
                curTick(), psel.getCounter());
    }

}

ReplaceableEntry*
DRRIPRP::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // Use first candidate as dummy victim
    ReplaceableEntry* victim = candidates[0];

    // Store victim->rrpv in a variable to improve code readability
    int victim_RRPV = std::static_pointer_cast<DRRIPReplData>(
                        victim->replacementData)->rrpv;

    // Visit all candidates to find victim
    for (const auto& candidate : candidates) {
        std::shared_ptr<DRRIPReplData> candidate_repl_data =
            std::static_pointer_cast<DRRIPReplData>(
                candidate->replacementData);

        // Stop searching for victims if an invalid entry is found
        if (!candidate_repl_data->valid) {
            return candidate;
        }

        // Update victim entry if necessary
        int candidate_RRPV = candidate_repl_data->rrpv;
        if (candidate_RRPV > victim_RRPV) {
            victim = candidate;
            victim_RRPV = candidate_RRPV;
        }
    }

    // Get difference of victim's RRPV to the highest possible RRPV in
    // order to update the RRPV of all the other entries accordingly
    int diff = std::static_pointer_cast<DRRIPReplData>(
        victim->replacementData)->rrpv.saturate();

    // No need to update RRPV if there is no difference
    if (diff > 0){
        // Update RRPV of all candidates
        for (const auto& candidate : candidates) {
            std::static_pointer_cast<DRRIPReplData>(
                candidate->replacementData)->rrpv += diff;
        }
    }

    return victim;
}

std::shared_ptr<ReplacementData>
DRRIPRP::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new DRRIPReplData(numRRPVBits));
}

void
DRRIPRP::reset_policy1(const std::shared_ptr<DRRIPReplData>& replacement_data) const
{
    // Reset RRPV
    // Replacement data is inserted as "long re-reference" if lower than btp,
    // "distant re-reference" otherwise
    replacement_data->rrpv.saturate();
    if (random_mt.random<unsigned>(1, 100) <= btp) {
        replacement_data->rrpv--;
    }

    // Mark entry as ready to be used
    replacement_data->valid = true;
}

void
DRRIPRP::reset_policy2(const std::shared_ptr<DRRIPReplData>& replacement_data) const
{
    replacement_data->rrpv.saturate();
    // always insert as "long re-reference"
    replacement_data->rrpv--;

    // Mark entry as ready to be used
    replacement_data->valid = true;
}

// if constituency bits == offset bits
bool DRRIPRP::is_policy1_set(uint32_t set) const
{
    uint32_t sat_val = (1 << no_offset_bits) - 1;
    uint32_t offset = set & sat_val;
    uint32_t trunc_consti_id = (set >> no_offset_bits) & sat_val;
    if (offset == trunc_consti_id)
        return true;

    return false;
}

// if !constituency bits == offset bits
bool DRRIPRP::is_policy2_set(uint32_t set) const
{
    uint32_t sat_val = (1 << no_offset_bits) - 1;
    uint32_t offset = set & sat_val;
    uint32_t trunc_consti_id = (set >> no_offset_bits) & sat_val;
    if (offset == (~trunc_consti_id & sat_val))
        return true;

    return false;
}

DRRIPRP*
DRRIPRPParams::create()
{
    return new DRRIPRP(this);
}
