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
#ifndef _PHOTON_VM_H_
#define _PHOTON_VM_H_

// Include all needed prerequisites of the library.
#include "PhotonPrerequisites.h"


/*----------------------------------------------------------------------------------------------------------------
 * Build Settings
 *--------------------------------------------------------------------------------------------------------------*/  

// Settings of the VM for compilation of the C++ code.
#ifndef PHOTON_IS_HOST_CALL_STRICT
#define PHOTON_IS_HOST_CALL_STRICT 0 // Enable or disable strictness of Host-Calls. If enabled and no Host-Call can be found for a hcall instruction the VM will halt, otherwise it will continue.
#endif // PHOTON_IS_HOST_CALL_STRICT

#ifndef PHOTON_WARNINGS_ENABLED
#define PHOTON_WARNINGS_ENABLED 1 // Enable or disable compile and runtime warnings.
#endif // PHOTON_WARNINGS_ENABLED

#ifndef PHOTON_DEBUG_CALLBACK_ENABLED
#define PHOTON_DEBUG_CALLBACK_ENABLED 0 // Enable or disable the user debug callback on the virtual machine.
#endif // PHOTON_DEBUG_CALLBACK_ENABLED

/** Total number of Host-Calls that can be registered at once. This can be reduced if fewer calls are used.
 * The maximum number of calls is: 0xFFFFU = 4096U. Note that one group always consists of 256 functions. */
static const uint16_t MAX_NUM_HOST_CALLS = 8U;


/*----------------------------------------------------------------------------------------------------------------
 * Version Information
 *--------------------------------------------------------------------------------------------------------------*/  

/** Version number of the PhotonVM in X.YYY.ZZ (Major, Minor, Sub-Minor) format. Change only this value if the version changes. */
static const int32_t PHOTON_VM_VERSION				= 100000;
/** Major version extracted from PHOTON_VM_VERSION. */
static const int32_t PHOTON_VM_VERSION_MAJOR		= (PHOTON_VM_VERSION / 100000);
/** Minor version extracted from PHOTON_VM_VERSION. */
static const int32_t PHOTON_VM_VERSION_MINOR		= ((PHOTON_VM_VERSION / 100) % 1000);
/** Sub-Minor version extracted from PHOTON_VM_VERSION. */
static const int32_t PHOTON_VM_VERSION_SUB_MINOR	= (PHOTON_VM_VERSION % 100);


/*----------------------------------------------------------------------------------------------------------------
 * Includes
 *--------------------------------------------------------------------------------------------------------------*/  

// Include all needed headers to use the VM properly in code.
#include "VirtualMachine.h"
#include "Lexer.h"


#endif //_PHOTON_VM_H_
