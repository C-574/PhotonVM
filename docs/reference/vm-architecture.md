# VM Architecture #
---------------------

This section describes the architecture that is used to create the core of the Photon VM.
The core consists of a list of registers that the compiled byte-code can operate on. The following graphic shows an overview of the core architecture of the VM.

![CoreArchitecture](../images/CoreArchitecture.png?raw=true)

Registers
---------

Photon's core consists of *13* different registers of which *12* can be used by the user code or as result registers for more complex host calls. The minimum value that a register can hold is *-2<sup>32</sup>* and the maximum is *2<sup>32</sup>-1*. The registers are grouped into three categories: 

- General use registers. These range from **reg0** to **reg5** and are only used by the user code and no external modification should be made to these registers other than the instructions that are defined by the user.

- Temporary registers. These range from **tmp0** to **tmp5** and can be used for host calls or other computations as output or working registers. It is not guaranteed that a value that is set by the user will *not* get  overwritten by any external code.

- Special purpose registers. These include the **local** register. This register is only used for short-time storage because it is **not** guaranteed that the value will not get overwritten by one of the following instructions. Use it to transfer data from one instruction to the next.

Note that the VM does **not** support string and floating-point types. If string types are needed, for example as identifier, then use string hashing at compile time level or plain indices instead. 


Instruction Encoding
--------------------

Instruction codes in Photon are encoded into 16-bit unsigned integer values. These contain all data of an instruction so the VM is able to decode and execute it on the fly.

Values (or scalars/constants) are encoded in the last 8-bits of the instruction, so the VM supports values in a range from *0-255*. The data gets placed from the last-significant-bit position:

	| 7| 6| 5| 4| 3| 2| 1| 0|

The parameters are stored using 4-bits for all:

	|11|10| 9| 8|

The first 4-bits are used to store the instruction type:

	|15|14|13|12|

As an example, an instruction that loads a constant value into a VM register would be represented as following:

	setreg   reg2   #30
	-------------------------
	0001     0010   00011110 => 0x121E


Note that registers can store values that are **greater than 255**, e.g. as the result of an addition.
Some instructions use three parameters to define their behaviour, this works by splitting the 8-bit constant section into two 4-bit sections. This works because registers **never** exceed the *0x0 - 0xF* range so they can be stored using only 4-bits.


Halt Instruction
----------------

The halt instruction is similar to C/C++ `return` or `exit`. It can indicate an error in the VM byte-code that can either be emitted by the VM itself, e.g. by an *out of bounds jump* or a *divide by zero*. The halt instruction does not need to take any parameter, in this case it **will always** return success to the host-application. The host-application can query the last exit code from the VM via the `getExitCode()` method.

Photon defines a fixed number of exit codes that can be emitted by the VM itself to report internal errors that are critical and should halt the VM immediately. The following table shows all internal exit codes:

<table>
	<tr>
		<td><b>Exit Code</b></td>
		<td><b>Code Name</b></td>
		<td><b>Description</b></td>
	</tr>
	<tr>
		<td><i>0</i></td>
		<td><b><i>ExitCodeSuccess</i></td>
		<td>Signals success.</td>
	</tr>
	<tr>
		<td><i>1</i></td>
		<td><b><i>ExitCodeDivideByZero</i></b></td>
		<td>Signals a division by zero error.</td>
	</tr>
	<tr>
		<td><i>2</i></td>
		<td><b><i>ExitCodeJumpOutOfBounds</i></b></td>
		<td>Signals that the offset of a jump-instruction is out of bounds.</td>
	</tr>
	<tr>
		<td><i>3</i></td>
		<td><b><i>ExitCodeRegisterFault</i></b></td>
		<td>Signals that the byte-code tried to access an invalid register.</td>
	</tr>
	<tr>
		<td><i>4</i></td>
		<td><b><i>ExitCodeInvalidHostCall</i></b></td>
		<td>Signals that a host call function was requested but could not be found. This will only be signalled if <b><i>PHOTON_IS_HOST_CALL_STRICT</i></b> is enabled.</td>
	</tr>
	<tr>
		<td><i>5</i></td>
		<td><b><i>ExitCodeUserCode</i></b></td>
		<td>First error code that should be used by any non VM code. User errors should start at <b><i>ExitCodeUserCode + userCodeNumber</i></b> as they will otherwise conflict with the above values.</td>
	</tr>
</table>

If the result of the VM execution is greater or equal to `ExitCodeUserCode` then the exit code is emitted by the user byte-code.

The exit code is returned from the `run()` method of the VM instance or alternatively by calling `getExitCode()` on the VM instance.

	Photon::VMExitCode code = virtualMachine.run();
	// OR
	Photon::VMExitCode code = virtualMachine.getExitCode();

	// Check the exit code for success.
	if(code != Photon::VMExitCodes::ExitCodeSuccess)
	{
		printf("The VM has exited with exit code: %d.\n", code);
	}
	else
	{	
		printf("VM successfully finished!");
	}