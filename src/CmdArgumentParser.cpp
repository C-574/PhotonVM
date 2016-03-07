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
#include "CmdArgumentParser.h"
#include <assert.h>
#include <cstdlib>
#include <cstring>


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

CmdArgumentParser::CmdArgumentParser(int argc, char** argv)
{
	assert(argc > 0);

	// Set the argument count and initialize the array.
	m_ArgumentCount = argc;
	m_Arguments = new Argument[argc];

	// Map the fist argument to the variable PATH.
	m_Arguments[0].key = "PATH";
	m_Arguments[0].value = argv[0];

	// Iterate over all arguments. Ignore the PATH agument.
	for(int i = 1; i < argc; ++i)
	{

		// MSVC specific version using the _s version to make the compiler happy.
#ifdef _MSC_VER
		char* nextToken = nullptr;
		// Split the argument to get the name.
		m_Arguments[i].key = strtok_s(argv[i], " =-", &nextToken);
		// Split again to get the value.
		m_Arguments[i].value = strtok_s(NULL, " =-", &nextToken);
#else
		// Split the argument to get the name.
		m_Arguments[i].key = strtok(argv[i], " =-");
		// Split again to get the value.
		m_Arguments[i].value = strtok(nullptr, " =-");
#endif //_MSC_VER

	}
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

CmdArgumentParser::~CmdArgumentParser()
{
	if(m_Arguments)
		delete[] m_Arguments;
	m_ArgumentCount = 0;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

const bool CmdArgumentParser::argumentExists(const char* name) const
{
	// Iterate over all arguments.
	for(int i = 0; i < m_ArgumentCount; ++i)
	{
		// Are the strings equal?
		if(strcmp(name, m_Arguments[i].key) == 0)
			return true;
	}

	// Nothing found
	return false;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

const char* CmdArgumentParser::getString(const char* name, const char* defaultString) const
{
	// Iterate over all arguments.
	for(int i = 0; i < m_ArgumentCount; ++i)
	{
		// Are the strings equal?
		if(strcmp(name, m_Arguments[i].key) == 0)
			return (m_Arguments[i].value != nullptr ? m_Arguments[i].value : defaultString);
	}

	return defaultString;
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

int CmdArgumentParser::getInteger(const char* name, int defaultInt) const
{
	// Get the argument as string.
	const char* str = getString(name, nullptr);
	if(str == nullptr)
		return defaultInt;

	// Try to convert the string to an integer value.
	char* end;
	return strtol(str, &end, 10);
}


/*----------------------------------------------------------------------------------------------------------------
 * 
 *--------------------------------------------------------------------------------------------------------------*/

bool CmdArgumentParser::getBool(const char* name, bool defaultBool) const
{
	// Get the argument as string.
	const char* str = getString(name, nullptr);
	if(str == nullptr)
		return defaultBool;

	// Maually check for 'TRUE' or 'FALSE'.
	if(strcmp(str, "TRUE") == 0)
		return true;
	else if(strcmp(str, "FALSE") == 0)
		return false;

	// Otherwise try to convert the string to an integer value and check against it.
	char* end;
	return (strtol(str, &end, 10) > 0);
}
