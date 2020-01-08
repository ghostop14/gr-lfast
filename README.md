# gr-lfast - Timed, tuned, and accelerated GNURadio blocks

## Overview
The goal of the gr-lfast project is to increase flowgraph performance while running on a general purpose CPU.  So far the project uses several techniques to achieve the performance boosts:

1.  No algorithm changes are made, but common C++ optimization techniques could be applied to increase overall throughput.
2.  Block consolidation at the code level rather than a hierarchical block (reusing variables and buffers without needing to recreate per block).
3.  Fused Multiply/Add CPU execution when supported.
3.  Multi-threading is used where possible (FIR filters for example).
4.  For filters, the OpenCL gr-clenabled project timing study called out that the GNURadio FFT filters are faster in some instances compared to the FIR versions.  However the standard convenience wrappers for low pass, high pass, and root-raised cosine among others are using the FIR filters.  Therefore convenience wrappers around the FFT equivalents are included here as well.

## Included Blocks

1.  Costas Loop (2nd and 4th Orders)
2.  AGC Control (Complex and Float)
3.  FFT-based Low Pass Filter Convenience Wrapper
4.  FFT-based High Pass Filter Convenience Wrapper
5.  FFT-based Root Raised Cosine Filter Convenience Wrapper
6.  Aggregated block that does Complex to Real->Byte->Vector in a single C++ implementation

7.  Log Block (n*log10(x) + k) implemented with Volk [Note that as of GNU Radio 3.8, the volk approach is now in the standard block]
8.  Multi-threaded FIR filters

## Command-line tools

The project includes a command-line tool called test-lfast which will provide timing on the non-filter blocks.  The timing output shows the original GNURadio block throughput along with the new optimized throughput on your specific hardware.  Since so many factors can go into the resulting timing, it's a good idea to run this on the specific system you'll be using the blocks on.

## Building
gr-lfast is available in the pybombs repository.  However to build gr-lfast from source, simply follow the standard module build process.  Git clone it to a directory, close GNURadio if you have it open, then use the following build steps:

cd <clone directory>

mkdir build

cd build

cmake ..

make

[sudo] make install

sudo ldconfig

If each step was successful (do not overlook the "sudo ldconfig" step).

## Details
In terms of code optimization, gr-lfast focuses on using basic C++ code optimization techniques such as eliminating stack pushes associated with function jumps, Fused Multiply/Add (FMA) operations if the CPU supports it in hardware, eliminating unnecessary loops, and other techniques to increase overall throughput without the need to rewrite any of the signal processing algorithms themselves.  

For instance the native 2nd order Costas Loop module running on an i7-6700 clocked at processing about 22.2 Msps.  After optimizing the code
the 2nd order loop was capable of processing almost 38 Msps (a 71% speed increase).  A 4th order loop went from about 21.8 Msps to almost 33 Msps (a 50.6% improvement).  
  
The same approach was applied to the AGC block for about a 17-20% speed increase to about 104 MSPS.  

The block Complex->Real->Char->Vector combines Complex->Real, Float->char, and stream->Vector in a single block, saving on buffer copies 
and multiple threads.  Overall speedup on that block was nominal, 2-3%.  

Note there is a significant speed increase in log calculations with some math refactoring to take advantage of a volk log2 function to calculate the log10 (n*log10(x) = n*log10(x) as n*log2(x)/log2(10) = (n/log2(10)) * log2(x)).  This resulted in a 350% speed increase.

FIR filters were also optimized to take advantage of multi-threading.  The test-lfast tool can be used as shown below to determine the optimal number of threads given your CPU and number of taps.  Generally 3-4 threads is a good place to be.

The plan is to add more blocks as I run into needing them.

The following output from running 'test-lfast' shows the speed increases on a newer laptop with an Intel i7-7700HQ 7th Gen processor.

Testing 2nd order Costas Loop with 8,192 samples...
Original Code Run Time:      0.000544 s  (15,054,931.000000 sps)
LFAST Code Run Time:      0.000202 s  (40,552,132.000000 sps)
Speedup:        169.36% faster

Testing 4th order Costas Loop with 8,192 samples...
Original Code Run Time:      0.000373 s  (21,946,850.000000 sps)
LFAST Code Run Time:      0.000240 s  (34,098,004.000000 sps)
Speedup:         55.37% faster

----------------------------------------------------------
Testing AGC with 8,192 samples...
Original Code Run Time:      0.000093 s  (87,887,608.000000 sps)
LFAST Code Run Time:      0.000075 s  (109,181,128.000000 sps)
Speedup:         24.23% faster

----------------------------------------------------------
Testing Complex->Real->Char->Vector with 8,192 samples...
Original Code Run Time:      0.000007 s  (1,207,321,088.000000 sps)
LFAST Code Run Time:      0.000007 s  (1,242,644,864.000000 sps)
Speedup:          2.93% faster

----------------------------------------------------------
Testing volk nlog10+k with 8,192 samples...
Original Code Run Time:      0.000135 s  (60,607,308.000000 sps)
LFAST Code Run Time:      0.000041 s  (199,728,688.000000 sps)
Speedup:        229.55% faster

----------------------------------------------------------
Testing FIR filter with complex data and float taps with 241 taps, 8,192 samples...
Original Code Run Time:      0.000447 s  (18,344,648.000000 sps)
LFAST Code Run Time [1 threads]:      0.000646 s  (12,673,606.000000 sps)
1-thread Speedup:        -30.91% faster
LFAST Code Run Time [2 threads]:      0.000287 s  (28,563,974.000000 sps)
2-thread Speedup:         55.71% faster
LFAST Code Run Time [3 threads]:      0.000294 s  (27,822,952.000000 sps)
3-thread Speedup:         51.67% faster
LFAST Code Run Time [4 threads]:      0.000268 s  (30,516,536.000000 sps)
4-thread Speedup:         66.35% faster
LFAST Code Run Time [5 threads]:      0.000347 s  (23,585,654.000000 sps)
5-thread Speedup:         28.57% faster
LFAST Code Run Time [6 threads]:      0.000291 s  (28,183,880.000000 sps)
6-thread Speedup:         53.64% faster
LFAST Code Run Time [7 threads]:      0.000386 s  (21,199,370.000000 sps)
7-thread Speedup:         15.56% faster
LFAST Code Run Time [8 threads]:      0.000339 s  (24,150,214.000000 sps)
8-thread Speedup:         31.65% faster

Fastest filter thread performance:
Number of Samples in test case (can be varied with command-line parameter): 8,192
Number of taps in filter (can be varied with command-line parameter): 241
Fastest Thread Count: 4
Original SPS: 18,344,648.00
Fastest SPS: 30,516,536.00
Speedup: 66.35% faster



