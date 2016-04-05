/*********************************************************************************
 * Copyright (c) 2015-2016 by Niklas Grabowski (C-574)                           *
 *                                                                               *
 * Permission is hereby granted, free of charge, to any person obtaining a copy  *
 * of this software and associated documentation files (the "Software"), to deal *
 * in the Software without restriction, including without limitation the rights  *
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell     *
 * copies of the Software, and to permit persons to whom the Software is         *
 * furnished to do so, subject to the following conditions:                      *
 *                                                                               *
 * The above copyright notice and this permission notice shall be included in    *
 * all copies or substantial portions of the Software.                           *
 *                                                                               *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR    *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,      *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE   *
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER        *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN     *
 * THE SOFTWARE.                                                                 *
 *********************************************************************************/
#include <stdio.h>
#include "CmdArgumentParser.h"
#include "PhotonVM.h"
#include "StandardHostCalls.h"


/*----------------------------------------------------------------------------------------------------------------
 * FORWARD DECLARATIONS
 *--------------------------------------------------------------------------------------------------------------*/  

void showHelpMessage();
void showLogo();
Photon::VerbosityLevel convertVerbositylevel(int level);
void compileToByteCode(const char* inputFile, const char* outputFile);
Photon::ByteCode* compileFromSource(const char* inputFile);
Photon::ByteCode* loadFromByteCode(const char* inputFile);
void executeByteCode(Photon::ByteCode* byteCode, const Photon::VerbosityLevel& verbosity);
void registerHostCallFunctions(Photon::VirtualMachine& virtualMachine);


/*----------------------------------------------------------------------------------------------------------------
 * MAIN FUNCTION
 *--------------------------------------------------------------------------------------------------------------*/  

int main(int argc, char** argv)
{
	Photon::ByteCode* byteCode = nullptr;
	Photon::VerbosityLevel verbosity = Photon::VerbosityLevelDefault;

	showLogo();

	// Create the command line argument handler.
	CmdArgumentParser cmdArguments(argc, argv);

	// Check for the help command; -h.
	if(cmdArguments.argumentExists("h") || argc <= 1)
	{
		showHelpMessage();
		return 0;
	}

	// Get the output file of the compiler.
	const char* outputFileName = cmdArguments.getString("o", "./byteCode.pbc");

	// Check if a source file needs to be compiled.
	if(cmdArguments.argumentExists("c"))
	{
		compileToByteCode(cmdArguments.getString("c"), outputFileName);
	}

	// Check if a source file should be executed.
	if(cmdArguments.argumentExists("r"))
	{
		byteCode = compileFromSource(cmdArguments.getString("r"));
	}
	// Load byte-code from a file.
	else if(cmdArguments.argumentExists("b"))
	{
		byteCode = loadFromByteCode(cmdArguments.getString("b"));
	}

	// Get the verbosity level.
	if(cmdArguments.argumentExists("v"))
	{
		verbosity = convertVerbositylevel(cmdArguments.getInteger("v"));
	}

	// Execute the byte-code if it was loaded.
	if(byteCode)
	{
		executeByteCode(byteCode, verbosity);
		delete byteCode;
	}

#if 0
	getchar();
#endif

	return 0;
}


/*----------------------------------------------------------------------------------------------------------------
 * COMMANDS
 *--------------------------------------------------------------------------------------------------------------*/  

void showLogo()
{
	printf("-------------------------------------------------------------------------\n");
	printf("  PVM - PhotonVM V.%d.%d.%d | Copyright (c) Niklas Grabowski.\n", PHOTON_VM_VERSION_MAJOR, PHOTON_VM_VERSION_MINOR, PHOTON_VM_VERSION_SUB_MINOR);
	printf("-------------------------------------------------------------------------\n");
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

Photon::VerbosityLevel convertVerbositylevel(int level)
{
	Photon::VerbosityLevel verbosityLevel = Photon::VerbosityLevelDefault;

	// Convert the enumeration.
	switch(level)
	{
	case 1:
		verbosityLevel = Photon::VerbosityLevelSilent;
		break;
	case 2:
		verbosityLevel = Photon::VerbosityLevelAll;
		break;
	case 3:
		verbosityLevel = Photon::VerbosityLevelError;
		break;
	case 0:
	default:
		break;
	}

	return verbosityLevel;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

void showHelpMessage()
{
	static const char* message = "Usage: pvm [OPTIONS]\n"
		"\t-h        Shows this help message.\n"
		"\t-c=[file] Compiles an input file to byte-code.\n"
		"\t-o=[file] Sets the output file of the compiler. Default is ./byteCode.pbc.\n"
		"\t-b=[file] Executes compiled Photon byte-code. Can NOT be used in combination with -r.\n"
		"\t-r=[file] Executes Photon source code. Can NOT be used in combination with -b.\n"
		"\t-v=[num]  Sets the verbosity level of the VM. 0=Default (Errors & Warnings), 1=Silent, 2=All, 3=Error only.\n";
	puts(message);
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

void compileToByteCode(const char* inputFile, const char* outputFile)
{
	Photon::ByteCodeReadWriteResult result;
	Photon::ByteCode* byteCode;
	
	// Compile the byte code from source.
	byteCode = compileFromSource(inputFile);
	if(!byteCode)
	{
		printf("=> Failed to compile to byte-code.\n");
		return;
	}

	printf("=> Generating output file \"%s\"...\n", outputFile);

	// Write the byte-code block to the output file.
	result = Photon::writeByteCodeToFile(outputFile, *byteCode);
	if(result != Photon::ByteCodeReadWriteResultSuccess)
	{
		// Handle error codes.
		if(result == Photon::ByteCodeReadWriteResultFileOpenFailed)
			printf("=> ERROR: Failed to generate output file! Failed to open file: \"%s\"!\n", outputFile);
		else /* result == Photon::ByteCodeReadWriteResultIncorrectData */
			printf("=> ERROR: Failed to generate output file! Incorrect number of bytes written!\n");
	}

	printf("==> Generated byte-code consists of %d instructions, size is %d bytes.\n", byteCode->byteCodeInstructionCount, 
		static_cast<uint32_t>(sizeof(Photon::RawInstruction) * byteCode->byteCodeInstructionCount));

	// Make sure to release the byte code.
	if(byteCode)
		delete byteCode;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

Photon::ByteCode* compileFromSource(const char* inputFile)
{
	printf("=> Compiling \"%s\"...\n", inputFile);

	// Create a lexer to generate byte-code instructions from the input file.
	Photon::Lexer lexer;

	// NOTE: Make sure to release the byte-code via delete.
	Photon::ByteCode* byteCode = lexer.parseFromFile(inputFile);

	// List all errors.
	while(lexer.hasError())
	{
		printf("=> %s\n", lexer.getErrorMessage().c_str());
	}

	return byteCode;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/ 

Photon::ByteCode* loadFromByteCode(const char* inputFile)
{
	Photon::ByteCodeReadWriteResult result;

	printf("=> Loading byte-code from file \"%s\"...\n", inputFile);

	// Create the byte-code object.
	Photon::ByteCode* byteCode = new Photon::ByteCode();

	// Load the byte-code from the input file.
	result = Photon::loadByteCodeFromFile(inputFile, *byteCode);
	if(result != Photon::ByteCodeReadWriteResultSuccess)
	{
		// Handle error codes.
		if(result == Photon::ByteCodeReadWriteResultFileOpenFailed)
			printf("=> ERROR: Failed to open file: \"%s\"!\n", inputFile);
		else if(result == Photon::ByteCodeReadWriteResultInvalidFileType)
			printf("=> ERROR: Incorrect byte-code format or file does not contain Photon byte-code!\n");
		else if(result == Photon::ByteCodeReadWriteResultIncompatibleByteCode)
			printf("=> ERROR: Byte-code was compiled with a newer version and is not compatible!\n");
#if PHOTON_WARNINGS_ENABLED
		else if(result == Photon::ByteCodeReadWriteResultDeprecatedWarning)
			printf("=> WARNING: Byte-code was compiled with and older version of the instruction set and may be deprecated!\n");
#endif
		else if(result == Photon::ByteCodeReadWriteResultIncorrectData)
			printf("=> ERROR: Failed to read byte-code data!\n");
	}

	// Return the created byte-code object.
	return byteCode;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/ 

#if PHOTON_DEBUG_CALLBACK_ENABLED
/* Example on how to use a callback to get the instruction that got executed and all registers. 
 * This is only used if PHOTON_DEBUG_CALLBACK_ENABLED is enabled. */
void debugCallback(const Photon::Instruction& instruction, const Photon::RegisterType* registers)
{
	printf("Instruction executed: %d, local=%d\n", instruction.instructionCode, registers[Photon::VirtualMachine::Local]);
}
#endif


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/ 

void executeByteCode(Photon::ByteCode* byteCode, const Photon::VerbosityLevel& verbosity)
{
	// Create the VM to execute the byte-code.
	Photon::VirtualMachine virtualMachine(byteCode);
	
#if PHOTON_DEBUG_CALLBACK_ENABLED
	// Set the user callback function.
	virtualMachine.setUserCallbackFunction(debugCallback);
#endif

	// Setup the verbosity level.
	virtualMachine.setVerbosityLevel(verbosity);

	// Register all Host-Call functions.
	registerHostCallFunctions(virtualMachine);

	// Execute the byte-code.
	Photon::VMExitCode exitCode = virtualMachine.run();
	if(exitCode != Photon::VMExitCodes::ExitCodeSuccess)
	{
		printf("=> ERROR: Failed to execute byte-code! The VM has exited with exit code: %hu.\n", exitCode);
	}
}


/*----------------------------------------------------------------------------------------------------------------
 * UTILLITY
 *--------------------------------------------------------------------------------------------------------------*/ 

void registerHostCallFunctions(Photon::VirtualMachine& virtualMachine)
{
	// Register all standard Host-Call functions.
	PhotonStandardHostCalls::registerStandardHostCalls(virtualMachine);

	// Add custom Host-Calls...
}
