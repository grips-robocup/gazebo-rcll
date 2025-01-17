
/***************************************************************************
 *  string_split.h - Split string functions
 *
 *  Created: Wed Apr 03 18:01:30 2013
 *  Copyright  2006-2013  Tim Niemueller [www.niemueller.de]
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version. A runtime exception applies to
 *  this software (see LICENSE.GPL_WRE file mentioned below for details).
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL_WRE file in the doc directory.
 */

#ifndef __UTILS_MISC_STRING_SPLIT_H_
#define __UTILS_MISC_STRING_SPLIT_H_

#include <list>
#include <queue>
#include <string>
#include <vector>

namespace fawkes {

/** Split string by delimiter.
 * @param s string to split
 * @param delim delimiter
 * @return vector of split strings
 */
static inline std::vector<std::string>
str_split(const std::string &s, char delim = '/')
{
	std::vector<std::string> elems;
	std::stringstream        ss(s);
	std::string              item;
	while (std::getline(ss, item, delim)) {
		if (item != "")
			elems.push_back(item);
	}
	return elems;
}

/** Split string by delimiter.
 * @param s string to split
 * @param delim delimiter
 * @return queue of split strings
 */
static inline std::queue<std::string>
str_split_to_queue(const std::string &s, char delim = '/')
{
	std::queue<std::string> elems;
	std::stringstream       ss(s);
	std::string             item;
	while (std::getline(ss, item, delim)) {
		if (item != "")
			elems.push(item);
	}
	return elems;
}

/** Join list of strings string using given delimiter.
 * @param l list with strings to join
 * @param delim delimiter
 * @return string of strings in list separated by given delimiter
 */
static inline std::string
str_join(const std::list<std::string> &l, char delim = '/')
{
	std::string rv;
	bool        first = true;
	for (std::list<std::string>::const_iterator i = l.begin(); i != l.end(); ++i) {
		if (first)
			first = false;
		else
			rv += delim;
		rv += *i;
	}
	return rv;
}

/** Join list of strings string using given delimiter.
 * @param l list with strings to join
 * @param delim delimiter
 * @return string of strings in list separated by given delimiter
 */
static inline std::string
str_join(const std::list<std::string> &l, std::string delim)
{
	std::string rv;
	bool        first = true;
	for (std::list<std::string>::const_iterator i = l.begin(); i != l.end(); ++i) {
		if (first)
			first = false;
		else
			rv += delim;
		rv += *i;
	}
	return rv;
}

} // end namespace fawkes

#endif
