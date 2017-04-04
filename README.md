# gr-lfast - Timed, tuned, and accelerated GNURadio blocks

gr-lfast builds on gr-clenabled by focusing on CPU-only blocks and looking for optimizations.  For instance the Costas Loop module clocked at about 24 MSPS
throughput.  However to process wider signals, this block would need to process more data.  Optimizations were identified in the code that increased 
block throughput to 38 MSPS (almost 58% speed improvement).  More blocks will be added as they are tested.


After building, in the build directory's lib subdirectory will be a file called test-lfast that will display metrics for before and after tuning on your system.  The following shows the throughput on an i7-6700 CPU @ 3.40GHz:

Testing Costas Loop with 8192 samples...

Original Code Run Time:      0.000339 s  (24134816.000000 sps)

LFAST Code Run Time:      0.000215 s  (38078292.000000 sps)

Speedup:         57.77% faster


To build gr-lfast, simply follow the standard module build process.  Git clone it to a directory, close GNURadio if you have it open, then use the following build steps:

cd <clone directory>

mkdir build

cd build

cmake ..

make

sudo make install

sudo ldconfig

If each step was successful (don’t overlook the ‘sudo ldconfig’ step).

