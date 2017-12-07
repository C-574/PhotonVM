# The Language
The PhotonVM source code language is similar to the Assembly language, as it consists of a series of instructions that are combined to create an executable program. An instruction is defined by the name of the instruction to call and a list of up to three parameters, for example:
``` asm
	instr param1 param2 param3   
```

Every instruction only operates on the VM registers and has no stack or dynamic memory like other languages. Also an instruction can only take either up to three registers or a single constant as a parameter per instruction, if any are supported for the specific instruction.

Parameters can be of tow types:

- *Register*: Registers are addressed as ``reg0 - reg12`` or ``r0 - r12``
- *Constant value*: Constants are represented as `123`. The valid range for constants is [0, 255]

The order of execution is linear, so the first instruction in the source or byte-code will be the first one to be executed (FIFO).

## Comments
The Photon language also supports basic line comments to add notes or documentation to Photon source code. A comment starts with `#` and is completely ignored by the Photon compiler when generating executable byte-code. Multiline comments are not supported.
``` asm
	# This is a comment.
	inv reg0

	inv reg0 # A comment after an instruction.
```

## Instructions
As Photon uses only 16-bits to encode an instruction, it has a very limited but powerful set of instructions. The following table shows all instructions that are currently supported:

| Op Code | Syntax                                         | Description                                                                                                                                                                                                                                                                                |
| ------- | ---------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| 0x0    | halt **[exitCode]**                            | Halts the execution of the virtual machine immediately. A value of `0` represents success. The host can check the result of the execution. See `VMExitCodes` enumeration for predefined codes.                                                                               |
| 0x1     | set **[destRegister] [value]**                 | Writes a constant value to a specific VM register. To clear a register set it to `0` (zero). Note that by default all registers are reset to zero.                                                                                                                                         |
| 0x2     | cpy **[destRegister] [register]**              | Copies the content of *register* to the specified destination register.                                                                                                                                                                                                                    |
| 0x3     | add **[destRegister] [registerA] [registerB]** | Adds the value of *registerB* to the value of *registerA*. The result is stored in *destRegister*.                                                                                                                                                                                         |
| 0x4     | sub **[destRegister] [registerA] [registerB]** | Subtracts the value of *registerB* from the value of *registerA*. The result is stored in *destRegister*.                                                                                                                                                                                  |
| 0x5     | mul **[destRegister] [registerA] [registerB]** | Multiplies the value of *registerB* with the value of *registerA*. The result is stored in *destRegister*.                                                                                                                                                                                 |
| 0x6     | div **[destRegister] [registerA] [registerB]** | Divides the value of *registerB* by the value of *registerA*. The result is stored in *destRegister*. If *registerB* is zero the VM will halt with a "Division by zero".                                                                                                                   |
| 0x7     | inv **[register]**                             | Inverts the sign of the value that is stored in the specified register. Thre result is stored in the same register.                                                                                                                                                                        |
| 0x8     | eql **[destRegister] [registerA] [registerB]** | Checks if the value of *registerB* and the value of *registerA* are equal. The result is either `0` or `1` and is stored in *destRegister*.                                                                                                                                                |
| 0x9     | neq **[destRegister] [registerA] [registerB]** | Checks if the value of *registerB* and the value of *registerA* are not equal. The result is either `0` or `1` and is stored in *destRegister*.                                                                                                                                            |
| 0xA     | gre **[destRegister] [registerA] [registerB]** | Checks if the value of *registerA* is greater than the value of *registerB*. The result is either `0` or `1` and is stored in *destRegister*.                                                                                                                                              |
| 0xB     | les **[destRegister] [registerA] [registerB]** | Checks if the value of *registerA* is less than the value of *registerB*. The result is either `0` or `1` and is stored in *destRegister*                                                                                                                                                  |
| 0xC     | jmp **[register] [isAbsolute]**                | Jumps the number of in *register* stored instructions backward or forward in the instruction queue relative to the current position if *isAbsolute* is zero (default). Otherwise the jump is absolute to the fist instruction (zero-based). If the value is zero then no jump is executed. |
| 0xD     | hcl **[groupId] [functionId]**                 | Executes a function in the host application. The function to call is defined by *groupId* and *functionId*. For more information on how to use Host Calls see the topic on [Host Calls](integration-guide/#host-calls).                                                                    |


## Tips & Tricks

This section features a list of useful tips and tricks that can be used to write your own Photon scripts. Some of them are used to imitate the behaviour of a higher level language like C/C++ or Java that support control structures like ***if-statements*** or ***for-loops***.

!!! missing
	This section is incomplete and will be changed later. Sorry.

### If-Statement

### For-Loop