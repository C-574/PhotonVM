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
#ifndef _VIRTUAL_MACHINE_H_
#define _VIRTUAL_MACHINE_H_

// Include all needed prerequisites of the library.
#include "PhotonPrerequisites.h"
#include "ByteCode.h"
#include "HostCall.h"


namespace Photon
{


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


/**
 * \brief This class is the virtual machine that executes a list of byte code.
 */
class VirtualMachine
{
public:
	/** Define a common function signature that can be used to add a custom debugging callback. 
	 * Note that only one callback can be registered with an instance of the virtual machine at a time.
	 * This can be used to build instruction specific behaviour into the host application or even a to 
	 * create small 'debugger' arround the VM. The behaviour can be enabled or disabled using the
	 * PHOTON_DEBUG_CALLBACK_ENABLED flag in the Photon main header file. By default this is enabled. 
	 * Disabling the feature will remove the callback function from the VM code but keeps all interface
	 * functionality so the setUserCallbackFunction can still be called. */
	typedef void(*UserExecutionCallback)(const Photon::Instruction& instruction, const Photon::RegisterType* registers);


	/** Constructor of a virtual machine. The VM is halted by default to execute it call the run() method.
	 * \param	byteCode	Byte code to execute on the VM. Default is <b>nullptr</b>. */
	VirtualMachine(ByteCode* byteCode = nullptr) :
		m_IsHalted(true),
		m_ExitCode(VMExitCodes::ExitCodeSuccess),
		m_ByteCode(byteCode),
		m_CurrentPosition(0),
		m_VerbosityLevel(VerbosityLevelDefault)
	{
		// Clear all registers to zero.
		clearRegisters();

#if PHOTON_DEBUG_CALLBACK_ENABLED
		// Reset the user callback function pointer.
		m_UserCallback = nullptr;
#endif
	}

	/** Destructor of a virtual machine. */
	~VirtualMachine()
	{
		// Empty, but it is better to have a destructor.
	}

	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/  

	/** Set the current verbosity level of the VM's output. 
	 * \param	verbosity	New verbosity level of the VM. */
	void setVerbosityLevel(const VerbosityLevel& verbosity)
	{
		m_VerbosityLevel = verbosity;
	}


	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/  


	/** Run the virtual machine and execute the byte-code. 
	 * \return	Returns the exit code of the VM that was set when the VM halts. */
	VMExitCode run()
	{
		// Start the VM by unhalting it and resetting the exit code.
		m_IsHalted = false;
		m_ExitCode = VMExitCodes::ExitCodeSuccess;

		// Clear all registers to zero.
		clearRegisters();


		// The next byte-code instruction that will be executed by the VM.
		RawInstruction instructionCode;

		// The next instruction that will be executed. Unpacked from the raw instruction code.
		Instruction nextInstruction = {};


		/* Run as long as the machine has not halted.
		 * The machine will either halt because the execution has finished
		 * or a halt instruction is executed. */
		while(!m_IsHalted)
		{
			// Get the code of the next instruction to execute.
			getNextInstructionCode(instructionCode);

			// Unpack the instruction byte-code to an instruction.
			unpackInstruction(instructionCode, nextInstruction);

			// Execute the instruction on the VM registers.
			executeInstruction(nextInstruction);


#if PHOTON_DEBUG_CALLBACK_ENABLED
			// Execute the user callback funtion.
			if(m_UserCallback) m_UserCallback(nextInstruction, m_Registers);
#endif
		}

		return m_ExitCode;
	}


	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/  


	/** Set the byte code that will be executed by the VM.
	 * If the VM is currently running then it will get halted before setting the new byte-code.
	 * It will also reset the current execution position as the old byte code gets invalidated. */
	inline void setByteCode(ByteCode* byteCode)
	{
		// Request the VM to halt execution.
		// NOTE: Really needed? We do not call this from another thread (currently).
		halt(VMExitCodes::ExitCodeHaltRequested);
		m_ByteCode = byteCode;
		m_CurrentPosition = 0;
	}

	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/ 


	/** Set the user callback function of the virtual machine. This will be called after executing each byte code instruction.
	 * \param	callback	Callback function pointer to set. Note that the signature MUST macth the signature that is defined by \ref UserExecutionCallback. 
	 * \note	This will only set the function if the VM was compiled with PHOTON_DEBUG_CALLBACK_ENABLED set to enabled. */
	inline void setUserCallbackFunction(UserExecutionCallback callback)
	{
#if PHOTON_DEBUG_CALLBACK_ENABLED
		m_UserCallback = callback;
#endif
	}

	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/  

	
	 /** Checks if the VM is in a halted state or is currently running. */
	inline bool isHalted() const
	{
		return m_IsHalted;
	}

	/** Halts the VM on the next instruction that will be executed.
	 * \param	exitCode	Code that the byte-code emits when halting the VM indicating either success (0) or failure. */
	inline void halt(VMExitCode exitCode);

	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/ 

	/** Get the code that the VM has reported on the last halt-instruction. This is only valid if the VM has halted. 
	 * If the result is zero then the VM has finished execution successfully, otherwise it returns the error code that was passed to the last halt-instruction. 
	 * For mode information about VM exit codes see the VMExitModes enumeration. */
	VMExitCode getExitCode()
	{
		return m_ExitCode;
	}

	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/ 


	/** Registers a new Host-Call function to the VM or overwrites an existing one.
	 * \param	hostCallFunction	Instance of the Host-Call function to register. Note that the VM now takes over the ownership of the instance.
	 * \return	Returns <b>true</b> if the function was registered sucessfully or <b>false</b> if it failed because the function call id (group|function) are out of valid bounds or the specified object is invalid. */
	bool registerHostCallFunction(IHostCallFunction* hostCallFunction)
	{
		return m_HostCallFunctionContainer.registerHostCallFunction(hostCallFunction);
	}

	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/ 


	/** Pack a VM instruction to a raw instruction.
	 * \param	instruction		VM instruction to encode.
	 * \return	Returns a raw %instruction that contains packed byte-code data. */
	static RawInstruction packInstruction(const Instruction& instruction)
	{
		return (
		((instruction.instructionCode << 12) & 0xF000) |
		((instruction.param1 << 8) & 0x0F00) |
		((instruction.param2 << 4) & 0x00F0) |
		(instruction.param3 & 0x000F) |
		(instruction.value & 0x00FF));
	}

private:
	/** Unpack a raw instruction from a VM instruction. 
	 * \param	rawInstruction	Raw instruction to decode.
	 * \param	instruction		Instruction to decode the raw data to. */
	static void unpackInstruction(const RawInstruction& rawInstruction, Instruction& instruction)
	{
		instruction.instructionCode = static_cast<InstructionCode>((rawInstruction & 0xF000) >> 12);
		instruction.param1 = (rawInstruction & 0x0F00) >> 8;
		instruction.param2 = (rawInstruction & 0x00F0) >> 4;
		instruction.param3 = rawInstruction & 0x000F;
		instruction.value  = rawInstruction & 0x00FF;
	}

	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/  


	/** Executes an instruction on the registers of the virtual machine.
	 * \param	instruction		Instruction data to execute on the VM registers. */
	void executeInstruction(Instruction& instruction)
	{
		/* Switch on the instruction code.
		 * NOTE: For performance the compiler should (in release builds) replace the
		 * switch with jump tables. Any recent compiler should do this including MSVC,
		 * GCC and Clang. */
		switch(instruction.instructionCode)
		{
		case InstructionSetRegister:
		{
			setRegister(instruction);
			break;
		}
		case InstructionCopyRegister:
		{
			copyRegister(instruction);
			break;
		}
		case InstructionAddRegister:
		{
			addRegister(instruction);
			break;
		}
		case InstructionSubRegister:
		{
			subtractRegister(instruction);
			break;
		}
		case InstructionMulRegister:
		{
			multiplyRegister(instruction);
			break;
		}
		case InstructionDivRegister:
		{
			divideRegister(instruction);
			break;
		}
		case InstructionInvRegister:
		{
			inverteRegister(instruction);
			break;
		}
		case InstructionEqRegister:
		{
			equalRegister(instruction);
			break;
		}
		case InstructionNeqRegister:
		{
			notEqualRegister(instruction);
			break;
		}
		case InstructionGrtRegister:
		{
			greaterRegister(instruction);
			break;
		}
		case InstructionLetRegister:
		{
			lessRegister(instruction);
			break;
		}
		case InstructionJump:
		{
			jumpInstruction(instruction);
			break;
		}
		case InstructionCallHost:
		{
			hostCallInstruction(instruction);
			break;
		}
		// NOTE: (Deprecated) This instruction will get removed in some later release.
		case InstructionDumpRegister:
		{
			dumpRegisters(instruction);
			break;
		}
		case InstructionHalt:
		default:
		{
			halt(instruction.value);
			break;
		}
		} /* End of switch. */
	}

	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/  


	/** Get a register from the VM by the specified register index.
	 * \param	registerIndex	Index of the register to get.
	 * \return	Returns a reference to the value of the register at the register index.
	 * \warning If the specified index is invalid the VM will halt execution. */
	RegisterType& getRegister(const int32_t registerIndex)
	{
		// Check if the index is valid.
		if(registerIndex <= RegisterCount && registerIndex >= 0)
		{
			// Return a reference to the register at the specified index.
			return m_Registers[registerIndex];
		}
		else
		{
			// If it is invalid, halt the VM.
			printMessage(VerbosityLevelError, "VMFAULT: Tried to access invalid register at index %d!\n", registerIndex);
			halt(VMExitCodes::ExitCodeRegisterFault);
		}

		// Return 'local' as a default.
		return m_Registers[Local];
	}

	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/  


	/** Get the packed instruction byte-code of the next instruction that will be executed by the VM.
	 * \param	rawInstruction	Returns either the next byte code to excecute or a <i>halt</i> instruction of no byte-code is available. */
	void getNextInstructionCode(RawInstruction& rawInstruction)
	{
		// Initialize the instruction code with the halt-instruction by default.
		rawInstruction = 0x0000;

		// Only try to get an instruction if any byte-code is set.
		if(m_ByteCode && 
			(m_ByteCode->instructionCount > 0) &&
			(m_CurrentPosition <= m_ByteCode->instructionCount))
		{
			// Set the instruction code and increment the position.
			rawInstruction = m_ByteCode->instructions[m_CurrentPosition];
			m_CurrentPosition++;
		}
	}

	/*----------------------------------------------------------------------------------------------------------------
	 * VM Instructions
	 *--------------------------------------------------------------------------------------------------------------*/  

	/** Set a VM register to the specified value. 
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void setRegister(const Instruction& instruction);

	/** Copy the content of a VM register into the specified register. 
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void copyRegister(const Instruction& instruction);


	/** Add the values of two registers together.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void addRegister(const Instruction& instruction);

	/** Subtract the values of two registers from eachother.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void subtractRegister(const Instruction& instruction);

	/** Multiply the values of two registers.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void multiplyRegister(const Instruction& instruction);

	/** Divide the values of two registers.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void divideRegister(const Instruction& instruction); 


	/** Inverts the value of a register in place.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void inverteRegister(const Instruction& instruction);


	/** Check if the values of two registers are equal.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void equalRegister(const Instruction& instruction);

	/** Check if the values of two registers are not equal.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void notEqualRegister(const Instruction& instruction);

	/** Check if the value of the first register is greater than the value of the second register.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void greaterRegister(const Instruction& instruction);

	/** Check if the value of the first register is less than the value of the second register.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void lessRegister(const Instruction& instruction);


	/** Executes a call to the host-application to execute a function.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void jumpInstruction(const Instruction& instruction);

	/** Jumps forward or backward on the instruction queue by a specified number of instructions.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void hostCallInstruction(const Instruction& instruction);

	/** Dump all VM registers and the corresponding values to the standard output.
	 * \param	instruction		Instruction that determines the behaviour of the register operation. */
	inline void dumpRegisters(const Instruction& instruction);


private:
	
	/** Hidden copy constructor that prevents the VM from being copied.
	 * \param   other   Other virtual machine to copy from. */
	VirtualMachine(const VirtualMachine& other)
	{
	}

	/** Jumps a specified number of instructions forwards or backwards in the instruction queue relative to the current position or absolute to the start in the byte-code queue.
	 * \param	jumpOffset		Offset to the instruction to jump to. In relative mode -1 will jump one instruction back and a value of 0 will be ignored and no jump will be executed. 
	 * \param	isRelative		Flag to indicate if the jump is relative to the current position (<b>true</b>) or absolute to the first instruction (<b>false</b>). 
	 * \return	Returns <b>true</b> if the jump executed successfully and the next instruction is set or <b>false</b> if the new position is out of bunds. */
	bool jumpTo(int32_t jumpOffset, uint8_t isRelative)
	{
		// If we are in absolute mode and the jump offset is zero, we do not execute a jump.
		if(isRelative && jumpOffset == 0) 
			return true;


		/* Calculate the new position in the instruction queue. We subtract one instruction in relative mode so we are not 
		 * skipping the new set instruction as the instruction counter will get incremented on the next instruction fetch. */
		const uint32_t newPosition = (m_CurrentPosition * isRelative) + jumpOffset - isRelative;
		

		// Check if the specified offset is a valid index.
		if(m_ByteCode && (newPosition <= m_ByteCode->instructionCount))
		{
			// Jump to the address.
			m_CurrentPosition = newPosition;
			return true;
		}

		// The position is out of bounds.
		printMessage(VerbosityLevelError, "VMFAULT: Failed to jump to specified instruction! Instruction address is out of bounds. \n\tInstruction position: %d (%s + %d), begin = 0, end = %d.\n",
			newPosition, (isRelative ? "current" : "0"), jumpOffset, (m_ByteCode ? m_ByteCode->instructionCount : 0));

		return false;
	}

	/** Clears the values of all registers to zero. */
	inline void clearRegisters()
	{
		memset(&m_Registers, 0, sizeof(m_Registers));
	}


	/** Outputs messages that are emitted by the VM to the standard output.
	 * \param	verbosity	Verbosity level of the emitted message. If this is below the VM's level then the message will be ignored.
	 * \param	message		Message to display. */
	template <typename... Args>
	inline void printMessage(VerbosityLevel verbosity, const char* message, Args... args)
	{
		// Ignore the call if we are in silent mode.
		if(verbosity == VerbosityLevelSilent)
			return;

		if(verbosity & m_VerbosityLevel)
		{
			if(verbosity & VerbosityLevelError)
				fprintf(stderr, message, args...);
			else
				fprintf(stdout, message, args...);
		}
	}
	

public:
	/** Enumeration of all registers. */
	enum Register
	{
		Reg0 = 0,	///< Main register of the VM.

		Reg1 = 1,	///< Standard register 1.
		Reg2 = 2,	///< Standard register 2.
		Reg3 = 3,	///< Standard register 3.
		Reg4 = 4,	///< Standard register 4.
		Reg5 = 5,	///< Standard register 5.

		Tmp0 = 6,	///< Temporary register 0.
		Tmp1 = 7,	///< Temporary register 1.
		Tmp2 = 8,	///< Temporary register 2.
		Tmp3 = 9,	///< Temporary register 3.
		Tmp4 = 10,	///< Temporary register 4.
		Tmp5 = 11,	///< Temporary register 5.

		Local = 12, ///< Short time local register. Value may get overwritten in next instruction.
		//Return = 14,	///< Byte-code instruction index to where the last jump operation occured.

		RegisterCount ///< Total number of registers of the Virtual Machine.
	};


private:
	/** Flag to indicate if the virtual machine has halted or is running. */
	bool			m_IsHalted;	

	/** Exit code that gets set on a halt-instruction to indicate the success of the executed byte-code. 
	 * The default exit code is zero. */
	VMExitCode		m_ExitCode;

	/** Registers of the virtual machine. All instructions will operate on this registers. */
	RegisterType	m_Registers[RegisterCount];

	/** Pointer to the byte code that will be executed. */
	ByteCode*		m_ByteCode;

	/** Current position of the VM in the byte code array. */
	uint32_t		m_CurrentPosition;

	/** A container for all registered Host-Call functions. */
	HostCallFunctionContainer m_HostCallFunctionContainer;

	/** Current output verbosity level of the VM. */
	VerbosityLevel	m_VerbosityLevel;


#if PHOTON_DEBUG_CALLBACK_ENABLED
	/** Debug callback function of the VM. This can be set by the user via the setUserCallbackFunction() method. */
	UserExecutionCallback m_UserCallback;
#endif

};

} /* End of Photon namespace. */


// Include the implementation of the instructions.
#include "Instructions.inl"

#endif //_VIRTUAL_MACHINE_H_
