### ONION CLIENT LIBRARY ###
This is a library and a set of test code files.
The library can be built against C or C++ code.
There are 2 examples: 1 using C++ and 1 using C.
The C example is test.c
The C++ example is test.cpp

### BUILD
To build test code use make in this folder:

make

### RUN
Test application defined in the test.cpp file is created as executable "testExe".
To run:

C++ Example
./cppTest

C Example
./cTest


### Expanding to a specific application
To use this library with a C based project will require that there is also a compatible C++ compiler.
In the make file C is defined as gcc and CC is g++, any cross-compilers needed can be used in place
of gcc and g++.  The c++ compiler is needed for compiling the library and linking the c and c++ code together.

