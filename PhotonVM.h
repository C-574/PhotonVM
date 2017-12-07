/*********************************************************************************
 * Copyright (c) 2015-2017 by Niklas Grabowski (C-574)                           *
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
#ifndef _PHOTON_VM_H_
#define _PHOTON_VM_H_

// @Cleanup: Unify register types. Some are int32_t and some are uint32_t. We schould make more use of the RegisterType where applicable.
//    - C-574 (15.11.2017)

#include <cstdint>
#include <cstring>
#include <cstdio>
#ifndef PHOTON_NO_COMPILER
    #include <cstdarg> // For error reporting; va_list...
#endif // PHOTON_NO_COMPILER


/*----------------------------------------------------------------------------------------------------------------
 * Build Settings
 *--------------------------------------------------------------------------------------------------------------*/  

/* To make Photon use your own memory allocation and deallocation functions define PVM_MALLOC_OVERRIDE
 * and define pho_malloc and pho_free with your implementation. */
#ifndef PHOTON_MALLOC_OVERRIDE
    #include <stdlib.h>
    #define pho_malloc(size) (malloc(size))
    #define pho_free(ptr) (free(ptr))
#endif // PHOTON_MALLOC_OVERRIDE

/* Define PHOTON_STATIC to make the implementation private to the source file that generates it. */
#ifdef PHOTON_STATIC
    #define PHO_DECL static
#else
    #define PHO_DECL extern
#endif // PHOTON_STATIC

/* Total number of Host-Calls that can be registered at once. This can be reduced if fewer calls are used.
 * The maximum number of calls is: 0xFFF = 4095. Note that one group always consists of 256 functions. */
#ifndef PHOTON_MAX_HOST_CALLS
    #define PHOTON_MAX_HOST_CALLS 32
#endif // PHOTON_MAX_HOST_CALLS

#ifndef PHOTON_DEBUG_CALLBACK_ENABLED
    #define PHOTON_DEBUG_CALLBACK_ENABLED 0 // Enable or disable the user debug callback on the virtual machine.
#endif // PHOTON_DEBUG_CALLBACK_ENABLED

#ifndef PHOTON_IS_HOST_CALL_STRICT
    #define PHOTON_IS_HOST_CALL_STRICT 0 // Enable or disable strictness of Host-Calls. If enabled and no Host-Call can be found for a hcall instruction the VM will halt, otherwise it will continue.
#endif // PHOTON_IS_HOST_CALL_STRICT

#ifndef PHOTON_COMPILER_ERROR_STRICT
    #define PHOTON_COMPILER_ERROR_STRICT 0 // If set to 1 then the lexer will stop after it encounters an error, otherwise it will continue.
#endif // PHOTON_COMPILER_ERROR_STRICT


/*----------------------------------------------------------------------------------------------------------------
 * Version Information
 *--------------------------------------------------------------------------------------------------------------*/  

/** Version number of the PhotonVM in X.YYY.ZZ (Major, Minor, Sub-Minor) format. Change only this value if the version changes. */
const int32_t PHOTON_VM_VERSION				= 200000;
/** Major version extracted from PHOTON_VM_VERSION. */
const int32_t PHOTON_VM_VERSION_MAJOR		= (PHOTON_VM_VERSION / 100000);
/** Minor version extracted from PHOTON_VM_VERSION. */
const int32_t PHOTON_VM_VERSION_MINOR		= ((PHOTON_VM_VERSION / 100) % 1000);
/** Sub-Minor version extracted from PHOTON_VM_VERSION. */
const int32_t PHOTON_VM_VERSION_SUB_MINOR	= (PHOTON_VM_VERSION % 100);


/*----------------------------------------------------------------------------------------------------------------
 * INTERFACE
 *--------------------------------------------------------------------------------------------------------------*/  

namespace Photon
{

/** Define a raw/encoded instruction as a 16-bit signed integer. */
typedef int16_t RawInstruction;
/** Define the type of a register. */
typedef int32_t RegisterType;
/** Reference to a register. */
typedef RegisterType* RegisterRef;
/** Define an exit code that can be emitted by the VM or user code. */
typedef uint8_t VMExitCode;

struct MappedInstruction;
/** Define a common function signature that can be used to add a custom debugging callback. 
 * Note that only one callback can be registered with an instance of the virtual machine at a time.
 * This can be used to build instruction specific behaviour into the host application or even a to 
 * create small 'debugger' arround the VM. The behaviour can be enabled or disabled using the
 * PHOTON_DEBUG_CALLBACK_ENABLED flag. By default this is enabled. Disabling the feature will remove
 * the callback function from the VM code but keeps all interface functionality so the 
 * setUserCallbackFunction can still be called. */
#define DebugCallback(name) void name(const Photon::MappedInstruction* instruction, const Photon::RegisterType* registers)
typedef DebugCallback(fDebugCallback);

/** Signature of an application host call. All host calls are rquired to have the function signature as seen below. 
 * Register a host call function with a virtual machine using the registerHostCall function. */
#define HostCallback(name) void name(Photon::RegisterType* registers)
typedef HostCallback(fHostCallback);


/** Enumerations of all verbosity levels of the VM. */
enum VerbosityLevel
{
	/** Disables all messages. */
	VerbosityLevelSilent = 0,
	/** Display warnings. */
	VerbosityLevelWarning = 1,
	/** Display error messages. */
	VerbosityLevelError = 2,
	/** Display debugging information. */
	VerbosityLevelDebugInfo	= 4,

	/** Display all warnings errors and debug info. */
	VerbosityLevelAll = VerbosityLevelError | VerbosityLevelWarning | VerbosityLevelDebugInfo,
	/** Default verbosity level. This only shows warnings and errors. */
	VerbosityLevelDefault = VerbosityLevelError | VerbosityLevelWarning
};

/** Exit codes that can be emitted by the VM itself. 
 * User errors range from <i>1</i> to <i>250</i> as they will otherwise conflict with the values below which get emitted by the VM. */
enum VMExitCodes
{
    /** Signals success. */
    ExitCodeSuccess = 0,
    /** Signals that the VM should be halted by a user request. 
     * This does not mean that the VM has finished execution of the byte-code. */
    ExitCodeHaltRequested = 0xFB,
    /** Signals a division by zero error. */
    ExitCodeDivideByZero = 0xFC,
    /** Signals that the offset of a jump-instruction is out of bounds. */
    ExitCodeJumpOutOfBounds = 0xFD,
    /** Signals that the byte-code tried to access an invalid register. */
    ExitCodeRegisterFault = 0xFE,
    /** Signals that a Host-Call function was requested but could not be found.
     * \note This will only be signalled if PHOTON_IS_HOST_CALL_STRICT is enabled. */
    ExitCodeInvalidHostCall = 0xFF,
    /** First code that can be emitted by user code. */
    ExitCodeUserCode = 1
};

/** Enumeration of all VM instructions. */
enum OpCode
{
    /** Halt the execution of the virtual machine. */
    OpCodeHalt = 0x00,
    /** Writes a value to a specific VM register. */
    OpCodeSet = 0x01,
    /** Copies the content of one register into another. */
    OpCodeCopy = 0x02,
    /** Add the content of two registers together. */
    OpCodeAdd = 0x03,
    /** Subtract the contents of two registers. */
    OpCodeSub = 0x04,
    /** Multiply the contents of two registers. */
    OpCodeMul = 0x05,
    /** Divide the contents of two registers. */
    OpCodeDiv = 0x06,
    /** Invert the value of a register in place. */
    OpCodeInv = 0x07,
    /** Compares two register values and stores <b>1</b> if both are equal, otherwise <b>0</b>. */
    OpCodeEql = 0x08,
    /** Compares two register values and stores <b>1</b> if both are <b>not</b> equal, otherwise <b>0</b>. */
    OpCodeNeq = 0x09,
    /** Compares two register values and stores <b>1</b> if the first value is greater than the second, otherwise <b>0</b>. */
    OpCodeGrt = 0x0A,
    /** Compares two register values and stores <b>1</b> if the second value is less than the second, otherwise <b>0</b>. */
    OpCodeLet = 0x0B,
    /** Jumps to a specified instruction in the instruction queue. If the specified instruction index is invalid then no jump is made. */
    OpCodeJump = 0x0C,
    /** Execute a function in the host application space. */
	OpCodeCallHost = 0x0D,
};

/** Enumeration of all registers. */
enum Register
{
    Reg0,
    Reg1,
    Reg2,
    Reg3,
    Reg4,
    Reg5,
    Reg6,
    Reg7,
    Reg8,
    Reg9,
    Reg10,
    Reg11,

    Reg12, ///< Also addressable as "local". Short time local register. Value may get overwritten in next instruction.
    Local = Reg12, ///< Alias of the Reg12 register. This may get removed!
    //Return = 14,	///< Byte-code instruction index to where the last jump operation occured.

    RegisterCount ///< Total number of registers of the Virtual Machine.
};


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

struct MappedInstruction
{
	/** VM instruction code. This defines which parameters to use. */
	OpCode opCode;
	struct
	{
		uint32_t destReg;

		// set, jmp, hcl, halt
		int32_t value; // 8 bits used.
			
		// everything else
		int32_t argRegA; // 4 bits used
		int32_t argRegB; // 4 bits used
	} params;
};


inline RawInstruction packInstruction(MappedInstruction* inst)
{
	// argA is stored in the high four bits and argB in the low four bits.
	uint32_t value = inst->params.value | (inst->params.argRegB | (inst->params.argRegA << 4));
    
    RawInstruction result = 0;
    result |= (inst->opCode << 12) & 0xF000;
    result |= ((inst->params.destReg << 8) & 0x0F00);
    result |= ((value) & 0x00FF);
    return result;
}

inline void unpackInstruction(RawInstruction raw, MappedInstruction* inst)
{
	uint32_t opCode  = (raw & 0xF000) >> 12;
	uint32_t destReg = (raw & 0x0F00) >> 8;
	uint32_t value   = (raw & 0x00FF);

	uint32_t argRegA = (value >> 4) & 0x0F;
	uint32_t argRegB = value & 0x0F;

	inst->opCode = static_cast<OpCode>(opCode);
	inst->params.destReg = destReg;
	inst->params.value   = value;
	inst->params.argRegA = argRegA;
	inst->params.argRegB = argRegB;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

/** This structure contains byte-code data that can be executed by the VM. 
 * Note that this is only a container class, you need to free the byte-code array after using it. */
struct ByteCode
{
    /** Pointer to the byte-code array to execute. */
    RawInstruction* instructions;
    /** Total number of byte-code instructions that are stored in the byte code array. */
    uint32_t instructionCount;
};


/** Check if the specified byte-code is valid and could be executed by the VM.
 * Note that this function only checks if any byte-code is set, not if the set byte-code instructions are valid. 
 * \param	byteCode	Byte-code to check for.
 * \return	Returns <b>true</b> if the byte-code is valid and contains any instructions or otherwise <b>false</b>. */
inline bool isByteCodeValid(ByteCode* byteCode)
{
    return (byteCode && byteCode->instructions && byteCode->instructionCount);
}

/** Release the byte-code data. This will deallocate the instruction data and reset the instruction count to zero. 
 * After this method has been called the specified byte-code is invalidated (isByteCodeValid will return <b>false</b>) and the VM will no longer be able to execute it.
 * \param	byteCode	Byte-code to release. */
PHO_DECL void releaseByteCode(ByteCode* byteCode);


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

/** Helper structure that contains all registered script host call functions.
 * The host callbacks are stored as simple function pointers and are indexed by
 * a packed value*/
struct HostCallContainer
{
    fHostCallback* callbacks[PHOTON_MAX_HOST_CALLS]; // 0xFFFU Max count of functions: 0xF groups, 0xFF functions.

    uint16_t usedCallCount;
    uint16_t firstFreeEntryIndex;
};

/** Register a host callback with the specified virtual machine.
 * \param   vm          Virtual machine to which the callback should be registered.
 * \param   callback    Host application callback function to register. 
 * \param   groupId     Id of the group that the callback will be assigned to. Range is [0, 15].
 * \param   functionId  Id of the function slot that the callback will be assigned to inside of the group. Range is [0, 255]. 
 * \return  Returns 0 on success. -1 if the packed id is out of range and 1 if an already registered callback will be overwritten. */
PHO_DECL int32_t registerHostCall(struct VirtualMachine* vm, fHostCallback* callback, uint8_t groupId, uint8_t functionId);


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

struct VirtualMachine
{
    /** Flag to indicate if the virtual machine has halted or is running. */
    bool isHalted;	
    /** Exit code that gets set on a halt-instruction to indicate the success of the executed byte-code. 
     * The default exit code is zero. */
    VMExitCode exitCode;

    /** Registers of the virtual machine. All instructions will operate on this registers. */
    RegisterType registers[RegisterCount];
    /** Pointer to the byte code that will be executed. */
    ByteCode byteCode;
    /** Current position of the VM in the byte code array. */
    uint32_t currentPosition;
    /** A container for all registered Host-Call functions. */
    HostCallContainer hostCallContainer;
    /** Current output verbosity level of the VM. */
    VerbosityLevel verbosityLevel;

#if PHOTON_DEBUG_CALLBACK_ENABLED
    /** Debug callback function of the VM. This can be set by the user via the setDebugCallback() method. */
    fDebugCallback* debugCallback;
#endif
};

/** Create a new virtual machine. The VM is halted by default. To execute it call the run method.
 * \param	byteCode	Byte code to execute on the VM. 
 * \param   verbosity   Output verbosoty of the vm. Default is VerbosityLevelDefault. */
PHO_DECL VirtualMachine createVirtualMachine(ByteCode byteCode, VerbosityLevel verbosity = VerbosityLevelDefault);
/** Run the virtual machine and execute the byte-code. 
 * \param   vm  Virtual machine to execute.
 * \return	Returns the exit code which was set when the VM halts. */
PHO_DECL VMExitCode run(VirtualMachine* vm);
/** Set the debug callback function of the specified VM. */
PHO_DECL void setDebugCallback(VirtualMachine* vm, fDebugCallback* callback);

#ifndef PHOTON_NO_COMPILER
/** Compile Photon byte-code from the specified string of source code.
 * \param   source      Null-terminated string that contains the source data.
 * \param   fileName    Path to the file that gets compiled. Only for debug output. Default is <b>nullptr</b>. */
PHO_DECL ByteCode compile(char* source, const char* fileName = nullptr);
#endif // PHOTON_NO_COMPILER


/*----------------------------------------------------------------------------------------------------------------
 * IMPLEMENTATION
 *--------------------------------------------------------------------------------------------------------------*/  

#ifdef PHOTON_IMPLEMENTATION

static void instructionHalt(VirtualMachine* vm, VMExitCode exitCode);

PHO_DECL void releaseByteCode(ByteCode* byteCode)
{
    if(byteCode && isByteCodeValid(byteCode))
    {
        pho_free(byteCode->instructions);
        byteCode->instructions = nullptr;
        byteCode->instructions = 0U;
    }
}

PHO_DECL int32_t registerHostCall(struct VirtualMachine* vm, fHostCallback* callback, uint8_t groupId, uint8_t functionId)
{
    int32_t result = 0;
    HostCallContainer* container = &vm->hostCallContainer;

    if(callback)
    {
        uint32_t id = (groupId << 8) | functionId;
        if(id < PHOTON_MAX_HOST_CALLS)
        {
            if(container->callbacks[id] != nullptr)
            {
                result = 1; // Callback overwrite.
            }

            container->callbacks[id] = callback;
            container->usedCallCount++;
            if(id < container->firstFreeEntryIndex)
            {
                container->firstFreeEntryIndex = id;
            }
        }
        else
        {
            result = -1; // Out of range.
        }
    }

    return result;
}

// @Cleanup: Use a single, overridable error reporting function like reportErrorInternal.
//    - C-574 (28.09.2017)
/** Outputs messages that are emitted by the VM to the standard output.
 * \param	verbosity	Verbosity level of the emitted message. If this is below the VM's level then the message will be ignored.
 * \param	message		Message to display. */
template <typename... Args>
inline void printMessage(VirtualMachine* vm, VerbosityLevel verbosity, const char* message, Args... args)
{
    if(verbosity == VerbosityLevelSilent)
        return;

    if(verbosity & vm->verbosityLevel)
    {
        if(verbosity & VerbosityLevelError)
            fprintf(stderr, message, args...);
        else
            fprintf(stdout, message, args...);
    }
}

/** Get a register from the VM at the specified register index.
 * \param	registerIndex	Index of the register to get.
 * \return	Returns a pointer to register at the register index. The Local register will be returned if the index is invalid.
 * \warning If the specified index is invalid the VM will halt execution. */
static RegisterRef getRegister(VirtualMachine* vm, int32_t registerIndex)
{
    if(registerIndex < RegisterCount && registerIndex >= 0)
    {
        return (&vm->registers[registerIndex]);
    }
    else
    {
        printMessage(vm, VerbosityLevelError, "VMFAULT: Tried to access invalid register at index %d!\n", registerIndex);
        instructionHalt(vm, ExitCodeRegisterFault);
    }

    return (&vm->registers[Local]);
}

/** Jumps a specified number of instructions forwards or backwards in the instruction queue relative to the current position or absolute to the start in the byte-code queue.
 * \param	jumpOffset		Offset to the instruction to jump to. In relative mode -1 will jump one instruction back and a value of 0 will be ignored and no jump will be executed. 
 * \param	isRelative		Flag to indicate if the jump is relative to the current position (<b>true</b>) or absolute to the first instruction (<b>false</b>). 
 * \return	Returns <b>true</b> if the jump executed successfully and the next instruction is set or <b>false</b> if the new position is out of bunds. */
static bool jumpTo(VirtualMachine* vm, int32_t jumpOffset, uint8_t isRelative)
{
    if(isRelative && jumpOffset == 0) 
        return true;

    /* Calculate the new position in the instruction queue. We subtract one instruction in relative mode so we are not 
    * skipping the new set instruction as the instruction counter will get incremented on the next instruction fetch. */
    const uint32_t newPosition = (vm->currentPosition * isRelative) + jumpOffset - isRelative;
    
    if(isByteCodeValid(&vm->byteCode) && (newPosition < vm->byteCode.instructionCount))
    {
        vm->currentPosition = newPosition;
        return true;
    }

    printMessage(vm, VerbosityLevelError, "VMFAULT: Failed to jump to specified instruction! Instruction address is out of bounds. \n\tInstruction position: %d (%s + %d), begin = 0, end = %d.\n",
        newPosition, (isRelative ? "current" : "0"), jumpOffset, (isByteCodeValid(&vm->byteCode) ? (vm->byteCode.instructionCount - 1) : 0));

    return false;
}

/*----------------------------------------------------------------------------------------------------------------
 * Instructions
 *--------------------------------------------------------------------------------------------------------------*/  

#define PHOTON_INSTRUCTION(name) static void name(VirtualMachine* vm, MappedInstruction* instruction)
#define storeRegister(reg, value) *(reg) = (value)
#define loadRegister(vm, registerIndex) (*getRegister(vm, registerIndex))


static void instructionHalt(VirtualMachine* vm, VMExitCode exitCode)
{
    if(!vm->isHalted)
    {
        vm->isHalted = true;
        vm->exitCode = exitCode;

        printMessage(vm, VerbosityLevelDebugInfo, "-- HALTING VIRTUAL MACHINE --\n");
    }
}

PHOTON_INSTRUCTION(instructionSet)
{
    RegisterRef reg = getRegister(vm, instruction->params.destReg);
    storeRegister(reg, instruction->params.value);

    printMessage(vm, VerbosityLevelDebugInfo, "set reg%d #%d\n", instruction->params.destReg, instruction->params.value);
}

PHOTON_INSTRUCTION(instructionCopy)
{
    RegisterRef result = getRegister(vm, instruction->params.destReg);
	RegisterType regSource = loadRegister(vm, instruction->params.argRegA);
	storeRegister(result, regSource);

	printMessage(vm, VerbosityLevelDebugInfo, "cpy reg%d reg%d (#%d)\n", instruction->params.destReg, instruction->params.argRegA, *result);
}

PHOTON_INSTRUCTION(instructionAdd)
{
    RegisterRef result = getRegister(vm, instruction->params.destReg);
	RegisterType regA = loadRegister(vm, instruction->params.argRegA);
    RegisterType regB = loadRegister(vm, instruction->params.argRegB);
    storeRegister(result, regA + regB);

	printMessage(vm, VerbosityLevelDebugInfo, "add reg%d reg%d => reg%d=%d\n", instruction->params.argRegA, instruction->params.argRegB, instruction->params.destReg, *result);
}

PHOTON_INSTRUCTION(instructionSubtract)
{
    RegisterRef result = getRegister(vm, instruction->params.destReg);
	RegisterType regA = loadRegister(vm, instruction->params.argRegA);
	RegisterType regB = loadRegister(vm, instruction->params.argRegB);
	storeRegister(result, regA - regB);

	printMessage(vm, VerbosityLevelDebugInfo, "sub reg%d reg%d => reg%d=%d\n", instruction->params.argRegA, instruction->params.argRegB, instruction->params.destReg, *result);
}

PHOTON_INSTRUCTION(instructionMultiply)
{
    RegisterRef result = getRegister(vm, instruction->params.destReg);
	RegisterType regA = loadRegister(vm, instruction->params.argRegA);
	RegisterType regB = loadRegister(vm, instruction->params.argRegB);
	storeRegister(result, regA * regB);

	printMessage(vm, VerbosityLevelDebugInfo, "mul reg%d reg%d => reg%d=%d\n", instruction->params.argRegA, instruction->params.argRegB, instruction->params.destReg, *result);
}

PHOTON_INSTRUCTION(instructionDivide)
{
    RegisterRef result = getRegister(vm, instruction->params.destReg);
	RegisterType regA = loadRegister(vm, instruction->params.argRegA);
	RegisterType regB = loadRegister(vm, instruction->params.argRegB);
    if(regB != 0)
	{
		storeRegister(result, regA / regB);
	}
	else
	{
		fprintf(stderr, "VMFAULT: Invalid division by zero! Arguments: reg%d reg%d(%d) reg%d(%d)\n", instruction->params.destReg, instruction->params.argRegA, regA, instruction->params.argRegB, regB);
		instructionHalt(vm, ExitCodeDivideByZero);
	}

	printMessage(vm, VerbosityLevelDebugInfo, "div reg%d reg%d => reg%d=%d\n", instruction->params.argRegA, instruction->params.argRegB, instruction->params.destReg, *result);
}

PHOTON_INSTRUCTION(instructionInvert)
{
    RegisterRef result = getRegister(vm, instruction->params.destReg);
	storeRegister(result, -(*result));

	printMessage(vm, VerbosityLevelDebugInfo, "inv reg%d => reg%d=%d\n", instruction->params.destReg, instruction->params.destReg, *result);
}

PHOTON_INSTRUCTION(instructionEquals)
{
    RegisterRef result = getRegister(vm, instruction->params.destReg);
	RegisterType regA = loadRegister(vm, instruction->params.argRegA);
	RegisterType regB = loadRegister(vm, instruction->params.argRegB);
	storeRegister(result, regA == regB);

	printMessage(vm, VerbosityLevelDebugInfo, "eql reg%d reg%d => reg%d=%d\n", instruction->params.argRegA, instruction->params.argRegB, instruction->params.destReg, *result);
}

PHOTON_INSTRUCTION(instructionNotEquals)
{
    RegisterRef result = getRegister(vm, instruction->params.destReg);
	RegisterType regA = loadRegister(vm, instruction->params.argRegA);
	RegisterType regB = loadRegister(vm, instruction->params.argRegB);
	storeRegister(result, regA != regB);

	printMessage(vm, VerbosityLevelDebugInfo, "neq reg%d reg%d => reg%d=%d\n", instruction->params.argRegA, instruction->params.argRegB, instruction->params.destReg, *result);
}

PHOTON_INSTRUCTION(instructionGreater)
{
    RegisterRef result = getRegister(vm, instruction->params.destReg);
	RegisterType regA = loadRegister(vm, instruction->params.argRegA);
	RegisterType regB = loadRegister(vm, instruction->params.argRegB);
    storeRegister(result, regA > regB);

	printMessage(vm, VerbosityLevelDebugInfo, "gre reg%d(%d) reg%d(%d) => reg%d=%d\n", instruction->params.argRegA, regA, instruction->params.argRegB, regB, instruction->params.destReg, *result);
}

PHOTON_INSTRUCTION(instructionLess)
{
    RegisterRef result = getRegister(vm, instruction->params.destReg);
	RegisterType regA = loadRegister(vm, instruction->params.argRegA);
	RegisterType regB = loadRegister(vm, instruction->params.argRegB);
    storeRegister(result, regA < regB);
    
	printMessage(vm, VerbosityLevelDebugInfo, "les reg%d(%d) reg%d(%d) => reg%d=%d\n", instruction->params.argRegA, regA, instruction->params.argRegB, regB, instruction->params.destReg, *result);
}

PHOTON_INSTRUCTION(instructionJump)
{
    RegisterRef regSource = getRegister(vm, instruction->params.destReg);
	int32_t instructionJumpOffset = *regSource;
	uint8_t isAbsolute = instruction->params.value != 0;

	if(!jumpTo(vm, instructionJumpOffset, !isAbsolute))
		instructionHalt(vm, ExitCodeJumpOutOfBounds);

	printMessage(vm, VerbosityLevelDebugInfo, "jmp => %s + reg%d(%d)\n", (isAbsolute ? "0" : "current"), instruction->params.destReg, *regSource);
}


PHOTON_INSTRUCTION(instructionHostCall)
{
    fHostCallback* callback = nullptr;
    uint32_t groupId = instruction->params.destReg;
    uint32_t functionId = instruction->params.value;

    uint32_t id = (groupId << 8) | functionId;
    if(id < PHOTON_MAX_HOST_CALLS)
    {
        callback = vm->hostCallContainer.callbacks[id];
        if(callback)
        {
            callback(vm->registers);
            printMessage(vm, VerbosityLevelDebugInfo, "hcl %d %d\n", groupId, functionId);
        }
        else
        {
            printMessage(vm, VerbosityLevelError, "No callable function found with gid=%d and fid=%d, callback is null.\n", groupId, functionId);
        }
    }
    else
    {
		printMessage(vm, VerbosityLevelError, "Host callback IDs are out of range. Got: gid=%d and fid=%d\n", groupId, functionId);
    }

#if PHOTON_IS_HOST_CALL_STRICT
    if(!callback)
    {
        // We are strict and do not allow the execution to continue as the call could be important.
        instructionHalt(vm, VMExitCodes::ExitCodeInvalidHostCall);
    }
#endif // PHOTON_IS_HOST_CALL_STRICT
}

#undef PHOTON_INSTRUCTION
#undef storeRegister
#undef loadRegister


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

PHO_DECL VirtualMachine createVirtualMachine(ByteCode byteCode, VerbosityLevel verbosity)
{
    VirtualMachine vm = {};
    vm.isHalted = true;
    vm.byteCode = byteCode;
    vm.verbosityLevel = verbosity;

    return vm;
}

PHO_DECL VMExitCode run(VirtualMachine* vm)
{
    if(!vm) return ExitCodeHaltRequested;

    vm->isHalted = false;
    vm->exitCode = ExitCodeSuccess;
    memset(&vm->registers, 0, sizeof(vm->registers));

    RawInstruction rawInstruction;
    MappedInstruction instruction;

    while(!vm->isHalted)
    {
        rawInstruction = 0;
        if(isByteCodeValid(&vm->byteCode) &&
           vm->currentPosition < vm->byteCode.instructionCount)
        {
            rawInstruction = vm->byteCode.instructions[vm->currentPosition];
            vm->currentPosition++;
        }

        unpackInstruction(rawInstruction, &instruction);
        if(instruction.opCode == OpCodeHalt)
        {
            instructionHalt(vm, instruction.params.value);
        }
        else
        {
            switch(instruction.opCode)
            {
                case OpCodeSet:
                {
                    instructionSet(vm, &instruction);
                } break;
                case OpCodeCopy:
                {
                    instructionCopy(vm, &instruction);
                } break;
                case OpCodeAdd:
                {
                    instructionAdd(vm, &instruction);
                } break;
                case OpCodeSub:
                {
                    instructionSubtract(vm, &instruction);
                } break;
                case OpCodeMul:
                {
                    instructionMultiply(vm, &instruction);
                } break;
                case OpCodeDiv:
                {
                    instructionDivide(vm, &instruction);
                } break;
                case OpCodeInv:
                {
                    instructionInvert(vm, &instruction);
                } break;
                case OpCodeEql:
                {
                    instructionEquals(vm, &instruction);
                } break;
                case OpCodeNeq:
                {
                    instructionNotEquals(vm, &instruction);
                } break;
                case OpCodeGrt:
                {
                    instructionGreater(vm, &instruction);
                } break;
                case OpCodeLet:
                {
                    instructionLess(vm, &instruction);
                } break;
                case OpCodeJump:
                {
                    instructionJump(vm, &instruction);   
                } break;
                case OpCodeCallHost:
                {
                    instructionHostCall(vm, &instruction);
                } break;
                case OpCodeHalt:
                default:
                {
                    instructionHalt(vm, instruction.params.value);
                } break;
            }
        }


#if PHOTON_DEBUG_CALLBACK_ENABLED
        if(vm->debugCallback) vm->debugCallback(&instruction, vm->registers);
#endif // PHOTON_DEBUG_CALLBACK_ENABLED
    }

    return (vm->exitCode);
}

PHO_DECL void setDebugCallback(VirtualMachine* vm, fDebugCallback* callback)
{
#if PHOTON_DEBUG_CALLBACK_ENABLED
    vm->debugCallback = callback;
#endif // PHOTON_DEBUG_CALLBACK_ENABLED
}


/*----------------------------------------------------------------------------------------------------------------
 * Compiler Implementation
 *--------------------------------------------------------------------------------------------------------------*/  

#ifndef PHOTON_NO_COMPILER

/** Checks if the specified character is an end of line. 
 * This will handle both \n and \r end-lines. */
inline bool isEndOfLine(uint8_t c)
{
    return ((c == '\n') ||
            (c == '\r'));
}

/** Checks if the specified character is whitespace (space, tab or end of line). */
inline bool isWhitespace(uint8_t c)
{
    return ((c == ' ') ||
            (c == '\t') ||
            (isEndOfLine(c)));
}

/** Check if the specified character is an alphabetic value.*/
inline bool isAlpha(uint8_t c)
{
    return (((c >= 'a') && (c <= 'z')) ||
            ((c >= 'A') && (c <= 'Z')));
}

/** Check if the specified character is a numeric value. */
inline bool isNumber(uint8_t c)
{
    return ((c >= '0') && (c <= '9'));
}

/** Convert a string to a signed 32-bit integer value.
 * This supports decimal numbers as well as negative and positive numbers. 
 * If the number could not be parsed then this function will return the numeric value up to the 
 * point of the parser error (e.g. "24c6" => 24). */
inline int32_t stringToSint(const char* str)
{
    int32_t sign = 1;
    int32_t value = 0;
    uint8_t c;

    if(*str == '-')
    {
        sign = -1;
        ++str;
    }
    
    for(;;)
    {
        c = *str;
        ++str;
        if((c < '0') || (c > '9'))
            return (sign * value);
        value = value * 10 + c - '0';
    }
}


/*----------------------------------------------------------------------------------------------------------------
 * Data Structures
 *--------------------------------------------------------------------------------------------------------------*/  

/** Token types that can be generated by the lexer. */
enum Token
{
    /** Unknown or invalid token. */
    TokenUnknown,
    /** The token indicates that the end of the file to parse has been reached. */
    TokenEOF,
    
    /** The token is an identifier. This can be an instruction or a function name. */ 
    TokenIdentifier,
    /** The token is a numeric value. */
    TokenNumber,
    /** The token is a register of the VM. */
    TokenRegister,
};


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

/** Internal string structure that indexes into a larger source string. */
struct StringRef
{
    /** Pointer to the start of the string. */
    char* text;
    /** Size of the string in characters. */
    size_t length;
};


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

/** Structure that contains data for a single instruction. */
struct InstructionNode
{
    /** Next instruction node in the list. Null if no successor. */
    InstructionNode* next;
    /** MappedInstruction data mapped into memory. */
    MappedInstruction inst;
};


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

/** Internal structure that contains data that is used to generate tokens from a source string. */
struct Lexer
{
    /** Current position of the lexer in the string to parse. */
    char* at;
    /** The last string that was found with the last Identifier token. */
    StringRef identifierString;
    /** The current token. Use getNextToken to get the next token in the input stream. */
    Token token;

    /** Head of the instruction node list. */
    InstructionNode* nodeHead;
    /** Tail of the instruction node list. */
    InstructionNode* nodeTail;

    /** Current line that the parser is currently at. For error reporting only. */
    uint32_t lineNumber;
    /** Path to the file that is getting parsed. For error reporting only. */
    const char* fileName;
};


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

/** Reports a compiler error to the user. */
static void reportErrorInternal(Lexer* lexer, const char* format, ...)
{
    va_list list;

    // @Extendable: Notify lexer or do some sort of error tracking. 
    //    - C-574 (18.08.2017)

    va_start(list, format);
    vprintf(format, list);
    va_end(list);
}

#define reportError(lexer, format, ...) \
{ reportErrorInternal(lexer, "%s(%d): [ERROR] " format "\n", (lexer->fileName ? lexer->fileName : "<unknown>"), lexer->lineNumber, ##__VA_ARGS__); }


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

/** Convert a token type to a human readable string. */
inline const char* tokenToString(Token token)
{
    switch(token)
    {
    case TokenEOF:
        return "end of file";
    case TokenIdentifier:
        return "identifier";
    case TokenNumber:
        return "number";
    case TokenRegister:
        return "register";
    case TokenUnknown:
    default: {} break;
    }
    return "unknown";
}


/*----------------------------------------------------------------------------------------------------------------
 * Lexical Functions 
 *--------------------------------------------------------------------------------------------------------------*/  

/** Check if the current identifier equals a required token string.
 * \param   lexer       Lexer which contains the current identifier string to compare to. 
 * \param   tokenString Token string that is required. 
 * \return Returns <b>true</b> if the required token is found or otherwise <b>false</b>. */
inline bool isTokenStringEqual(Lexer* lexer, const char* tokenString)
{
    return (strncmp(tokenString, lexer->identifierString.text, lexer->identifierString.length) == 0);
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

/** Check if the specified identifier string is a register name. 
 * This will automatically remove the register prefix (rX and regX) from the identifier leaving only the index.
 * Note that this will <b>not</b> check whether the register index is out of bounds. 
 * \param	identifier	Identifier to check.
 * \return	Returns <b>true</b> if the identifier is a register name, <b>false</b> otherwise. */
static bool isIdentifierRegister(StringRef* identifier)
{
    // regX
    if(identifier->length >= 4 && 
        identifier->text[0] == 'r' &&
        identifier->text[1] == 'e' &&
        identifier->text[2] == 'g' &&
        isNumber(identifier->text[3]))
    {
        identifier->text += 3;
        identifier->length -= 3;
        return true;
    }
    // rX
    if(identifier->length >= 1 && 
        identifier->length <= 3 && 
        identifier->text[0] == 'r' && 
        isNumber(identifier->text[1]))
    {
        identifier->text += 1;
        identifier->length--;
        return true;
    }

    return false;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

/** Moves the position of the lexer's cursor forward until all whitespace is skipped.
 * This will automatically ignore spaces, tabs, end-of-lines and comments. 
 * \param   lexer   Lexer to parse the input. */
static void eatAllWhitespace(Lexer* lexer)
{
    while(isWhitespace(lexer->at[0]))
    {
        if(isEndOfLine(lexer->at[0]))
        {
            if(isEndOfLine(lexer->at[1])) // Make sure to handle \r\n as one new line.
                ++lexer->at;
            ++lexer->lineNumber;
        }
        ++lexer->at;
    }

    if(lexer->at[0] == '#')
    {
        do ++lexer->at;
        while(lexer->at[0] != '\0' && !isEndOfLine(lexer->at[0]));

        eatAllWhitespace(lexer);
    }
    else if(lexer->at[0] == '\0')
    {
        lexer->token = TokenEOF;
    }
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

/** Get the next token from the input data and stores it in the lexer's token field. 
 * \param   lexer   Lexer that is used to convert the input data into tokens.  */
static void getNextToken(Lexer* lexer)
{
    Token token = TokenUnknown;
    eatAllWhitespace(lexer);

    if(lexer->token == TokenEOF)
        return;

    // Identifier: [a-zA-Z][a-zA-Z0-9]*
    if(isAlpha(lexer->at[0]))
    {
        lexer->identifierString.text = lexer->at;
        while(isAlpha(lexer->at[0]) || isNumber(lexer->at[0]))
        {
            ++lexer->at;
        } 

        token = TokenIdentifier;
        lexer->identifierString.length = lexer->at - lexer->identifierString.text; 

        if(isIdentifierRegister(&lexer->identifierString))
        {
            token = TokenRegister;
        }
#if 0
        // Handle specific token strings.
        if(isTokenStringEqual(lexer, "function"))  token = TokenFunctionBegin;
        else if(isTokenStringEqual(lexer, "end"))  token = TokenFunctionEnd;
        else if(isTokenStringEqual(lexer, "call")) token = TokenFunctionCall;
#endif
    }
    // Number: [0-9.]+
    else if(isNumber(lexer->at[0]))
    {
        StringRef valueString = {};
        valueString.text = lexer->at;
        while(isNumber(lexer->at[0]))
        {
            ++lexer->at;
        } 

        valueString.length = lexer->at - valueString.text;
        lexer->identifierString = valueString;

        token = TokenNumber;
    }
    else
    {
        reportError(lexer, "Unknown character '%c'! Only alphabetic or numeric characters are allowed.", lexer->at[0]);
        ++lexer->at;
    }

    lexer->token = token;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

static int32_t getNumber(Lexer* lexer)
{
    int32_t result = 0;

    getNextToken(lexer);
    if(lexer->token == TokenNumber)
    {
        result = stringToSint(lexer->identifierString.text);
        if(result > UINT8_MAX || result < 0)
        {
            reportError(lexer, "Numeric value is out of range! Got: '%d', minimum is 0 and maximum is 255", result);
            result = 0;
        }
    }
    else
    {
        reportError(lexer, "Expected numeric value! Got: '%.*s' (%s)", (int)lexer->identifierString.length, lexer->identifierString.text, tokenToString(lexer->token));
    }

    return result;
}

static Register getRegister(Lexer* lexer)
{
    Register result = Reg0;

    getNextToken(lexer);
    if(lexer->token == TokenRegister)
    {
        result = static_cast<Register>(stringToSint(lexer->identifierString.text));
        if(result >= RegisterCount)
        {
            reportError(lexer, "Register index out of bounds! Got: '%d', maximum is %d", result, (RegisterCount - 1));
            result = static_cast<Register>(RegisterCount - 1);
        }
    }
    else
    {
        reportError(lexer, "Expected a register name! Got: '%.*s' (%s)", (int)lexer->identifierString.length, lexer->identifierString.text, tokenToString(lexer->token));
    }

    return result;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

static OpCode getOpCode(Lexer* lexer)
{
    OpCode opCode = OpCodeHalt;

    if(isTokenStringEqual(lexer, "set"))
        opCode = OpCodeSet;
    else if(isTokenStringEqual(lexer, "cpy"))
        opCode = OpCodeCopy;
    else if(isTokenStringEqual(lexer, "add"))
        opCode = OpCodeAdd;
    else if(isTokenStringEqual(lexer, "sub"))
        opCode = OpCodeSub;
    else if(isTokenStringEqual(lexer, "mul"))
        opCode = OpCodeMul;
    else if(isTokenStringEqual(lexer, "div"))
        opCode = OpCodeDiv;
    else if(isTokenStringEqual(lexer, "inv"))
        opCode = OpCodeInv;
    else if(isTokenStringEqual(lexer, "eql"))
        opCode = OpCodeEql;
    else if(isTokenStringEqual(lexer, "neq"))
        opCode = OpCodeNeq;
    else if(isTokenStringEqual(lexer, "gre"))
        opCode = OpCodeGrt;
    else if(isTokenStringEqual(lexer, "les"))
        opCode = OpCodeLet;
    else if(isTokenStringEqual(lexer, "jmp"))
        opCode = OpCodeJump;
    else if(isTokenStringEqual(lexer, "hcl"))
        opCode = OpCodeCallHost;
    else if(isTokenStringEqual(lexer, "halt"))
        opCode = OpCodeHalt;
    else
        reportError(lexer, "Unknown instruction '%.*s'!", (int)lexer->identifierString.length, lexer->identifierString.text);

    return opCode;
}

static void handleInstruction(Lexer* lexer, MappedInstruction* inst)
{
    // @Bug: Every unknown instruction will be interpreted as "halt". This will eat the next token as it will be seen as the
    // halt argument. We should add an OpCodeInvald(0) and handle it here and in getOpCode accordingly (e.g. generate "halt 251").
    //    - C-574 (28.09.2017)  
    OpCode opCode = getOpCode(lexer);
    switch(opCode)
    {
    case OpCodeHalt:
    {
        inst->params.value = getNumber(lexer);
    } break;
    case OpCodeSet:
    {
        inst->params.destReg = getRegister(lexer);
        inst->params.value   = getNumber(lexer);
    } break;
    case OpCodeCopy: 
    {
        inst->params.destReg = getRegister(lexer);
        inst->params.argRegA = getRegister(lexer);
    } break;
    case OpCodeAdd:
    case OpCodeSub:
    case OpCodeMul:
    case OpCodeDiv:
    case OpCodeEql:
    case OpCodeNeq:
    case OpCodeGrt:
    case OpCodeLet:
    {
        inst->params.destReg = getRegister(lexer);
        inst->params.argRegA = getRegister(lexer);
        inst->params.argRegB = getRegister(lexer);

    } break;
    case OpCodeInv:	
    {
        inst->params.destReg = getRegister(lexer);
    } break;
    
    case OpCodeJump:
    {
        inst->params.destReg = getRegister(lexer);
        inst->params.value   = getNumber(lexer);
    } break;
    case OpCodeCallHost:
    {
        int32_t groupId = getNumber(lexer);
        if(groupId > 0xF)
        {
            reportError(lexer, "Host call group id is out of bounds! Got: '%d', maximum is 15", groupId);
            groupId = 0xF;
        }
        inst->params.destReg = groupId;
        inst->params.value   = getNumber(lexer);
    } break;
    default:
    {
    } break;
    }

    inst->opCode = opCode;
}

static void handleIdentifier(Lexer* lexer)
{
    InstructionNode* node = static_cast<InstructionNode*>(pho_malloc(sizeof(InstructionNode)));
    if(node)
    {
        node->inst = {};
        node->next = nullptr;
        handleInstruction(lexer, &node->inst);

        if(lexer->nodeTail == nullptr)
            lexer->nodeTail = lexer->nodeHead = node;

        lexer->nodeHead->next = node;
        lexer->nodeHead = node;
    }
    else
    {
        fprintf(stderr, "INTERNAL COMPILER ERROR: Failed to allocate instruction node!\n");
    }
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  

PHO_DECL ByteCode compile(char* source, const char* fileName)
{
    ByteCode byteCode = {};
    Lexer lexer = {};
    lexer.at = source;
    lexer.lineNumber = 1;
    lexer.fileName = fileName;

    getNextToken(&lexer);
    bool isParsing = true;
    while(isParsing)
    {
#if PHOTON_COMPILER_ERROR_STRICT
        if(lexer.token == TokenEOF || lexer.token == TokenUnknown)
#else
        if(lexer.token == TokenEOF)
#endif
        {
            isParsing = false;
        }
        else
        {
            if(lexer.token == TokenIdentifier)
            {
                handleIdentifier(&lexer);
            } 
            else
            {
                // If we get here we have propably a syntax error. Unexpected number at new line, etc.
                Lexer* lexerPtr = &lexer;
                reportError(lexerPtr, "Unexpected token on line %d: '%.*s' (%s)", lexer.lineNumber, (int)lexer.identifierString.length, lexer.identifierString.text, tokenToString(lexer.token));
            }

            getNextToken(&lexer);
        }
    }

    uint32_t instructionCount = 0;
    InstructionNode* currentNode = lexer.nodeTail;
    while(currentNode)
    {
        // @Incomplete: Optimize node list, e.g. removal of chained halt instructions.
        //    - C-574 (18.08.2017)
        ++instructionCount;
        currentNode = currentNode->next;
    }

    RawInstruction* instructions = static_cast<RawInstruction*>(pho_malloc(sizeof(RawInstruction) * instructionCount));

    uint32_t instructionIndex = 0;
    currentNode = lexer.nodeTail;
    while(currentNode)
    {	
        instructions[instructionIndex] = packInstruction(&currentNode->inst);
        ++instructionIndex;

        InstructionNode* nodeToDelete = currentNode;
        currentNode = currentNode->next;
        pho_free(nodeToDelete);
    }
    lexer.nodeHead = lexer.nodeTail = nullptr;

    byteCode.instructionCount = instructionCount;
    byteCode.instructions     = instructions;
    return byteCode;
}

#endif // PHOTON_NO_COMPILER

#endif // PHOTON_IMPLEMENTATION


} /* End of Photon namespace. */

#endif // _PHOTON_VM_H_