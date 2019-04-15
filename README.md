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

**New** 7.  n*log10(x) + k implemented with Volk
**New** 8.  Multi-threaded FIR filters

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
  
The same approach was applied to the AGC block for about a 12% speed increase from 91 MSPS to about 102.7 MSPS.  

The block Complex->Real->Char->Vector combines Complex->Real, Float->char, and stream->Vector in a single block, saving on buffer copies 
and multiple threads.  Overall speedup on that block was nominal, 2-3%.  

Note there is a significant speed increase in log calculations with some math refactoring to take advantage of a volk log2 function to calculate the log10 (n*log10(x) = n*log10(x) as n*log2(x)/log2(10) = (n/log2(10)) * log2(x)).  This resulted in a 350% speed increase.

FIR filters were also optimized to take advantage of multi-threading.  The test-lfast tool can be used as shown below to determine the optimal number of threads given your CPU and number of taps.  Generall 3-4 threads is a good place to be.

The plan is to add more blocks as I run into needing them.

The following output from running 'test-lfast' shows the speed increases on a newer laptop with an Intel i7-7700HQ 7th Gen processor.

Testing 2nd order Costas Loop with 8,192 samples...
Original Code Run Time:      0.000571 s  (14,340,659.000000 sps)
LFAST Code Run Time:      0.000271 s  (30,281,824.000000 sps)
Speedup:        111.16% faster

Testing 4th order Costas Loop with 8,192 samples...
Original Code Run Time:      0.000531 s  (15,418,123.000000 sps)
LFAST Code Run Time:      0.000315 s  (26,030,892.000000 sps)
Speedup:         68.83% faster

----------------------------------------------------------
Testing AGC with 8,192 samples...
Original Code Run Time:      0.000157 s  (52,168,060.000000 sps)
LFAST Code Run Time:      0.000143 s  (57,258,728.000000 sps)
Speedup:          9.76% faster

----------------------------------------------------------
Testing volk nlog10+k with 8,192 samples...
Original Code Run Time:      0.000175 s  (46,798,640.000000 sps)
LFAST Code Run Time:      0.000039 s  (209,097,200.000000 sps)
Speedup:        346.80% faster

----------------------------------------------------------
Testing FIR filter with complex data and float taps with 241 taps, 8,192 samples...
Original Code Run Time:      0.000456 s  (17,961,178.000000 sps)
LFAST Code Run Time [1 threads]:      0.000487 s  (16,833,790.000000 sps)
1-thread Speedup:         -6.28% faster
LFAST Code Run Time [2 threads]:      0.000283 s  (28,978,810.000000 sps)
2-thread Speedup:         61.34% faster
LFAST Code Run Time [3 threads]:      0.000232 s  (35,335,860.000000 sps)
3-thread Speedup:         96.73% faster
LFAST Code Run Time [4 threads]:      0.000198 s  (41,386,656.000000 sps)
4-thread Speedup:        130.42% faster
LFAST Code Run Time [5 threads]:      0.000265 s  (30,883,978.000000 sps)
5-thread Speedup:         71.95% faster
LFAST Code Run Time [6 threads]:      0.000299 s  (27,423,570.000000 sps)
6-thread Speedup:         52.68% faster
LFAST Code Run Time [7 threads]:      0.000267 s  (30,719,422.000000 sps)
7-thread Speedup:         71.03% faster
LFAST Code Run Time [8 threads]:      0.000342 s  (23,955,840.000000 sps)
8-thread Speedup:         33.38% faster

Fastest filter thread performance:
Number of Samples in test case (can be varied with command-line parameter): 8,192
Number of taps in filter (can be varied with command-line parameter): 241
Fastest Thread Count: 4
Original SPS: 17,961,178.00
Fastest SPS: 41,386,656.00
Speedup: 130.42% faster



