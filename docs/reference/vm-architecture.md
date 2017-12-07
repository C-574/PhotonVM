# VM Architecture

This section describes the architecture that is used to create the core of the Photon VM.
The core consists of a fixed set of registers that the compiled byte-code can operate on. The following graphic shows an overview of the core architecture of the VM.

![CoreArchitecture](../images/CoreArchitecture.png?raw=true)

!!! danger
	Need to rework the graphic!

Registers
---------

Photon's core consists of *13* different registers of which *12* can be used by the user code or as result registers for more complex host calls. The minimum value that a register can hold is *-2<sup>32</sup>* and the maximum is *2<sup>32</sup>-1*. The registers are grouped into two categories: 

- General use registers. These range from **reg0** to **reg11** and are only used by the user code and no external modification should be made to these registers other than the instructions that are defined by the user.

- Special purpose registers. The register **reg12** (aka. *local*) is only used for short-time storage because it is **not** guaranteed that the value will not get overwritten by one of the following instructions. Use it to transfer data from one instruction to the next.

Note that the VM does **not** support string and floating-point types. If string types are needed, for example as identifier, then use string hashing at compile time level or plain indices instead.

!!! info
    Photon byte-code can not encode values that are greater than 255 or negative but registers can, for example as the result of an addition or using the `:::asm inv` instruction.


## Instruction Encoding
Instruction codes in Photon are encoded into 16-bit unsigned integer values. These contain all data of an instruction so the VM is able to decode and execute it on the fly.

Constants are encoded in the last 8-bits of the instruction, so the VM supports values in a range from *0-255*. The data gets placed from the last-significant-bit position:

	| 7| 6| 5| 4| 3| 2| 1| 0|

Parameters are stored using 4-bits for all:

	|11|10| 9| 8|

The most significant 4 bits are used to store the instruction type:

	|15|14|13|12|

As an example, an instruction that loads a constant value into a VM register would be represented as following:

	set      reg2   30
	-------------------------
	0001     0010   00011110 => 0x121E

Many instructions use three parameters instead of two. The 8-bit constant section then gets split into two 4-bit sections. This works because registers **never** exceed the `[0x0, 0xF]` range so they can be stored using only 4-bits.


## Halt Instruction
The halt instruction is similar to C/C++ `:::c return` or `:::asm exit()`. It indicates an errors in the VM byte-code that can either be emitted by the VM itself, e.g. by an *out of bounds jump* or a *divide by zero*, or from user code by using the `:::asm halt` instruction. By default every script will contain a halt at the end with a parameter of `0`, though it is advised to explicitly halt the VM at the end of script execution.

Photon defines a fixed number of exit codes that can be emitted by the VM itself to report internal errors which should halt the VM immediately. These codes are encoded in the most significant bits while user errors should be encoded in the least significant bits to avoid conflicts. The following table shows all internal exit codes:

| Exit Code | Name                    | Description                                                                                                                                               |
| --------- | ----------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0         | ExitCodeSuccess         | Signals sucessfull execution of the code.                                                                                                                 |
| 251       | ExitCodeHaltRequested   | Signals that the VM should be halted by a user request. This does not mean that the VM has finished execution of the byte-code.                                                                                                                 |
| 252       | ExitCodeDivideByZero    | Signals a division by zero error.                                                                                                                         |
| 253       | ExitCodeJumpOutOfBounds | Signals that the offset of a jump instruction is out of bounds.                                                                                           |
| 254       | ExitCodeRegisterFault   | Signals that the byte-code tried to access an invalid register.                                                                                           |
| 255       | ExitCodeInvalidHostCall | Signals that a host call function was requested but could not be resolved. This will only be signalled if `:::cpp PHOTON_IS_HOST_CALL_STRICT` is enabled. |

## Code Execution
Photon byte-code is stored in a contiguous block of memory as a list of packed 16-bit instruction codes. Execution of this byte-code list will always start at the first instruction and it is guaranteed that all registers are cleared to zero before the first instruction gets executed. The VM will run until either a halt instruction is executed or no more instructions are left to execute. In the latter case success of the execution is assumed. Furthermore when executing any byte-code the implementation will **never** assume that the actual byte-code is correct and should handle invalid execution by halting.

If debug callbacks are used then they get called *after* the instruction got executed.