# gr-lfast - Timed, tuned, and accelerated GNURadio blocks

## Overview
The goal of the gr-lfast project is to increase flowgraph performance while running on a general purpose CPU.  So far the project takes 3 techniques to achieve the performance boosts:

1.  No algorithm changes are made, but common C++ optimization techniques could be applied to increase overall throughput.
2.  Block consolidation at the code level rather than a hierarchical block
3.  For filters, the OpenCL gr-clenabled project timing study called out that the GNURadio FFT filters are much faster than the FIR versions.  However the standard convenience wrappers for low pass, high pass, and root-raised cosine among others are using the FIR filters.  Therefore convenience wrappers around the FFT equivalents are included here as well.

**New** An approach to bring Volk SIMD to the log10 block has been added as well

## Included Blocks

1.  Costas Loop (2nd and 4th Order)
2.  AGC Control (Complex and Float)
3.  FFT-based Low Pass Filter Convenience Wrapper
4.  FFT-based High Pass Filter Convenience Wrapper
5.  FFT-based Root Raised Cosine Filter Convenience Wrapper
6.  Aggregated block that does Complex to Real->Byte->Vector in a single C++ implementation
**New** 7.  n*log10(x) + k implemented with Volk

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

The plan is to add more blocks as I run into needing them.

After building, in the build directory's lib subdirectory will be a file called test-lfast that will display metrics for before and after 
tuning on your system.  The following shows the throughput on a laptop with an Intel i7-7700HQ 7th Gen processor.

Testing 2nd order Costas Loop with 8192 samples...
Original Code Run Time:      0.000613 s  (13358778.000000 sps)
LFAST Code Run Time:      0.000285 s  (28697090.000000 sps)
Speedup:        114.82% faster

Testing 4th order Costas Loop with 8192 samples...
Original Code Run Time:      0.000512 s  (15993678.000000 sps)
LFAST Code Run Time:      0.000327 s  (25063888.000000 sps)
Speedup:         56.71% faster

----------------------------------------------------------
Testing AGC with 8192 samples...
Original Code Run Time:      0.000119 s  (69126912.000000 sps)
LFAST Code Run Time:      0.000110 s  (74262144.000000 sps)
Speedup:          7.43% faster

----------------------------------------------------------
Testing Complex->Real->Char->Vector with 8192 samples...
Original Code Run Time:      0.000005 s  (1640657536.000000 sps)
LFAST Code Run Time:      0.000005 s  (1660787456.000000 sps)
Speedup:          1.23% faster

----------------------------------------------------------
Testing nlog10+k with 8192 samples...
Original Code Run Time:      0.000161 s  (50971952.000000 sps)
LFAST Code Run Time:      0.000006 s  (1453247488.000000 sps)
Speedup:       2751.07% faster



