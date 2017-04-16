# PICON

## Overview

Picon is an implementation of Control Flow Integrity protection made
of two parts. The first part is a standalone program, called
picon-monitor, that monitors program execution to ensure the integrity
of its execution flow according to its expected control flow
integrity. The last part is a LLVM compiler pass to instrument a
program to be monitored in order to inject communication routines and
message exchanges with the external picon-monitor.

Picon has been published in SSTIC 2015. Article, video and slides are
[available here](https://www.sstic.org/2015/presentation/control_flow_integrity_on_llvm_ir/)

## Prerequisites

A working LLVM installation (either installed from LLVM sources, or
from the packages of a Linux distribution) is required to build the
LLVM pass.  A C compiler is required to build the monitor.  Clang is
also used to build examples, and Python to run the prelink script.

The current code has been developed and tested against
version 3.8. More recent versions are expected to work, but may
require some changes to match the changes in LLVM API.

## Building the project

The toplevel `Makefile` can be used to build the LLVM pass and the
monitor.  Run `make` to build the projects.

The following files can be edited to change or set options:

* `Makefile`:
    * Name of the `llvm-config` and `opt` tools (with versions if
      needed)
* `monitor/Makefile`:
    * `TIMEOUT`: if set, enables the timeout feature of the monitor,
      that is the maximum delay before the monitor kills the program
      if no signal is received; in this case the variables
      `TIMEOUT_SEC_MONITOR_NO_SIGNAL` and
      `TIMEOUT_NANOSEC_MONITOR_NO_SIGNAL` must also be set (see
      `struct timespec` in `man 2 nanosleep`)
    * `NB_PRELOADED_MONITOR_OK_ANSWERS=<n>`: preload `n` answers in
      the monitor answer pipe, that is the number of signals the
      program will send and be able to continue processing in parallel
      (i.e. control flow integrity checks in the monitor are
      desynchronized by exactly `n` signals)
    * `SILENT` : if set, no output (debug or error) is produced by the
      monitor, even in case of compromission of control flow integrity
      of the monitored program
    * `CFI_DEBUG` : if set, the monitor will display (a lot of) debug
      output

## Usage

The overall process is:

* build each source file into LLVM bitcode, using `clang -emit-llvm`
* run the Picon LLVM pass on each bitcode file, to produced the
  processed bitcode file, and the temporary Picon files
* run `picon-prelink` on the temporary picon files to produce the
  injected source code
* build the injected source code, and all processed bitcode files to
  object files
* link all object files (including the injected code)

The resulting binary *must* be executed by the Picon monitor.

As depicted in `examples/unit_tests/Makefile.cfi`, the Picon LLVM pass
requires several options:

* `-cfi` instructs the `opt` tool to actually pass through the Picon
  instrumentation
* `-cfi-prefix=<path/basename>`: path + basename of all files produced
  by the LLVM pass. LLVM must have the permission to write files in this
  path, and the same path and basename options must be used for all
  files built that will be part of the same binary (shared library or
  executable)
* `-cfi-level=[0|1]` defines the level of instrumentation/protection
    * 0 : protection of functions calls only
    * 1 : protection of functions calls and basic blocks transitions
* `-cfi-ignore=<file>` gives a list of function names (one per line)
  which were not been instrumented/protected with Picon LLVM pass and
  for which functions calls cannot be protected

The LLVM pass is an incremental process: files produced using the
`-cfi-prefix` option are read and updated by each compilation. These
files must be deleted when cleaning the project, to ensure that a
rebuild will work properly.

After the compilation of all source files of a project, the files
produced by the `-cfi-prefix` option must be analyzed before linking the
binary. This is done by the `picon-prelink` script, which takes the
`path/basename` option as argument.
`picon-prelink` parses the files and will produced a source file in its
standard output, containing the structures and code required by the
Picon monitor at runtime. This source file must be compiled, and
included in the link process of the binary.


## Examples

The examples can be built by running `make` in the `examples`
directory, after building and installing the main project.

Note that some examples are expected to fail.

The example file `unit_tests/src/test_no_basic_blocks.c` shows how to
suspend basic blocks transitions in a given function using the
`CFI_IGNORE_BASIC_BLOCKS` directive (declared in `picon/picon.h`).

## Authors

Picon has been developed by the following authors:

* Pierre Chifflier
* Thomas Coudray
* Arnaud Fontaine

## License

Picon is licensed under the GNU Lesser General Public License,
version 2.1 or (at your option) any later version. As such, it is
provided without any warranty, and in the hope that it will be useful.

