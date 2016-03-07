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
#ifndef _CMD_ARGUMENT_PARSER_H_
#define _CMD_ARGUMENT_PARSER_H_

/*----------------------------------------------------------------------------------------------------------------
 * Classes
 *--------------------------------------------------------------------------------------------------------------*/


/** 
 * \brief A utillity class to simplify the access of command line arguments.
 * 
 * An argument is defined as a key-value pair, where the key is the argument name and the value
 * is the assigned data, if any (Flag arguments).
 * To get an argument from the class you have to call the one of the <i>get</i> methods.
 * By specifying a default parameter a valid value will be returned if the searched command line
 * argument could not be found, e.g. default value of 800 for the variable WINDOW_WIDTH.
 * To check if an argument is existing use the CmdArguments::argumentExists method.
 *
 * The class automatically stores the first argument, which is always the execution path, in the
 * pre-defined argument named <i>"PATH"</i>.
 * Arguments are split from their data using the '=' character and prefixed with the '-' character.
 */
class CmdArgumentParser
{
public:
	/** Constructor of a command line argument list.
	 * \param	argc	Total number of arguments that are passed in.
	 * \param	argv	ASCII string that contains the command line arguments. */
	CmdArgumentParser(int argc, char** argv);
	/** Destructor of a command line argument list. */
	~CmdArgumentParser();


	/** Check if the specified argument exists in the command line argument list.
	 * \param	name	Name of the argument to check for.
	 * \return	Returns <b>true</b> if the argument was found in the list, otherwise it returns <b>false</b>. */
	const bool argumentExists(const char* name) const;

	
	/** Queries the command line argument list for the value of an argument as text.
	 * \note	Note that the value of the argument can <b>not</b> contain whitespaces.
	 * \param	name			Name of the argument to find.
	 * \param	defaultString	Default text that will be returned if the argument could not be found. Default is empty string.
	 * \return	Returns either the value of the searched argument or defaultString. */
	const char* getString(const char* name, const char* defaultString = "") const;
	/** Queries the command line argument list for the value of an argument as an integer.
	 * \param	name			Name of the argument to find.
	 * \param	defaultInt		Default value that will be returned if the argument could not be found. Default is zero.
	 * \return	Returns either the value of the searched argument or defaultInt. */
	int getInteger(const char* name, int defaultInt = 0) const;
	/** Queries the command line argument list for the value of an argument as a boolean.
	 * \param	name			Name of the argument to find.
	 * \param	defaultBool		Default value that will be returned if the argument could not be found. Default is <b>false</b>.
	 * \return	Returns either the value of the searched argument or defaultBool. */
	bool getBool(const char* name, bool defaultBool = false) const;


private:
	/** A structure that contains the key-value data of one command line argument. */
	struct Argument
	{
		const char*	key;	///< Key, used to identify the argument, e.g. WINDOW_WIDTH.
		const char*	value;	///< Value of the argument, e.g. 800, TRUE, HELLO.
	};

private:
	int		m_ArgumentCount;	///< Total numer of arguments that are given. One key-value pair counts as one argument.
	Argument*	m_Arguments;		///< An array of all arguments and their data.
};

#endif //_CMD_ARGUMENT_PARSER_H_