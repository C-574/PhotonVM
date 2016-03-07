# Using PVM #
-------------

Together with the VM header files also comes a simple implementation of the VM called the **PVM**. This command line program implements all basic functionality that is needed to execute any Photon script. The source can be found in the `/source` directory of the project. To build the *PVM* you have to generate the corresponding project file(s) (*VS 2015/XCode/Makefiles*) for your platform using [CMake](https://cmake.org/).

PVM supports three basic operations that can be executed via command-line parameters:

- Compile from Photon source to byte-code
- Execute Photon source code
- Execute compiled byte-code

PVM uses a slightly different syntax for command line arguments than other applications. Instead of `-option foo` it uses `-option=foo`.
The following sections show how to use PVM on the command line.

Compile to byte-code
--------------------

To compile source code to executable byte-code an input and output file have to be specified. If no output is specified the compiler will default to `./byteCode.pbc`.

At least an input file has to be specified using the `-c` parameter in order to compile to byte-code. Note that the order of the different parameters are not important.

	pvm -c=./MyCode.pho -o=./MyOutput.pbc

Execute source code
-------------------

To compile and run source code in the VM use the following syntax:

	pvm -r=./MyCode.pho

Execute byte-code
-----------------

The syntax of running already compiled byte-code is pretty similar to executing non-compiled code:

	pvm -b=./MyCode.pbc

For more information about the command-line options use the `-h` command of PVM which will show the help message. 