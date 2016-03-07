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
#ifndef _HOST_CALL_H_
#define _HOST_CALL_H_

// Include all needed prerequisites of the library.
#include "PhotonPrerequisites.h"


namespace Photon 
{

/*----------------------------------------------------------------------------------------------------------------
 * Macros and Definitions
 *--------------------------------------------------------------------------------------------------------------*/  



/** Macro to quickly define a Host-Call. For an example on how to use the macro, please see the documentation.
 * \param	callClassName	Name of the Host-Call class, typically "HC_MyHostCall". This name is used when registering the call. 
 * \param	groupId			Id of the group that the call is assigned to. Used when calling from byte-code.
 * \param	functionId		Function id that the call is assigned to. Used when calling from byte-code. 
 * \param	functionName	Name of the Host-Call function as an ASCII string. Mainly used for debugging calls. */
#define DECLARE_SIMPLE_HOST_CALL_BEGIN(callClassName, groupId, functionId, functionName) class callClassName : public Photon::IHostCallFunction { \
		public: \
		callClassName() : IHostCallFunction(groupId, functionId, functionName)	{ } \
		~callClassName() { } \
		void execute(Photon::RegisterType* registers) override
		
/** End macro for the DECLARE_SIMPLE_HOST_CALL_BEGIN macro. Every declaration needs to end with the end macro. */
#define DECLARE_SIMPLE_HOST_CALL_END() };


/*----------------------------------------------------------------------------------------------------------------
 * Utillity Functions
 *--------------------------------------------------------------------------------------------------------------*/  



/** Packs two 8-bit values in one 16-bit value. 
 * \param	mostSignificantByte		The most significant byte to pack.
 * \param	leastSignificantByte	The least significant byte to pack. 
 * \return	Returns the packed value. */
inline uint16_t packTo16Bit(const uint8_t mostSignificantByte, const uint8_t leastSignificantByte)
{
	/* Layout: group   | function
	 *		   00000000 11111111	*/
	return (mostSignificantByte << 8 /*Bit-size of one uint8_t.*/ | leastSignificantByte);
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  



/** 
 * \brief An interface for a function that is callable from byte code.
 *
 * A Host-Call function is a function that is implemented in the host language (C++) and exposed to the byte-code for execution.
 * The function has the abillity to read <b>and</b> write to all VM registers. It is called by the <i>hcall</i> instruction with
 * two parameters. The first parameter defines the group in which the function is stored and the second defines the id of the
 * function inside of the group. This allows the system to make up to <b>65.535</b> functions acessable from byte-code with a 
 * lookup speed of <i>O(1)</i> seperated into up to 16 different groups.
 *
 * To implement a Host-Call function, just subclass from this class and implement the two methods:
 *	- constructor: Implement a custom constructor and call to the parent class constructor to set up the IDs. Note that the IDs 
 *		are fixed <b>per call type</b>!
 *	- execute: Implement the method that executes the callable code.
 * 
 * This <i>virtual-class</i> aproach is used because it can be easily integrated into an object-oriented environment. This means 
 * that an instance of the call can also operate on other OOP objects of the host code.
 */
class IHostCallFunction
{
public:
	/** Destructor of a Host-Call function. */
	virtual ~IHostCallFunction()
	{
	}


	/** Virtual method that needs to be implemented to execute the custom operation.
	 * \param	registers	Register set of the VM at the current execution state. Readable and writable. */
	virtual void execute(RegisterType* registers) = 0;


	/** Get the packed id that contains the group and function id. */
	const inline uint16_t getPackedId() const { return m_Id; }


	/** Get the group if of the host function call. */
	const inline uint8_t getGroupId() const { return (m_Id >> 8); }

	/** Get the function id of the host function call. */
	const inline uint8_t getFunctionId() const { return (m_Id & 0x00FF); }


	/** Get the function name of the Host-Call function. */
	const inline char* getName() const { return m_FunctionName; }


protected:
	/** Constructor of a Host-Call function. 
	 * \param	groupId		Id of the group that the method is assigned to, e.g. 6 = Utillity Functions. Ranges from 0 to 255.
	 * \param	functionId	Id of the function in the group. Ranges from 0 to 255.
	 * \param	name		Name of the Host-Call function. Mostly used for debugging. */
	IHostCallFunction(const uint8_t groupId, const uint8_t functionId, const char* name) : m_FunctionName(name)
	{
		// Pack the ids into one integer id.
		m_Id = packTo16Bit(groupId, functionId);
	}

	/** Get a value from the registers. 
	 * \param	registerIndex	Index of the register to get.
	 * \param	registers		VM register array to get the value from.
	 * \return	Returns a reference to the value of the register at the register index. */
	RegisterType& getValue(const int32_t registerIndex, RegisterType* registers)
	{
		return registers[registerIndex - 1];
	}

private:
	/** Hidden copy constructor to prevent copying of the class. */
	IHostCallFunction(const IHostCallFunction& other)
	{
	}


private:
	/** Packed ids for group and function. */
	uint16_t m_Id;

	/** ASCII name of the Host-Call function. */
	const char* m_FunctionName;
};



/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/  


/** 
 * \brief A container for Host-Call functions.
 *
 * The maximum number of Host-Call functions that can be registered is defined by MAX_NUM_HOST_CALLS, adjust this value if 
 * you need more/less functions. Register a new function call by calling the registerHostCallFunction method. If a call is 
 * registered, then the container does <b>not</b> take over the ownership of the object. One instance of a Host-Call can be
 * registered with multiple containers at a time.
 *
 * \note Currently there is no way to <i>unregister</i> a Host-Call from a container. This will change some day in the future.
 */
class HostCallFunctionContainer
{
public:
	/** Constructor of a Host-Call container. */
	HostCallFunctionContainer() : 
		m_CurrentRegisteredNum(0), m_FirstEntryIndex(MAX_NUM_HOST_CALLS)
	{
		// Clear the host list to zero.
		memset(m_CallList, 0, sizeof(m_CallList));
	}

	/** Destructor of a Host-Call container. */
	~HostCallFunctionContainer()
	{
		uint16_t clearedEntries = 0;

		for(uint16_t i = m_FirstEntryIndex; i < MAX_NUM_HOST_CALLS; i++)
		{
			// Check if we have released all registered entries.
			if(clearedEntries >= m_CurrentRegisteredNum)
				break;

			// Ignore empty slots.
			if(!m_CallList[i])
				continue;

			// Reset the entry.
			m_CallList[i] = nullptr;
			clearedEntries++;
		}
	}


	/** Register a new Host-Call. If the same call is already registered then the old one will be overwritten.
	 * When a Host-Call is registered then the container does <b>not</b> take over the ownership of the call instance.
	 * \param	hostCallFunction	Host-Call object to register. 
	 * \return	Returns <b>true</b> if the function was registered sucessfully or <b>false</b> if it failed because the function call id (group|function) are out of valid bounds or the specified object is invalid. */
	bool registerHostCallFunction(IHostCallFunction* hostCallFunction)
	{
		if(!hostCallFunction)
			return false;

		// Get the id of the function.
		const uint16_t id = hostCallFunction->getPackedId();


		// Check if the index is out of bounds.
		if(id >= MAX_NUM_HOST_CALLS)
			return false;

		// Store the new function.
		m_CallList[id] = hostCallFunction;

		// Increment the registered counter.
		m_CurrentRegisteredNum++;

		// If the registered call has an index that is smaller than the first known, replace it.
		if(id < m_FirstEntryIndex)
			m_FirstEntryIndex = id;

		return true;
	}

	/** Get the total number of currently registered Host-Calls. */
	const inline uint16_t getNumRegisteredHostCalls() const { return m_CurrentRegisteredNum; }


	/** Get a reference to a Host-Call function.
	 * \param	groupId		Id of the group that the method is assigned to, e.g. 6 = Utillity Functions. Ranges from 0 to 255.
	 * \param	functionId	Id of the function in the group. Ranges from 0 to 255. 
	 * \return	Returns either a pointer to the specified Host-Call function object or <b>nullptr</b> if no objects exists with the specified parameters. */
	IHostCallFunction* getHostCallFunction(const uint8_t& groupId, const uint8_t& functionId) const
	{
		// Get the id by packing the values.
		const uint16_t id = packTo16Bit(groupId, functionId);

		// Check if the id is out of bounds.
		if(id >= MAX_NUM_HOST_CALLS)
			return nullptr;
		
		// Return whatever is at the specified id.
		return m_CallList[id];
	}


private:
	/** A list that contains all Host-Calls that are avilable to the byte code. The total number of Host-Calls is defined in MAX_NUM_HOST_CALLS. */
	IHostCallFunction* m_CallList[MAX_NUM_HOST_CALLS];

	/** Total number of currently registered Host-Calls. */
	uint16_t m_CurrentRegisteredNum;

	/** Index of the first entry in the list. */
	uint16_t m_FirstEntryIndex;
};

}

#endif //_HOST_CALL_H_