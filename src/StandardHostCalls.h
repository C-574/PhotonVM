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
#ifndef _STANDARD_HOST_CALLS_H_
#define _STANDARD_HOST_CALLS_H_

/*
 * This file defines a set of standard Host-Call that have to exist in all implementations (also embedded) to provide a basic interface.
 */

#include "PhotonVM.h"


/** Pack all standard Host-Calls into a single namespace. */
namespace PhotonStandardHostCalls
{

	/** Enumeration of all standard Host-Call function groups. */
	enum STD_HC_GROUPS
	{
		STD_HC_GROUP_CORE = 0 ///< Core group containing all basic calls.
	};

	/** Enumeration of all standard Host-Call function ids. */
	enum STD_HC_FUNCTIONS
	{
		STD_HC_FUNCTION_PRINT_VERSION = 0,	///< Print the version info to the standard output.
		STD_HC_FUNCTION_GET_VERSION,		///< Get the version number of the VM that the byte-code runs on.
		STD_HC_FUNCTION_PRINT_VALUE,		///< Print the value to the standard output that is stored in the 'local' register as a signed integer.
		STD_HC_FUNCTION_PRINT_CHARACTER		///< Print the chacacter to the standard output that is stored in the 'local' register.
	};


	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/ 

	/** Print the VM version number to the standard output. */
	DECLARE_SIMPLE_HOST_CALL_BEGIN(STD_HC_PrintVersion, STD_HC_GROUP_CORE, STD_HC_FUNCTION_PRINT_VERSION, "PrintVersion")
	{
		printf("Photon Virtual-Machine V.%d.%d.%d\n", PHOTON_VM_VERSION_MAJOR, PHOTON_VM_VERSION_MINOR, PHOTON_VM_VERSION_SUB_MINOR);
	}
	DECLARE_SIMPLE_HOST_CALL_END()


	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/ 

	/** Get the version number of the VM. */
	DECLARE_SIMPLE_HOST_CALL_BEGIN(STD_HC_GetVersion, STD_HC_GROUP_CORE, STD_HC_FUNCTION_GET_VERSION, "GetVersion")
	{
		Photon::RegisterType param = registers[Photon::VirtualMachine::Local];
		if(param == 3)
			registers[Photon::VirtualMachine::Local] = PHOTON_VM_VERSION_SUB_MINOR;
		else if(param == 2)
			registers[Photon::VirtualMachine::Local] = PHOTON_VM_VERSION_MINOR;
		else
			registers[Photon::VirtualMachine::Local] = PHOTON_VM_VERSION_MAJOR;
	}
	DECLARE_SIMPLE_HOST_CALL_END()


	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/ 

	/** Print the value at 'local' as a signed integer value to the standard output. */
	DECLARE_SIMPLE_HOST_CALL_BEGIN(STD_HC_PrintValue, STD_HC_GROUP_CORE, STD_HC_FUNCTION_PRINT_VALUE, "PrintValue")
	{
		fprintf(stdout, "%d", registers[Photon::VirtualMachine::Local]);
	}
	DECLARE_SIMPLE_HOST_CALL_END()


	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/ 

	/** Print the value at 'local' as a character to the standard output. */
	DECLARE_SIMPLE_HOST_CALL_BEGIN(STD_HC_PrintCharacter, STD_HC_GROUP_CORE, STD_HC_FUNCTION_PRINT_CHARACTER, "PrintCharacter")
	{
		putc(registers[Photon::VirtualMachine::Local], stdout);
	}
	DECLARE_SIMPLE_HOST_CALL_END()


	/*----------------------------------------------------------------------------------------------------------------
	 * 
	 *--------------------------------------------------------------------------------------------------------------*/ 

	/** Registers all standard Host-Call functions to the specified VM instance.
	 * \param	vm	Instance of a virtual machine to register the calls with. */
	void registerStandardHostCalls(Photon::VirtualMachine& vm);

}

#endif //_STANDARD_HOST_CALLS_H_