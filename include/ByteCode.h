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
#ifndef _BYTE_CODE_H_
#define _BYTE_CODE_H_

// Include all needed prerequisites of the library.
#include "PhotonPrerequisites.h"


namespace Photon
{


/*----------------------------------------------------------------------------------------------------------------
 * INSTRUCTIONS
 *--------------------------------------------------------------------------------------------------------------*/  

/** Enumeration of all VM instructions. */
enum InstructionCode
{
	InstructionHalt			= 0x00,		///< Halt the execution of the virtual machine.
	InstructionSetRegister	= 0x01,		///< Writes a value to a specific VM register.
	InstructionCopyRegister = 0x02,		///< Copies the content of one register into another.
	InstructionAddRegister	= 0x03,		///< Add the content of two registers together.
	InstructionSubRegister	= 0x04,		///< Subtract the contents of two registers.
	InstructionMulRegister	= 0x05,		///< Multiply the contents of two registers.
	InstructionDivRegister	= 0x06,		///< Divide the contents of two registers.
	InstructionInvRegister	= 0x07,		///< Invert the value of a register in place.
	InstructionEqRegister	= 0x08,		///< Compares two register values and stores <b>1</b> if both are equal, otherwise <b>0</b>.
	InstructionNeqRegister	= 0x09,		///< Compares two register values and stores <b>1</b> if both are <b>not</b> equal, otherwise <b>0</b>.
	InstructionGrtRegister	= 0x0A,		///< Compares two register values and stores <b>1</b> if the first value is greater than the second, otherwise <b>0</b>.
	InstructionLetRegister	= 0x0B,		///< Compares two register values and stores <b>1</b> if the second value is less than the second, otherwise <b>0</b>.
	InstructionJump			= 0x0C,		///< Jumps to a specified instruction in the instruction queue. If the specified instruction index is invalid then no jump is made.
	InstructionCallHost		= 0x0D,		///< Execute a function in the host application space.
	InstructionDumpRegister	= 0x0E		///< Dump all register values to the standard output.
};


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


/** An instruction that can be executed by the virtual machine's registers. */
struct Instruction
{
	/** Default constructor of an instruction. */
	Instruction() : 
		instructionCode(InstructionHalt),
		param1(0), param2(0), param3(0), value(0)
	{
	}

	/** Clear the instruction data to its defaults. */
	void clear()
	{
		instructionCode = InstructionHalt;
		param1 = 0;
		param2 = 0;
		param3 = 0;
		value  = 0;
	}


	/** Instruction code of the instruction. See VMInstructions for more info.
	 * \todo Change to 8-/16-bit value? */
	int32_t instructionCode;
	/** The first parameter of the instruction. */
	int32_t param1;
	/** The second parameter of the instruction. */
	int32_t param2;
	/** The third parameter of the instruction. */
	int32_t param3;

	/** Value of the instruction, if any. */
	int32_t value;
};


/*----------------------------------------------------------------------------------------------------------------
 * BYTE CODE
 *--------------------------------------------------------------------------------------------------------------*/  

/** This structure contains byte code data that can be executed by the VM. 
 * Note that this is only a container class, you need to free the byte code array after using it. */
struct ByteCode
{
	/** Default constructor. */
	ByteCode() :
		byteCode(nullptr),
		byteCodeInstructionCount(0)
	{
	}

	/** Constructor. */
	ByteCode(RawInstruction* byteCode, uint32_t instructionCount) :
		byteCode(byteCode),
		byteCodeInstructionCount(instructionCount)
	{
	}

	/** Destructor of the byte code structure.
	 * This will release the byte code array that is stored in the structure. */
	~ByteCode()
	{
		if(byteCode)
			delete[] byteCode;
	}

	/** Pointer to the byte code array to execute. */
	RawInstruction* byteCode;

	/** Total number of byte code instructions that are stored in the byte code array. */
	uint32_t byteCodeInstructionCount;
};


/*----------------------------------------------------------------------------------------------------------------
 * Byte-Code Loader/Writer
 *--------------------------------------------------------------------------------------------------------------*/

#define BYTE_CODE_MAGIC_NUMBER { 'P', 'B', 'C', ' ' }

#pragma pack(push, 1)
/** A structire that contains the header of a Photon byte-code file. */
struct ByteCodeFileHeader
{
	/** Magic number of the file type. */
	char magic[4] = BYTE_CODE_MAGIC_NUMBER;
	/** Total number of instructions that are stored in the file. */
	uint32_t instructionCount;
};
#pragma pack(pop)


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

/** Writes byte-code to a file. 
 * \param	fileName	Output file to write to.
 * \param	byteCode	Byte-code to write to the file.
 * \return	Returns 1 if the file could not be opened for writing, 2 if the data could not be written corretly or zero if no error occured. */
static uint32_t writeByteCodeToFile(const char* fileName, ByteCode& byteCode)
{
	FILE* file;
	size_t result;
	ByteCodeFileHeader header;

	/* MSVC specific version using the _s version to make the compiler happy.
	 * Try to open the specified file in binary, write-only mode. */
#ifdef _PHOTON_COMPILER_MSVC
	errno_t error = fopen_s(&file, fileName, "wb");
	if(error)
	{
		// Failed to open the file.
		return 1U;
	}
#else
	// Try to open the specified file in binary, write-only mode.
	file = fopen(fileName, "wb");
	if(!file)
	{
		// Failed to open the file.
		return 1U;
	}
#endif //_PHOTON_COMPILER_MSVC

	// Setup the byte-code header.
	header.instructionCount = byteCode.byteCodeInstructionCount;

	// Write the header to the file.
	result = fwrite(&header, sizeof(ByteCodeFileHeader), 1, file);
	if(result != 1)
	{
		// Incorrect number of bytes written.
		fclose(file);
		return 2U;
	}

	// Write the byte code blob to the file.
	size_t dataBlobSize = sizeof(Photon::RawInstruction) * byteCode.byteCodeInstructionCount;
	result = fwrite(byteCode.byteCode, dataBlobSize, 1, file);
	if(result != 1)
	{
		// Incorrect number of bytes written.
		fclose(file);
		return 2U;
	}

	// Close the file and release the buffer.
	fclose(file);
	return 0;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

/** Reads byte-code from a file. 
 * \param	fileName	Input file to read the byte-code from.
 * \param	byteCode	Byte-code object to read the data in.
 * \return	Returns 1 if the file could not be opened for reading, 2 if the data could not be read corretly, 3 if the byte-code format is invalid or zero if no error occured. */
static uint32_t loadByteCodeFromFile(const char* fileName, ByteCode& byteCode)
{
	FILE* file;
	size_t result;
	ByteCodeFileHeader header;

	/* Try to open the specified file in binary, read-only mode.
	 * MSVC specific version using the _s version to make the compiler happy. */
#ifdef _PHOTON_COMPILER_MSVC
	errno_t error = fopen_s(&file, fileName, "rb");
	if(error)
	{
		// Failed to open the file.
		return 1U;
	}
#else 
	file = fopen(fileName, "rb");
	if(!file)
	{
		// Failed to open the file.
		return 1U;
	}
#endif //_PHOTON_COMPILER_MSVC

	// Read the byte-code header.
	result = fread(&header, sizeof(ByteCodeFileHeader), 1, file);
	if(result != 1)
	{
		// Incorrect number of bytes read.
		fclose(file);
		return 2U;
	}

	// Check the magic number for the file format.
	const char magicNum[] = BYTE_CODE_MAGIC_NUMBER;
	if(memcmp(header.magic, magicNum, sizeof(const char) * 4) != 0)
	{
		// Incorrect file format read.
		fclose(file);
		return 3U;
	}

	// Create the byte code buffer.
	RawInstruction* byteCodeBuffer = new RawInstruction[header.instructionCount];

	// Read the byte code blob from the file.
	size_t dataBlobSize = sizeof(Photon::RawInstruction) * header.instructionCount;
	result = fread(byteCodeBuffer, dataBlobSize, 1, file);
	if(result != 1)
	{
		// Incorrect number of bytes read.
		delete[] byteCodeBuffer;
		fclose(file);
		return 3U;
	}

	// Setup the byte-code data.
	byteCode.byteCodeInstructionCount = header.instructionCount;
	byteCode.byteCode = byteCodeBuffer;

	// Close the file and release the buffer.
	fclose(file);
	return 0;
}


} /* End of Photon namespace. */


#endif //_VIRTUAL_MACHINE_H_