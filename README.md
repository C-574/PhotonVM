![PhotonVM_Logo](docs/images/Photon_Logo_Text.png?raw=true)

[![GitHub license](https://img.shields.io/github/license/mashape/apistatus.svg)](https://github.com/C-574/PhotonVM/blob/master/LICENSE)

---------------------

# Home
PhotonVM is a register-based virtual machine that is written in C++ with the goals to be very small, in code size and byte code size, while still being a fully functional interpreted language. It was mostly written for educational purposes (and for fun, of course). 

- View the [PhotonVM Documentation](http://C-574.github.io/PhotonVM "PhotonVM Documentation")


# Features
Some of the features of Photon are:

- Easy to integrate (*only a single file*) with customizable interface for the host application
- No external dependencies (*except* for the C/C++ standard library and a C++11 compatible compiler)
- Small but fully functional instruction set
- Generates very small byte-code output (16-bits per instruction)
- Can compile and run Photon source code or pre-compiled byte-code
- No dynamic memory allocation at byte-code runtime 
- MIT licensed

The name *Photon* was chosen because every instruction that can be executed is very small and lightweight, as they are packed into only 16-bits. 

The code that the Photon compiler packs into an executable format is very Assembly-like and linear. Also the VM is portable between different operating systems (currently tested on MSVC 2015).


# Examples
This section shows a list of examples on how to use Photon source code to do some simple things.

For a more complete example on how to use Photon on the C++ side, see the example files in the `/src` directory. The PVM sample (`pvm.cpp`) will show how to compile byte-code and execute it using Photon including some utillity functions.

## Hello World

This simple example is kind of the *"Hello World"* of Photon. It shows how to add two values together and print the resulting value to the screen.
``` asm
# Store two values in two registers for addition.
set reg0 3
set reg1 7

# Let the VM add both registers together and store the result in another register, 'local' in this case.
add reg3 reg0 reg1

# Call the host application to output the calculated value to the console.
hcl 0 2

# Halt the execution of the VM.
halt 0
```

## Fibonacci

This example shows how to compute the Fibonacci sequence to a specific number of iterations.
This is a pretty complex example as it makes use of the `jmp` instruction to imitate a while-loop. 

The number of iterations of the algorithm is defined by the value of the *reg0* register while the result of the Fibonacci sequence computation is stored in the *reg1* register. This script ignores printing the result to the host application's output for simplicity.
``` asm
# Mapping Table:
# -----------------
# N:        | reg0
# Fib       | reg1
# FibN-1    | reg2
# FibN-2    | reg3
# i         | reg4 
# blockSize | reg5
# local <= reg12
# ------------------

# Defines the iteration count of the algorithm.
set reg0 18

# Define the variables that are used to compute the sequence.
set reg1 0
set reg2 0
set reg3 1


# reg5 = instruction count of loop-block.
set reg5 9
# reg4 = start index of loop (i).
set reg4 2

# while(i < N) ...
gre reg12 reg4 reg0
mul reg12 reg12 reg5
jmp reg12 0
# BEGIN - While
	add reg1 reg2 reg3
	cpy reg2 reg3
	cpy reg3 reg1

	# Increment the loop counter 'i'.
	set reg12 1
	add reg4 reg4 reg12
	# jump back to the loop-head.
	set reg12 10
	inv reg12
	jmp reg12 0
# END - While

halt 0
```