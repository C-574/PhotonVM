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


// Current version number of the compiled byte-code.
#define PHOTON_BYTE_CODE_VERSION 1U


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


/** Clear the instruction and reset it to its defaults. 
 * \param	instruction	Instruction to clear. */
inline void resetInstruction(Instruction* instruction)
{
	instruction->instructionCode = InstructionHalt;
	instruction->param1 = 0;
	instruction->param2 = 0;
	instruction->param3 = 0;
	instruction->value  = 0;
}


/*----------------------------------------------------------------------------------------------------------------
 * BYTE CODE
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
	return (byteCode->instructions && byteCode->instructionCount);
}

/** Release the byte-code data. This will deallocate the instruction data and reset the instruction count to zero. 
 * After this method has been called the specified byte-code is invalidated (isByteCodeValid will return <b>false</b>) and the VM will no longer be able to execute it.
 * \param	byteCode	Byte-code to release. */
inline void releaseByteCode(ByteCode* byteCode)
{
	if(byteCode && isByteCodeValid(byteCode))
	{
		delete[] byteCode->instructions;
		byteCode->instructions = nullptr;
		byteCode->instructions = 0U;
	}
}


/*----------------------------------------------------------------------------------------------------------------
 * Byte-Code Loader/Writer
 *--------------------------------------------------------------------------------------------------------------*/

#pragma pack(push, 1)
/** A structire that contains the header of a Photon byte-code file. */
struct ByteCodeFileHeader
{
	/** Magic number of the file type. */
	uint8_t magic[3] = {'P', 'B', 'C'};
	/** Version number of the compiled Photon byte-code. */
	uint8_t version = PHOTON_BYTE_CODE_VERSION;
	/** Total number of instructions that are stored in the file. */
	uint32_t instructionCount;
};
#pragma pack(pop)


/** Enumeration of all results that can be returned by a byte-code reader or writer. */
enum ByteCodeReadWriteResult
{
	/*-- Reader and writer. --*/

	/** The reader/writer was successfull. */
	ByteCodeReadWriteResultSuccess,
	/** The reader/writer has failed to open the spesified file on disk for reading or writing. */
	ByteCodeReadWriteResultFileOpenFailed,
	/** The reader/writer has read or written an incorrect number of bytes from/to a file. */
	ByteCodeReadWriteResultIncorrectData,

	/*-- Reader only. --*/

	/** The reader could not load the specified file because it does not contain Photon byte-code. */
	ByteCodeReadWriteResultInvalidFileType,
	/** The reader could not load the specified file because the byte-code to load was compiled with an newer version 
	 * of the VM byte-code and may be incompatible. Photon byte-code is not forward compatible. */
	ByteCodeReadWriteResultIncompatibleByteCode,
	/** The reader loaded the byte-code but warns that the used byte-code version and the used instruction set may
	 * have changed and are now deprecated. The code may work as expected if no breaking changes have been introduced
	 * but this is not guaranteed. It is recommanded to recompile the code using a newer version of Photon. This warning
	 * is for backwards compatibillity of byte-code and can be ignored in some cases. */
	ByteCodeReadWriteResultDeprecatedWarning
};


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

/** Writes byte-code to a file. 
 * \param	fileName	Output file to write to.
 * \param	byteCode	Byte-code to write to the file.
 * \return	Returns ByteCodeReadWriteResultSuccess if the data was written successfully or otherwise one of the other ByteCodeReadWriteResult values. */
static ByteCodeReadWriteResult writeByteCodeToFile(const char* fileName, ByteCode& byteCode)
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
		return ByteCodeReadWriteResultFileOpenFailed;
	}
#else
	// Try to open the specified file in binary, write-only mode.
	file = fopen(fileName, "wb");
	if(!file)
	{
		// Failed to open the file.
		return ByteCodeReadWriteResultFileOpenFailed;
	}
#endif //_PHOTON_COMPILER_MSVC

	// Setup the byte-code header.
	header.instructionCount = byteCode.instructionCount;

	// Write the header to the file.
	result = fwrite(&header, sizeof(ByteCodeFileHeader), 1, file);
	if(result != 1)
	{
		// Incorrect number of bytes written.
		fclose(file);
		return ByteCodeReadWriteResultIncorrectData;
	}

	// Write the byte-code blob to the file.
	size_t dataBlobSize = sizeof(Photon::RawInstruction) * byteCode.instructionCount;
	result = fwrite(byteCode.instructions, dataBlobSize, 1, file);
	if(result != 1)
	{
		// Incorrect number of bytes written.
		fclose(file);
		return ByteCodeReadWriteResultIncorrectData;
	}

	// Close the file and release the buffer.
	fclose(file);
	return ByteCodeReadWriteResultSuccess;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

/** Reads byte-code from a file. 
 * \param	fileName	Input file to read the byte-code from.
 * \param	byteCode	Byte-code object to read the data in.
 * \return	Returns ByteCodeReadWriteResultSuccess if the data was read successfully or otherwise one of the other ByteCodeReadWriteResult values. */
static ByteCodeReadWriteResult loadByteCodeFromFile(const char* fileName, ByteCode& byteCode)
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
		return ByteCodeReadWriteResultFileOpenFailed;
	}
#else 
	file = fopen(fileName, "rb");
	if(!file)
	{
		// Failed to open the file.
		return ByteCodeReadWriteResultFileOpenFailed;
	}
#endif //_PHOTON_COMPILER_MSVC

	// Read the byte-code header.
	result = fread(&header, sizeof(ByteCodeFileHeader), 1, file);
	if(result != 1)
	{
		// Incorrect number of bytes read.
		fclose(file);
		return ByteCodeReadWriteResultIncorrectData;
	}

	// Check the magic number of the file header.
	if( header.magic[0] != 'P' ||
		header.magic[1] != 'B' ||
		header.magic[2] != 'C')
	{
		// Incorrect file format read.
		fclose(file);
		return ByteCodeReadWriteResultInvalidFileType;
	}

	// Check the version of the byte-code and decide if we can load the specified version.
	// TODO(C-574): Should this condition really be forbidden or should we just warn the user?
	if(header.version > PHOTON_BYTE_CODE_VERSION)
	{
		fclose(file);
		return ByteCodeReadWriteResultIncompatibleByteCode;
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
		return ByteCodeReadWriteResultIncorrectData;
	}

	// Setup the byte-code data.
	byteCode.instructionCount	= header.instructionCount;
	byteCode.instructions		= byteCodeBuffer;

	// Close the file and release the buffer.
	fclose(file);

	// Check if the loaded byte-code version is deprecated.
	return (header.version < PHOTON_BYTE_CODE_VERSION ? 
		ByteCodeReadWriteResultDeprecatedWarning : ByteCodeReadWriteResultSuccess);
}


} /* End of Photon namespace. */


#endif //_VIRTUAL_MACHINE_H_