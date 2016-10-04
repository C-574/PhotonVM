# The Language #
----------------

The PhotonVM source code language is similar to the Assembly language, as it consists of a series of instructions that are combined to create an executable program. An instruction is defined by the name of the instruction to call and a list of up to three parameters, for example:

	instr param1 param2 param3/value   


Every instruction **only** operates on the VM registers and has no stack like other languages, e.g. Java. Also an instruction can only take **one constant value** as a parameter per instruction, if any are supported for the specific instruction.

Parameters can be of different types. The following list shows all parameter types that are supported by the language:

- *Register*: Registers are addressed as **reg0 - reg5, tmp0 - tmp5** and special register **"local"**
- *Constant value*: Scalar and constant values are represented as *#123*. They can range from **0** to **255** and can not be negative.
- *Group Id*: Group ids are used to access a specific Host-Call function-group and have the prefix **"$"**

**Comments**
------------

The Photon language also supports basic line comments to add notes or documentation to Photon source code. A comment starts with `--` and is completely ignored by the Photon compiler when generating executable byte-code. Currently it is **not** allowed to place comments in the same line as an uncommented instruction, it is only valid on a single line:

	invreg reg0 -- Wrong!
	
	-- Right. This is a comment.
	invreg reg0


**Instructions**
----------------

As Photon uses only 16-bits to encode an instruction, it has a very limited but powerful set of instructions. The following table shows all instructions that are currently supported:

<table>
	<tr>
		<td><b>Instruction Code</b></td>
		<td><b>Syntax</b></td>
		<td><b>Description</b></td>
	</tr>
	<tr>
		<td><i>0x00</i></td>
		<td><i>halt</i> <b>[value]</b></td>
		<td>Halts the execution of the virtual machine immediately. The default exit code of <i>0</i> represents success. The host-code can check the result of the execution. See <i>VMExitCodes</i> enumeration for predefined codes.</td>
	</tr>
    <tr>
		<td><i>0x01</i></td>
		<td><i>setreg</i> <b>[destRegister] [value]</b></td>
        <td>Writes a constant value to a specific VM register. To clear a register set it to <i>0</i> (zero). Note that by default all registers are reset to zero.</td>
    </tr>
	<tr>
		<td><i>0x02</i></td>
		<td><i>cpyreg</i> <b>[destRegister] [register]</b></td>
        <td>Copies the content of <i>register</i> to the specified destination register.</td>
    </tr>
	<tr>
		<td><i>0x03</i></td>
		<td><i>addreg</i> <b>[destRegister] [registerA] [registerB]</b></td>
        <td>Adds the value of <i>registerB</i> to the value of <i>registerA</i>. The result is stored in <i>destRegister</i>.</td>
    </tr>
	<tr>
		<td><i>0x04</i></td>
		<td><i>subreg</i> <b>[destRegister] [registerA] [registerB]</b></td>
        <td>Subtracts the value of <i>registerB</i> from the value of <i>registerA</i>. The result is stored in <i>destRegister</i>.</td>
    </tr>
	<tr>
		<td><i>0x05</i></td>
		<td><i>mulreg</i> <b>[destRegister] [registerA] [registerB]</b></td>
        <td>Multiplies the value of <i>registerB</i> with the value of <i>registerA</i>. The result is stored in <i>destRegister</i>.</td>
    </tr>
	<tr>
		<td><i>0x06</i></td>
		<td><i>divreg</i> <b>[destRegister] [registerA] [registerB]</b></td>
        <td>Divides the value of <i>registerB</i> by the value of <i>registerA</i>. The result is stored in <i>destRegister</i>.</td>
    </tr>
	<tr>
		<td><i>0x07</i></td>
		<td><i>invreg</i> <b>[register]</b></td>
        <td>Inverts the sign of the value that is stored in the specified register. Thre result is stored in the same register.</td>
    </tr>
	<tr>
		<td><i>0x08</i></td>
		<td><i>eqreg</i> <b>[destRegister] [registerA] [registerB]</b></td>
        <td>Checks if the value of <i>registerB</i> and the value of <i>registerA</i> are equal. The result is either <i>0</i> or <i>1</i> and   is stored in <i>destRegister</i>.</td>
    </tr>
	<tr>
		<td><i>0x09</i></td>
		<td><i>neqreg</i> <b>[destRegister] [registerA] [registerB]</b></td>
        <td>Checks if the value of <i>registerB</i> and the value of <i>registerA</i> are <b>not</b> equal. The result is either <i>0</i> or <i>1</i> and is stored in <i>destRegister</i>.</td>
    </tr>
	<tr>
		<td><i>0x0A</i></td>
		<td><i>grtreg</i> <b>[destRegister] [registerA] [registerB]</b></td>
        <td>Checks if the value of <i>registerA</i> is grater than the value of <i>registerB</i>. The result is either <i>0</i> or <i>1</i> and is stored in <i>destRegister</i>.</td>
    </tr>
	<tr>
		<td><i>0x0B</i></td>
		<td><i>letreg</i> <b>[destRegister] [registerA] [registerB]</b></td>
        <td>Checks if the value of <i>registerA</i> is less than the value of <i>registerB</i>. The result is either <i>0</i> or <i>1</i> and is stored in <i>destRegister</i>.</td>
    </tr>
	<tr>
		<td><i>0x0C</i></td>
		<td><i>jump</i> <b>[register] [isAbsolute]</b></td>
        <td>Jumps the number of <i>register</i> instructions backward or forward in the instruction queue relative to the current position if <i>isAbsolute</i> is zero (default). Otherwise the jump is absolute to the fist instruction (zero-based). If the value is <i>zero</i> then no jump is executed.</td>
    </tr>
	<tr>
		<td><i>0x0D</i></td>
		<td><i>hcall</i> <b>[groupId] [functionId]</b></td>
        <td>Executes a function in the host-application. The function to call is defined by the group-id and the function-id. For more information on how to use Host-Calls see the topic on <b><i>Host Calls</i></b>.</td>
    </tr>
</table>


Host-Calls
-----------------

A *Host-Call* is a function that resists in the C++ code of the host-application and that can be called by the VM via the ***hcall*** instruction. This instruction takes two parameters to determine the exact function to execute.

The first parameter is the **Group-Id**, this is used to group a number of functions together (mostly for organisation purposes). A group can contain up to 256 different functions.

The second parameter is the **Function-Id** which defines the index of the function in a specific group. Up to *4096* functions can be registered at the same time (total of all groups) and all can be callable via the byte-code. This number can be adjusted if fewer functions are used by the byte-code. 

The byte-code can call a function via:
	
	-- Group-Id: 0 Function-Id: 1
	hcall $0 #1

**For more information about *how to implement a custom Host-Call* see the [Integration Guide](integration-guide/#host-calls "Integration Guide")**


Tips & Tricks
-------------

This section features a list of useful tips and tricks that can be used to write your own Photon scripts. Some of them are used to imitate the behaviour of a higher level language like C/C++ or Java that support control structures like ***if-statements*** or ***for-loops***.

**If-Statement**

`ToDo`   

**For-Loop**

`ToDo`