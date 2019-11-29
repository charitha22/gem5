./build/ALPHA/gem5.opt \
--debug-flags=CacheReplPolicy \
-d lrurp_out \
configs/example/se.py \
--benchmark=bzip2 \
--cpu-type=DerivO3CPU \
--caches --l2cache \
--l2_size='512kB' \
--l2_assoc=8 \
--l1d_size='16kB' \
--l1i_size='16kB' \
-F 1000000000 \
-s 100000000 \
-W 100000000 \
-I 250000000




echo "Done" | mailx -s "Job Finished" cgusthin@purdue.edu
