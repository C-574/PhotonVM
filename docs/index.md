# Home #
---------------------

PhotonVM is a register-based virtual machine that is written in C++ with the goals to be very small, in code size and byte code size, while still being a fully functional interpreted language. It was mostly written for educational purposes (and for fun, of course). 

**This site contains all documentation about the Photon VM. To *get started* see the [Integration Guide](manual/integration-guide.md "Integration Guide").**

Features
--------

Some of the features of Photon are:

- Easy to integrate (*header-only library*) with customizable interface for the host application
- No external dependencies (*except* for the C/C++ standard library and a C++11 compatible compiler)
- Small but fully functional instruction set
- Generates very small byte-code output (16-bits per instruction)
- Can compile and run Photon source code or pre-compiled byte-code
- No dynamic memory allocation at byte-code runtime
- MIT licensed


The name *Photon* was chosen because every instruction that can be executed is very small and lightweight, as they  are packed into only 16-bits. 

The code that the Photon compiler packs into a executable format is very Assembly-like and linear. Also the VM is portable between different operating systems (currently only tested on Windows 7/10 and Linux Ubuntu 14.04 LTS).


# Examples #
------------

This section shows a list of examples. These examples can also be found in the `/examples` directory of the VM.

Hello World
-----------

This simple example is kind of the *"Hello World"* of Photon. It shows how to add two values together and print the resulting value to the screen.

	-- Store two values in two registers for addition.
	setreg reg0 #3
	setreg reg1 #7

	-- Let the VM add both registers together and store the result in another register, 'local' in this case.
	addreg local reg0 reg1
	
	-- Call the host application to output the calculated value.
	hcall $0 #2
	
	-- Halt the execution of the VM.
	halt 

Fibonacci
---------

This example shows how to compute the Fibonacci sequence to a specific number of iterations.
This is a pretty complex example as it makes use of the `jump` instruction and the special `local` register to imitate control structures such as *while-loops* and *if-statements*. 

The number of iterations of the algorithm is defined by the value of the *reg0* register while result of the Fibonacci sequence computation is stored in the *reg1* register. The script in the example directory also prints the result to the host application's output which is ignored here for simplicity.


	-- Defines the iteration count of the algorithm.
	setreg reg0 #6
	
	-- Define the variables that are used to compute the sequence.
	setreg reg1 #0
	setreg tmp0 #0
	setreg tmp1 #1
	
	
	-- tmp3 = instruction count of loop-block (lS).
	setreg tmp3 #9
	-- tmp2 = start index of loop (i).
	setreg tmp2 #2
	
	-- while(i < N) ...
	grtreg local tmp2 reg0
	mulreg local local tmp3
	jump local
	-- BEGIN - While
	addreg reg1 tmp0 tmp1
	cpyreg tmp0 tmp1
	cpyreg tmp1 reg1
	
	-- Increment the loop counter 'i'.
	setreg local #1
	addreg tmp2 tmp2 local
	-- Jump back to the loop-head.
	setreg local #10
	invreg local
	jump local
	-- END - While
	
	halt 