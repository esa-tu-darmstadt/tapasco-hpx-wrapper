# TaPaSCo HPX Wrapper

## Overview

The TaPaSCo HPX Wrapper allows to use TaPaSCo FPGA and AI Engine tasks transparently in HPX applications. It adopts the light-weight threading model of HPX in contrast to multi-threading on OS level when using TaPaSCo directly.

## Usage

In your `CMakeLists.txt`, include HPX, TaPaSCo and TaPaSCoHPX and link against all required libraries:

```
find_package(HPX REQUIRED)
find_package(Tapasco REQUIRED)
find_package(TapascoHPX REQUIRED)

target_link_libraries(your_executable HPX::hpx HPX::wrap_main tapasco TapascoHPX)
```

In your application use `#include <tapasco-hpx.hpp>` to include the HPX wrapper, which will also include the `tapasco.hpp` header file. At the beginning of your application, instantiate the `Tapasco`object:

```
tapasco::Tapasco tapasco;
```

If you have multiple FPGAs in your host computer, you need to pass the ID of your device when creating the object:

```
tapasco::Tapasco tapasco(tapasco::tlkm_access::TlkmAccessExclusive, dev_id);
```

You can then launch the TaPaSCo task by calling the provided wrapper function. Pass the TaPaSCo object, ID of the requested PE-type and all arguments to the wrapper function:

```
tapasco_hpx_wrapper(tapasco, pe_type, arg1, arg2, ...);
```

Accepted argument types are the same as for the reqular `tapasco.launch()` call. The wrapper function will handle and pass the arguments to the PE, start the PE and wait for the interrupt, while suspending the HPX light-weight thread in-between if necessary.

Please, consult the provided [examples](examples) and [TaPaSCo C++ examples](https://github.com/esa-tu-darmstadt/tapasco/tree/master/runtime/examples/C%2B%2B) for more details.

### Data transfers

If you need to transfer larger data buffers, we recommend to do allocations and data transfers in separate tasks using the `io_pool_executor` of HPX. The TaPaSCo API provides the `alloc()` and `free()` calls for memory management, and `copy_to()` and `copy_from()` for data transfers. Have a look in the [arraysum-pipeline example](examples/arraysum-pipeline). 

## Examples

In order to build and run the provided [examples](examples), first clone and build [TaPaSCo](https://github.com/esa-tu-darmstadt/tapasco) as described in the [Readme](https://github.com/esa-tu-darmstadt/tapasco/blob/master/README.md). The examples require a bitstream containing at least one `arraysum` kernel, the [arraysum-pipeline](examples/arraysum-pipeline) example also `arrayupdate` and `arrayinit` kernels. All three kernels are already provided by TaPaSCo. You can build the required bitstream (e.g. for the Alveo U280) using:

```
tapasco compose [arrayinit x 5, arrayupdate x 5, arraysum x 5] @ 250 MHz -p AU280
```

To build the example software, export the following variables so that CMake finds all required modules:

```
export HPX_DIR=/path/to/HPX
export TapascoHPX_DIR=/path/to/tapasco-hpx-wrapper
source /path/to/tapasco-workspace/tapasco-setup.sh
```

Then switch to the example directory, create a build directory and build with CMake:

```
cd /path/to/example/directory
mkdir build && cd build
cmake .. && make
```

Load the bitstream with

```
tapasco-load-bitstream your_bitstream.bit --reload-driver
```

and run the example application.

## Publication

If you want to cite this work, please use the following information:

[Kalkhof2024] Torben Kalkhof, Carsten Heinz, and Andreas Koch. 2024. **Enabling FPGA and AI Engine Tasks in the HPX Programming Framework for Heterogeneous High-Performance Computing**. In *Applied Reconfigurable Computing. Architectures, Tools, and Applications (ARC).* DOI: [10.1007/978-3-031-55673-9_6](http://dx.doi.org/10.1007/978-3-031-55673-9_6)

## Important Note

Please, use the [develop branch](https://github.com/esa-tu-darmstadt/tapasco/tree/develop) of TaPaSCo until the next official release.



