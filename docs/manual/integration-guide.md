# C++ Integration Guide #
-------------------------

This page describes how the Photon scripting language can be integrated into any C++ project. Only a *C++11* compatible compiler is required in order to compile with Photon. The process of integrating and customizing Photon with your own C++  project will be described in the following sections.

Setting Up Your Project
-----------------------

If you wan to add Photon support to a project, you first have to add the include directory `PhotonVM/include` to your projects include directories.

Photon does not require to link against any specific static or dynamic library at it is a *header-only* library that will get compiled with the code of your own project.

Now include the `PhotonVM.h` header file to get access to all parts of the virtual machine and its language. The following code-snippets shows how to create an instance of the VM that is able to execute compiled byte-code:

	Photon::VirtualMachine virtualMachine(byteCode);

alternatively the byte-code can be set after the VM has been created: 

	Photon::VirtualMachine virtualMachine;
	virtualMachine.setByteCode(byteCode);

Before we can start the machine up, we first have to register some *Host-Calls*. *Host-Calls* are Photon's way of communicating with the host application that the VM is running in. For more information about *Host-Calls* see the section on [Host-Calls](#host-calls "Host-Calls").  

It is now time to get the VM running. To do so, simply call the *run* method of the VM. This method executes the byte-code that was passed to the VM and returns a value that signals if any error has occurred during the execution of the code. This value can be used to e.g. show a message to the user that a script has failed to execute.

	// Execute the byte-code and check the return value.
	Photon::VMExitCode exitCode = virtualMachine.run();
	if(exitCode != Photon::VMExitCodes::ExitCodeSuccess)
	{
		printf("ERROR: Failed to execute byte-code! The VM has exited with exit code: %hu.\n", exitCode);
	}


Compiling Byte-Code
-------------------

To be able to execute anything on the VM compiled Byte-Code is required. This is a series of bytes that define instructions that the virtual machine can execute. This code can either be compiled directly from Photon source code or from a file that contains already pre-compiled code. This section shows both ways to get code into Photon.


**1) Compiling From Source Code**

Photon source code can be either compiled from a file on disk or directly from an input string that contains the source code. The code below shows how to use both methods. To convert the text data into instructions that are understandable to the VM *Lexer* is required. This Will parse the specified input and converts it into the corresponding VM instructions. For more information about instructions see the **Language** page of the documentation.  

	// Create a new lexer to parse the input code.
	Photon::Lexer lexer;

	// Parse the input using the 'parse' or 'parseFromFile' method of the lexer.
	const char* inputCode = "setreg reg0 #12";
	Photon::ByteCode* byteCode = lexer.parse(inputCode);
	// OR
	Photon::ByteCode* byteCode = lexer.parseFromFile(inputFile);


	// Report all errors of the lexer that occurred during compilation.
	while(lexer.hasError())
	{
		printf("%s\n", lexer.getErrorMessage().c_str());
	} 


**2) Loading From Byte-Code**

Byte-code is an already compiled form of VM instructions that can be loaded faster than regular source code and is very small compared to the original source. The following sample shows how to load pre-compiled byte-code from a file on disk.

	// Create the byte-code object that the data gets read into.
	Photon::ByteCode* byteCode = new Photon::ByteCode();

	// Load the actual byte-code from the input file.
	Photon::loadByteCodeFromFile(inputFile, *byteCode);

Similar to the `loadByteCodeFromFile` function a `writeByteCodeToFile` function exists which will write byte-code **to** a file on disk.


Customizing Photon
------------------

When integrating Photon into your own project you are able to customize it to the needs of your application. Some of this options can be found in the `PhotonVM.h` header file. The following table shows a list of all global options that can be set when compiling Photon:

<table>
	<tr>
		<td><b>Option</b></td>
		<td><b>Values</b></td>
		<td><b>Description</b></td>
	</tr>
	<tr>
		<td><i>PHOTON_IS_HOST_CALL_STRICT</i></td>
		<td><b><i>0-1</i></td>
		<td>Enable or disable strictness of host calls. If enabled and no host call can be found for an <i>hcall</i> instruction the VM will halt, otherwise it will continue.</td>
	</tr>
	<tr>
		<td><i>PHOTON_WARNINGS_ENABLED</i></td>
		<td><b><i>0-1</i></td>
		<td>Enable or disable compile and runtime warnings.</td>
	</tr>
	<tr>
		<td><i>PHOTON_DEBUG_CALLBACK_ENABLED</i></td>
		<td><b><i>0-1</i></td>
		<td>Enable or disable the user debug callback on the virtual machine. For more information see the section of this page about <i>Debug Callbacks</i>.</td>
	</tr>
	<tr>
		<td><i>MAX_NUM_HOST_CALLS</i></td>
		<td><b><i>1-4096</i></td>
		<td>Total number of host calls that can be registered at once. This can be reduced if fewer calls are used. The maximum number of calls is: 0xFFFFU = 4096U. Note that one group always consists of 256 functions.</td>
	</tr>
</table>


Host-Calls
----------

Another way of customizing Photon is to add application and script depended ***Host-Calls***. *Host-Calls* are functions that resist in the C++ code of the host application and can be accessed from scripts to communicate with the host of the VM. They can be used to add custom behaviour to the VM or simply provide more complex instructions that are not supported by the core VM. The example below shows how to create a custom function that can be called from script and squares the value of a register.

To expose a function to the VM byte-code from the host-application, a class needs to be declared that defines the C++ code that will be executed on a call from the byte-code.

	class HC_SquareValue : public Photon::IHostCallFunction
	{
	public:
		HC_SquareValue() : 
			IHostCallFunction(0 /*Group ID used by the VM*/, 1 /*Function ID that identifies the function*/, "SquareValue")
		{ }
	
		void execute(RegisterType* registers) override
		{
			// Get the input of the value to compute from the reg1 register.
			Photon::RegisterType value = registers[Photon::VirtualMachine::Reg1];
		
			// Compute the resulting output.
			value = value * value;
		
			// Store the result in the reg2 register.
			registers[Photon::VirtualMachine::Reg2] = value;
		}	
	}; 

If OOP is not required for a *Host-Call* then alternatively the `DECLARE_SIMPLE_HOST_CALL_BEGIN` and `DECLARE_SIMPLE_HOST_CALL_END` macros can be used to automatically wrap the implementation into a class.  

	DECLARE_SIMPLE_HOST_CALL_BEGIN(HC_SquareValue, 0 /*Group ID used by the VM*/, 1 /*Function ID that identifies the function*/, "SquareValue")
	{
		// Get the input of the value to compute from the reg1 register.
		Photon::RegisterType value = registers[Photon::VirtualMachine::Reg1];
	
		// Compute the resulting output.
		value = value * value;
	
		// Store the result in the reg2 register.
		registers[Photon::VirtualMachine::Reg2] = value;
	}
	DECLARE_SIMPLE_HOST_CALL_END()


After the *Host-Call* has been declared it only has to get registered with the VM that should call it. Note that the VM **does not** take over the ownership of the *Host-Call* instance which means that you are responsible to release the instance when it is no longer needed. Currently there is also no way to *unregister* a host call, this may get changed in the future.


	// Create an instance of the call and register it with the VM.
	static HC_SquareValue hcSquareValue;
	virtualMachine.registerHostCallFunction(&hcSquareValue);

Now a Photon script can call the function via the *hcall* instruction.

	setreg reg1 #10
	hcall $0 #1 


Debug Callbacks
---------------

Debug callbacks can be useful when debugging the VM. They report the decoded instruction that got executed by the VM and the current state of all registers. This information can be used to track bugs in Photon scripts. For this feature to work the **PHOTON_DEBUG_CALLBACK_ENABLED** option must be enabled. It is **important** that the signature of the custom callback function matches `void callbackName(const Photon::Instruction& instruction, const Photon::RegisterType* registers)`. Note that *neither* the instruction that was executed *nor* the registers themselves can be modified by the callback at any time. Also only *one* callback can be bound to *one* VM instance at a time. An example of a custom callback that prints the last executed instruction and current value of the `local` register to the standard output:

	void myCallback(const Photon::Instruction& instruction, const Photon::RegisterType* registers)
	{
		printf("Instruction executed: %d, local=%d\n", instruction.instructionCode, registers[Photon::VirtualMachine::Local]);
	}

Now the callback has to be registered with an instance of a virtual machine. Only one callback can be registered with one VM at a time. Debug callbacks always get called **after** an instruction has been executed by the virtual machine.

	// Register with a virtual machine.
	virtualMachine.setUserCallbackFunction(myCallback);