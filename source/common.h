//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_COMMONS_H_
#define RME_COMMONS_H_

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <iomanip>
#include <string>

#include "mt_rand.h"

//
inline bool testFlags(size_t flags, size_t test) {
	return (flags & test) != 0;
}

int32_t uniform_random(int32_t minNumber, int32_t maxNumber);
int32_t uniform_random(int32_t maxNumber);

// Function-like convertions between float, int and doubles
std::string i2s(int i);
std::string f2s(double i);
int s2i(std::string s);
double s2f(std::string s);
wxString i2ws(int i);
wxString f2ws(double i);
int ws2i(wxString s);
double ws2f(wxString s);

// replaces all instances of sought in str with replacement
void replaceString(std::string& str, const std::string sought, const std::string replacement);
// Removes all characters in t from source (from either start or beginning of the string)
void trim_right(std::string& source, const std::string& t);
void trim_left(std::string& source, const std::string& t);
// Converts the argument to lower/uppercase
void to_lower_str(std::string& source);
void to_upper_str(std::string& source);
std::string as_lower_str(const std::string& other);
std::string as_upper_str(const std::string& other);

// isFalseString returns true if the string is either "0", "false", "no", "not" or blank
// isTrueString returns the opposite value of isFalseString
bool isFalseString(std::string& str);
bool isTrueString(std::string& str);

// Generates a random number between low and high using the mersenne twister
int random(int high);
int random(int low, int high);

// Unicode conversions
std::wstring string2wstring(const std::string& utf8string);
std::string wstring2string(const std::wstring& widestring);

// Gets position values from ClipBoard
bool posFromClipboard(int& x, int& y, int& z);
bool posToClipboard(int x, int y, int z, int format);
bool posToClipboard(int fromx, int fromy, int fromz, int tox, int toy, int toz);

// Returns 'yes' if the defined value is true or 'no' if it is false.
wxString b2yn(bool v);

wxColor colorFromEightBit(int color);

#endif
