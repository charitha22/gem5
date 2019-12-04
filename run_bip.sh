./build/ALPHA/gem5.opt \
--debug-flags=CacheReplPolicy \
-d biprp_out \
configs/example/se.py \
--benchmark=mcf \
--cpu-type=DerivO3CPU \
--caches --l2cache \
--l2_size='256kB' \
--l2_assoc=8 \
--l1d_size='16kB' \
--l1i_size='16kB' \
--cache_repl_policy='BIPRP' \
-F 14750000000 \
-s 100000000 \
-W 100000000 \
-I 250000000


echo "Done" | mailx -s "Job Finished" cgusthin@purdue.edu
