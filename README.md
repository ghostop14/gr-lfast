# gr-lfast - Timed, tuned, and accelerated GNURadio blocks

gr-lfast builds on gr-clenabled by focusing on CPU-only blocks and looking for optimizations that can either allow the current native gnuradio
code to run with faster throughput, or run with a lower load on existing CPU's.  Using basic C++ code optimization (minimizing function jumps,
timing each line and looking for the heaviest CPU-intensive calls, etc.), it was possible to squeeze significant performance boosts without 
the need to rewrite any of the signal processing algorithms.  

For instance the native 2nd order Costas Loop module running on an i7-6700 clocked at processing about 22.2 Msps.  After optimizing the code
the 2nd order loop was capable of processing almost 38 Msps (a 71% speed increase).  A 4th order loop went from about 21.8 Msps to almost 33 Msps
(a 50.6% improvement).  
  
The same approach was applied to the AGC block for about a 12% speed increase from 91 MSPS to about 102.7 MSPS.  

The block Complex->Real->Char->Vector combines Complex->Real, Float->char, and stream->Vector in a single block, saving on buffer copies 
and multiple threads.  Overall speedup on that block was nominal, 2-3%.  

The plan is to add more blocks as I run into needing them.

After building, in the build directory's lib subdirectory will be a file called test-lfast that will display metrics for before and after 
tuning on your system.  The following shows the throughput on an i7-6700 CPU @ 3.40GHz:

----------------------------------------------------------

Testing 2nd order Costas Loop with 8192 samples...
Original Code Run Time:      0.000369 s  (22218604.000000 sps)
LFAST Code Run Time:      0.000216 s  (37982040.000000 sps)
Speedup:         70.95% faster

Testing 4th order Costas Loop with 8192 samples...
Original Code Run Time:      0.000374 s  (21877034.000000 sps)
LFAST Code Run Time:      0.000249 s  (32946308.000000 sps)
Speedup:         50.60% faster

----------------------------------------------------------
Testing AGC with 8192 samples...
Original Code Run Time:      0.000090 s  (91348728.000000 sps)
LFAST Code Run Time:      0.000080 s  (102704472.000000 sps)
Speedup:         12.43% faster

----------------------------------------------------------
Testing Complex->Real->Char->Vector with 8192 samples...
Original Code Run Time:      0.000005 s  (1806749312.000000 sps)
LFAST Code Run Time:      0.000004 s  (1854870912.000000 sps)
Speedup:          2.66% faster


To build gr-lfast, simply follow the standard module build process.  Git clone it to a directory, close GNURadio if you have it open, then use the following build steps:

cd <clone directory>

mkdir build

cd build

cmake ..

make

[sudo] make install

sudo ldconfig

If each step was successful (do not overlook the "sudo ldconfig" step).

