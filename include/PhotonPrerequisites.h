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
#ifndef _PHOTON_PREREQUISITES_H_
#define _PHOTON_PREREQUISITES_H_


/* All includes that exist outside of the build directory. */
#include <cctype>
#include <cstring>
#include <queue>
#include <sstream>

/* Define compiler specific macros.
 * This is used to use the _s versions of the functions like fopen on Windows. */
#ifdef _MSC_VER
	#define _PHOTON_COMPILER_MSVC
#endif


namespace Photon
{


/*----------------------------------------------------------------------------------------------------------------
 * Type definitions
 *--------------------------------------------------------------------------------------------------------------*/  

/** Define a raw/encoded instruction as a 16-bit signed integer. */
typedef int16_t RawInstruction;
/** Define the type of a register. */
typedef int32_t RegisterType;
/** Define an exit code that can be emitted by the VM or user code. */
typedef uint8_t VMExitCode;


/*----------------------------------------------------------------------------------------------------------------
 * Enumerations
 *--------------------------------------------------------------------------------------------------------------*/  

/** Enumerations of all verbosity levels of the VM. */
enum VerbosityLevel
{
	/** Disables all messages. */
	VerbosityLevelSilent	= 0,
	/** Display warnings. */
	VerbosityLevelWarning	= 1,
	/** Display error messages. */
	VerbosityLevelError		= 2,
	/** Display debugging information. */
	VerbosityLevelDebugInfo	= 4,

	/** Display all warnings errors and debug info. */
	VerbosityLevelAll		= VerbosityLevelError | VerbosityLevelWarning | VerbosityLevelDebugInfo,
	/** Default verbosity level. This only shows warnings and errors. */
	VerbosityLevelDefault	= VerbosityLevelError | VerbosityLevelWarning
};


/** Exit codes that can be emitted by the VM itself. 
 * Note that the first error code that should be used by any non VM code. User errors should start at 
 * <i>ExitCodeUserCode + userCodeNumber</i> as they will otherwise conflict with the values below. */
enum VMExitCodes
{
	/** Signals success. */
	ExitCodeSuccess = 0,
	/** Signals that the VM should be halted by a user request. 
	 * This does not mean that the VM has finished execution of the byte-code. */
	ExitCodeHaltRequested,
	/** Signals a division by zero error. */
	ExitCodeDivideByZero,
	/** Signals that the offset of a jump-instruction is out of bounds. */
	ExitCodeJumpOutOfBounds,
	/** Signals that the byte-code tried to access an invalid register. */
	ExitCodeRegisterFault,
	/** Signals that a Host-Call function was requested but could not be found.
	 * \note This will only be signalled if PHOTON_IS_HOST_CALL_STRICT is enabled. */
	ExitCodeInvalidHostCall,
	/** First code that can be emitted by user code. */
	ExitCodeUserCode
};


} /* End of Photon namespace. */

#endif //_PHOTON_PREREQUISITES_H_
