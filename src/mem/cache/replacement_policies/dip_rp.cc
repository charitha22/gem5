

#include "mem/cache/replacement_policies/dip_rp.hh"

#include <cmath>
#include <memory>

#include "base/random.hh"
#include "params/DIPRP.hh"
#include "debug/CacheReplPolicy.hh"

DIPRP::DIPRP(const Params *p)
    : LRURP(p), btp(p->btp),
    no_sets((p->size) / (p->assoc * p->cache_line_size)),
    psel(p->psel_bits,  1 << (p->psel_bits-1)),
    no_offset_bits((uint)std::log2(no_sets / p->sample_size)),
    psel_log_tick(0)
{
    DPRINTF(CacheReplPolicy, "psel init value : %d \n", 1 << (p->psel_bits-1));
    // std::cout << p->assoc << std::endl;
    // std::cout << p->size << std::endl;
    // std::cout << p->cache_line_size << std::endl;
    // std::cout << no_sets << std::endl;

    // assert(no_sets > 0);
    // int np1 = 0, np2 = 0;
    // for (uint i = 0; i < no_sets; i++)
    // {
    //     if (is_policy1_set(i))
    //     {
    //         std::cout << i << " : p1 set" << std::endl;
    //         np1++;
    //     }
    //     else if (is_policy2_set(i))
    //     {
    //         std::cout << i << " : p2 set" << std::endl;
    //         np2++;
    //     }
    // }

    // std::cout << "no of p1 sets = " << np1 << std::endl;
    // std::cout << "no of p2 sets = " << np2 << std::endl;
}

void
DIPRP::reset(const std::shared_ptr<ReplacementData> &replacement_data) const
{
    std::shared_ptr<DIPReplData> casted_replacement_data =
        std::static_pointer_cast<DIPReplData>(replacement_data);
    uint32_t set = casted_replacement_data->set;



    if (is_policy1_set(set)){
        psel++;
        // DPRINTF(CacheReplPolicy, "RESET p1 set %d ," 
        //     "psel value %d\n", set, psel.getCounter());
        reset_policy1(casted_replacement_data);
    }
    else if (is_policy2_set(set)){
        psel--;
        // DPRINTF(CacheReplPolicy, "RESET p2 set %d ,"
        //     "psel value %d\n", set, psel.getCounter());
        reset_policy2(casted_replacement_data);
    }
    else{
        if (psel.getMSB() == 0){
            // DPRINTF(CacheReplPolicy, "RESET follower set using p1 , set %d,"
            //     "psel value %d\n", set, psel.getCounter());
            reset_policy1(casted_replacement_data);
        } 
        else{
            // DPRINTF(CacheReplPolicy, "RESET follower set using p2 , set %d,"
            //     "psel value %d\n", set, psel.getCounter());
            reset_policy2(casted_replacement_data);
        }  
    }

    Tick t = curTick();
    if(t - psel_log_tick > 1000000000){
        DPRINTF(CacheReplPolicy, "value of psel counter at tick %d : %d\n",
                curTick(), psel.getCounter());
        psel_log_tick = t;
    }
    
}

void
DIPRP::reset_policy1(const std::shared_ptr<DIPReplData>& replacement_data) const
{
    // std::cout << "psel = " << psel.getCounter() << std::endl;
    // std::cout << "using p1" << std::endl;
    replacement_data->lastTouchTick = curTick();
}

void
DIPRP::reset_policy2(const std::shared_ptr<DIPReplData>& replacement_data) const
{
    // std::cout << "psel = " << psel.getCounter() << std::endl;
    // std::cout << "using p2" << std::endl;
    if (random_mt.random<unsigned>(1, 100) <= btp)
    {
        replacement_data->lastTouchTick = curTick();
    }
    else
    {
        // Make their timestamps as old as possible, so that they become LRU
        replacement_data->lastTouchTick = 1;
    }
}

std::shared_ptr<ReplacementData>
DIPRP::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new DIPReplData());
}

// if constituency bits == offset bits
bool DIPRP::is_policy1_set(uint32_t set) const
{
    uint32_t sat_val = (1 << no_offset_bits) - 1;
    uint32_t offset = set & sat_val;
    uint32_t trunc_consti_id = (set >> no_offset_bits) & sat_val;
    if (offset == trunc_consti_id)
        return true;

    return false;
}

// if !constituency bits == offset bits
bool DIPRP::is_policy2_set(uint32_t set) const
{
    uint32_t sat_val = (1 << no_offset_bits) - 1;
    uint32_t offset = set & sat_val;
    uint32_t trunc_consti_id = (set >> no_offset_bits) & sat_val;
    if (offset == (~trunc_consti_id & sat_val))
        return true;

    return false;
}

DIPRP *
DIPRPParams::create()
{
    return new DIPRP(this);
}
