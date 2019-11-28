./build/ALPHA/gem5.opt \
--debug-flags=CacheReplPolicy \
-d diprp_out \
configs/example/se.py \
--benchmark=mcf \
--cpu-type=DerivO3CPU \
--caches --l2cache \
--l2_size='512kB' \
--l2_assoc=8 \
--l1d_size='16kB' \
--l1i_size='16kB' \
--cache_repl_policy='DIPRP' \
-F 1000000 \
-s 1000000 \
-W 1000000 \
-I 1000000

echo "Done" | mailx -s "Job Finished" cgusthin@purdue.edu
