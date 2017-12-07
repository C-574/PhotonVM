# C++ Integration Guide
This page describes how the Photon scripting language can be integrated into any C++ project. No additional librarys are required except for the C++ standard library. The process of integrating and customizing Photon with your own project will be described in the following sections.

To use Photon simply include the ``PhotonVM.h`` header file where it is needed. In **one** of the C++ files write the following line before `#include "PhotonVM.h`:
``` cpp
#define PHOTON_IMPLEMENTATION
```
This will expand the actual implementation of the VM.

## Build Options
Photon provides a set of build options that can be overridden to better fit into your project. Define them before `#include "PhotonVM.h` (or define them globally) to override the default settings. The following table shows all available options:

| Name                          | Values | Default   | Description                                                                                                                                                                                                                        |
| ----------------------------- | ------ | --------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| PHOTON_MAX_HOST_CALLS         | 1-4096 | 32        | Total number of Host-Calls that can be registered at once. This can be reduced if fewer calls are used. The maximum number of calls is: 0xFFF = 4095. Note that one group always consists of 256 functions.                        |
| PHOTON_DEBUG_CALLBACK_ENABLED | 0-1    | 0         | Enable or disable the user debug callback on the virtual machine. See the section on [debug callbacks](#debug-callbacks) for more information.                                                                                                                                                              |
| PHOTON_IS_HOST_CALL_STRICT    | 0-1    | 0         | Enable or disable strictness of Host-Calls. If enabled and no Host-Call can be found for a hcall instruction the VM will halt, otherwise it will continue.                                                                         |
| PHOTON_COMPILER_ERROR_STRICT  | 0-1    | 0         | If enabled then the lexer will stop after it encounters an error, otherwise it will continue.                                                                                                                                      |
| PHOTON_NO_COMPILER            | -      | undefined | Defining this disables the internal Photon byte-code compiler.                                                                                                                                                                     |
| PHOTON_STATIC                 | -      | undefined | Defining this makes the implementation private to the source file that generates it.                                                                                                                                               |
| PHOTON_MALLOC_OVERRIDE        | -      | undefined | Defining this will disable the use of `malloc` and `free` for compiler memory allocation. If this is defined it is also required to define `pho_malloc(size)` and `pho_free(ptr)` with custom allocation and deallocation methods. |


## Creating a Virtual Machine
To create a Photon VM simply create a Virtual Machine with the `:::cpp Phtoton::createVirtualMachine(ByteCode byteCode, VerbosityLevel verbosity)` function and pass it the byte-code to execute. Additionally any Host Calls can be registered before calling `:::cpp Photon::run(VirtualMachine* vm)` to kick off the execution.

``` cpp
Photon::VirtualMachine vm = Photon::createVirtualMachine(byteCode, Photon::VerbosityLevelAll);
// Register additional Host Calls here...
Photon::run(&vm);
```

## Compiling Byte-Code
To execute anything on the VM byte-code is required which is a binary list of instructions that tell the VM what to do. As it is pretty difficult to write raw byte-code Photon defines a language that can be compiled into actual executable byte-code. For more information about the syntax of the language see the [language documentation](language.md).

!!! tip
    If byte-code is compiled offline instead and no compiler is required then consider disabling this feature altogether. See the [build options](#build-options) for more information.

To compile any string of Photon source code into byte-code Phtoton provides a `Photon::compile(char* source, const char* fileName)` function which gets passed the source code string to compile and an optional file name for debug output.
The resulting byte-code will always be accepted by the VM but the compiled result may actually be different from the specified input source.

``` cpp
Photon::ByteCode byteCode = Photon::compile(sourceString, "SomeFile.pho");
```

To check if any instruction was generated at all pass the byte-code to the `Photon::isByteCodeValid(ByteCode* byteCode)` function and check the result.
To verify the actual output of the compiler use debug callbacks as described in [this section](#debug-callbacks).

After the VM has finished executing and the byte-code is no longer needed it is recommended to free it. If the internal compiler generated the byte-code then call `Photon::releaseByteCode(ByteCode* byteCode)` to free it.

!!! tip
    Photon does not support loading byte-code or source code from file as this makes the library way more portable. If this is required simply `fread/fwrite` a header block containing metadata about the byte-code and read/write the actual data as a blob.

## Host Calls
Photon's instruction set is very minimal so sometimes it is required to extend it with new functionality that is not existing in Photon. So how does this work? Host calls for the rescue!

Host calls live in the host application (as the name implies) and can be called from a Photon script using the `hcl` instruction. They have the abillity to modify the VM registers and perform operations on them that are not possible in raw Photon instructions.

In this example a simple call is created that takes the value at `reg0`, then squares it and  stores it in `reg1`.
Creating a new Host Call is simple, first a call needs to be defined in the host by using the `HostCallback(name)` macro:
``` cpp
HostCallback(squareValue)
{
	// Get the input of the value to compute from the reg0 register.
	Photon::RegisterType value = registers[Photon::Reg0];

	// Compute the resulting output.
	value = value * value;

	// Store the result in the reg1 register.
	registers[Photon::Reg1] = value;
}
```

Now the call needs to be registered with an instance of the VM. To do this a group ID and function ID is required.

* **Group ID**: Used to group a number of functions together (mostly for organisation purposes). A group can contain up to 256 different functions. Ranges from [0, 15]
* **Function ID**: Defines the index of the function in a specific group. Ranges from [0, 255]

Up to *4096* functions can be registered at the same time (total of all groups) and all can be callable via the byte-code. This number can be adjusted if fewer functions are used by the byte code. 

``` cpp
const uint8_t HC_GROUP_DEFAULT = 0;
const uint8_t HC_FUNCTION_SQUARE = 2;
Photon::registerHostCall(&vm, sqareValue, HC_GROUP_DEFAULT, HC_FUNCTION_SQUARE);
```

Now any script that runs on the VM can now call this function using the following instructions:
``` asm
set reg0 2
hcl 0 2
```

!!! tip
    The maximum number of Host Calls can be changed by defining `PHOTON_MAX_HOST_CALLS`. See the [build options](#build-options) for more info.

## Debug Callbacks
Debug callbacks can be useful when debugging any Photon script. They report the decoded instruction and the current state of all registers after the VM has exeuted the instruction. This information can be used to track bugs in Photon scripts. For this feature to work the `PHOTON_DEBUG_CALLBACK_ENABLED` build option must be enabled. 

For ease of use the `DebugCallback(name)` macro can be used to define a callback function.

!!! attention
    Neither the instruction that was executed nor the registers themselves can be modified by the callback at any time.
The following example shows the use of a custom callback to print the last executed instruction and current value of the `reg0` register to the standard output:

```cpp
DebugCallback(myCallback)
{
	printf("Instruction op code: %d, reg0=%d\n", instruction->opCode, registers[Photon::Reg0]);
}
```

Finally the callback needs to be registered with a virtual machine using the `setDebugCallback` function.
Only one callback can be set to one VM instance at a time.

``` cpp
Photon::setDebugCallback(&vm, myCallback);
```