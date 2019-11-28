./build/ALPHA/gem5.opt \
--debug-flags=CacheReplPolicy \
-d biprp_out \
configs/example/se.py \
--benchmark=mcf \
--cpu-type=DerivO3CPU \
--caches --l2cache \
--l2_size='512kB' \
--l2_assoc=8 \
--l1d_size='16kB' \
--l1i_size='16kB' \
--cache_repl_policy='BIPRP' \
-F 1000000000 \
-I 500000000

echo "Done" | mailx -s "Job Finished" cgusthin@purdue.edu
