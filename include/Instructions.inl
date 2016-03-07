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

/*----------------------------------------------------------------------------------------------------------------
 * This inline file is included by VirtualMachine.h and implements the VM instructions.
 *--------------------------------------------------------------------------------------------------------------*/  

namespace Photon
{

/*----------------------------------------------------------------------------------------------------------------
 * SET/COPY INSTRUCTIONS
 *--------------------------------------------------------------------------------------------------------------*/ 


void VirtualMachine::setRegister(const Instruction& instruction)
{
	// Get the register to load the value to.
	RegisterType& reg = getRegister(instruction.param1);

	// Simply store the value into the specified register.
	reg = instruction.value;
	
	printMessage(VerbosityLevelDebugInfo, "setreg reg%d #%d\n", instruction.param1, instruction.value);
}

void VirtualMachine::copyRegister(const Instruction& instruction)
{
	// Get the register to copy the content to.
	RegisterType& reg = getRegister(instruction.param1);

	// Get the parameter register and copy its content.
	const RegisterType& regSource = getRegister(instruction.param2);
	reg = regSource;

	printMessage(VerbosityLevelDebugInfo, "cpyreg reg%d reg%d (#%d)\n", instruction.param1, instruction.param2, reg);
}

/*----------------------------------------------------------------------------------------------------------------
 * MATH INSTRUCTIONS
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::addRegister(const Instruction& instruction)
{
	// Get the two registers to operate on. Store the result in the first specified register.
	RegisterType& result		= getRegister(instruction.param1);
	const RegisterType& regA	= getRegister(instruction.param2);
	const RegisterType& regB	= getRegister(instruction.param3);
		

	// Do the actual operation.
	result = regA + regB;

	printMessage(VerbosityLevelDebugInfo, "addreg reg%d reg%d => reg%d=%d\n", instruction.param2, instruction.param3, instruction.param1, result);
}

/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::subtractRegister(const Instruction& instruction)
{
	// Get the two registers to operate on. Store the result in the first specified register.
	RegisterType& result		= getRegister(instruction.param1);
	const RegisterType& regA	= getRegister(instruction.param2);
	const RegisterType& regB	= getRegister(instruction.param3);

	// Do the actual operation.
	result = regA - regB;

	printMessage(VerbosityLevelDebugInfo, "subreg reg%d reg%d => reg%d=%d\n", instruction.param2, instruction.param3, instruction.param1, result);
}

/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::multiplyRegister(const Instruction& instruction)
{
	// Get the two registers to operate on. Store the result in the first specified register.
	RegisterType& result		= getRegister(instruction.param1);
	const RegisterType& regA	= getRegister(instruction.param2);
	const RegisterType& regB	= getRegister(instruction.param3);

	// Do the actual operation.
	result = regA * regB;

	printMessage(VerbosityLevelDebugInfo, "mulreg reg%d reg%d => reg%d=%d\n", instruction.param2, instruction.param3, instruction.param1, result);
}

/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::divideRegister(const Instruction& instruction)
{
	// Get the two registers to operate on. Store the result in the first specified register.
	RegisterType& result		= getRegister(instruction.param1);
	const RegisterType& regA	= getRegister(instruction.param2);
	const RegisterType& regB	= getRegister(instruction.param3);

	// Check if the divisor is valid.
	if(regB != 0)
	{
		// Do the actual operation.
		result = regA / regB;
	}
	else
	{
		// If we tried o divide by zero, throw an error message and halt the VM.
		fprintf(stderr, "VMFAULT: Invalid division by zero! Arguments: reg%d reg%d(%d) reg%d(%d)\n", instruction.param1, instruction.param2, regA, instruction.param3, regB);
		halt(VMExitCodes::ExitCodeDivideByZero);
	}

	printMessage(VerbosityLevelDebugInfo, "divreg reg%d reg%d => reg%d=%d\n", instruction.param2, instruction.param3, instruction.param1, result);
}

/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

void VirtualMachine::inverteRegister(const Instruction& instruction)
{
	// Get the register to operate on. Store the result in the same register.
	RegisterType& regA = getRegister(instruction.param1);

	// Do the actual operation.
	regA = -regA;

	printMessage(VerbosityLevelDebugInfo, "invreg reg%d => reg%d=%d\n", instruction.param1, instruction.param1, regA);
}

/*----------------------------------------------------------------------------------------------------------------
 * CONDITION CHECK INSTRUCTIONS
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::equalRegister(const Instruction& instruction)
{
	// Get the two registers to operate on. Store the result in the first specified register.
	RegisterType& result		= getRegister(instruction.param1);
	const RegisterType& regA	= getRegister(instruction.param2);
	const RegisterType& regB	= getRegister(instruction.param3);

	// Compare the register values and store the result.
	result = (regA == regB);

	printMessage(VerbosityLevelDebugInfo, "eqreg reg%d reg%d => reg%d=%d\n", instruction.param2, instruction.param3, instruction.param1, result);
}

/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::notEqualRegister(const Instruction& instruction)
{
	// Get the two registers to operate on. Store the result in the first specified register.
	RegisterType& result		= getRegister(instruction.param1);
	const RegisterType& regA	= getRegister(instruction.param2);
	const RegisterType& regB	= getRegister(instruction.param3);

	// Compare the register values and store the result.
	result = (regA != regB);

	printMessage(VerbosityLevelDebugInfo, "neqreg reg%d reg%d => reg%d=%d\n", instruction.param2, instruction.param3, instruction.param1, result);
}

/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::greaterRegister(const Instruction& instruction)
{
	// Get the two registers to operate on. Store the result in the first specified register.
	RegisterType& result		= getRegister(instruction.param1);
	const RegisterType& regA	= getRegister(instruction.param2);
	const RegisterType& regB	= getRegister(instruction.param3);

	// Compare the register values and store the result.
	if(regA > regB)
		result = 1;
	else
		result = 0;

	printMessage(VerbosityLevelDebugInfo, "geqreg reg%d reg%d => reg%d=%d\n", instruction.param2, instruction.param3, instruction.param1, result);
}

/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::lessRegister(const Instruction& instruction)
{
	// Get the two registers to operate on. Store the result in the first specified register.
	RegisterType& result		= getRegister(instruction.param1);
	const RegisterType& regA	= getRegister(instruction.param2);
	const RegisterType& regB	= getRegister(instruction.param3);

	// Compare the register values and store the result.
	if(regA < regB)
		result = 1;
	else
		result = 0;

	printMessage(VerbosityLevelDebugInfo, "leqreg reg%d reg%d => reg%d=%d\n", instruction.param2, instruction.param3, instruction.param1, result);
}

/*----------------------------------------------------------------------------------------------------------------
 * UTILLITY INSTRUCTIONS
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::halt(VMExitCode exitCode)
{
	// Only halt if the VM is running. Prevents multiple halt messages in one instruction.
	if(!m_IsHalted)
	{
		// Halt the VM and store the exit code.
		m_IsHalted = true;
		m_ExitCode = exitCode;

		printMessage(VerbosityLevelDebugInfo, "-- HALTING VIRTUAL MACHINE --\n");
	}
}

/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::jumpInstruction(const Instruction& instruction)
{
	// Get the parameter register and get its value.
	const RegisterType& regSource		= getRegister(instruction.param1);
	const int32_t instructionJumpOffset	= regSource;
	const uint8_t isAbsolute			= instruction.value;

	// Jump to the instruction. If it failed then halt the VM.
	if(!jumpTo(instructionJumpOffset, !isAbsolute))
		halt(VMExitCodes::ExitCodeJumpOutOfBounds);

	printMessage(VerbosityLevelDebugInfo, "jump => %s + reg%d(%d)\n", (isAbsolute ? "0" : "current"), instruction.param1, regSource);
}

/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::hostCallInstruction(const Instruction& instruction)
{
	// Get the group-id and the function-id from the instruction.
	const uint8_t groupId		= instruction.param1;
	const uint8_t functionId	= instruction.value;

#if PHOTON_WARNINGS_ENABLED
	if(instruction.param3 == 0 && instruction.param3 != 0)
		printMessage(VerbosityLevelWarning, "WARNING: One of the specified parameters is not a constant value! This may cause undefined behaviour.\n");
#endif // PHOTON_WARNINGS_ENABLED

	// Get the function to call from the container and if it exists, call it.
	Photon::IHostCallFunction* function = m_HostCallFunctionContainer.getHostCallFunction(groupId, functionId);
	if(function)
	{
		/* Convert to a reference to reference to avoid vtable lookup, as seen in Doom3 source code.
		 * See: http://fabiensanglard.net/doom3/index.php */
		Photon::IHostCallFunction& functionRef = *function;

		printMessage(VerbosityLevelDebugInfo, "Executing host call \"%s\" with gid=%d and fid=%d\n", functionRef.getName(), groupId, functionId);

		// Execute the C++ code.
		functionRef.execute(m_Registers);
	}
	else
	{
		printMessage(VerbosityLevelError, "Failed to execute host call! No callable function found with gid=%d and fid=%d.\n", groupId, functionId);

#if PHOTON_IS_HOST_CALL_STRICT
		// We are strict and do not allow the execution to continue as the call could be important.
		halt(VMExitCodes::ExitCodeInvalidHostCall);
#endif // PHOTON_IS_HOST_CALL_STRICT
	}
}

/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


void VirtualMachine::dumpRegisters(const Instruction& instruction)
{
	printMessage(VerbosityLevelDebugInfo, "Register Dump:\n");

	for(uint16_t i = 0, end = ((RegisterCount - 1)/ 2); i < end; i++)
		printMessage(VerbosityLevelAll, "\treg%d = %d\t| tmp%d = %d\n", i , getRegister(i), i , getRegister((i + Tmp0)));
	printMessage(VerbosityLevelAll, "\tlocal = %d\n", getRegister(Local));
}

} /* End of Photon namespace. */
