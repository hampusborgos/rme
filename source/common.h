//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/common.h $
// $Id: common.h 298 2010-02-23 17:09:13Z admin $

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
double frand();

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
bool posFromClipboard(Position& position, const int& mapWidth = MAP_MAX_WIDTH, const int& mapHeight = MAP_MAX_HEIGHT);

// Returns 'yes' if the defined value is true or 'no' if it is false.
wxString b2yn(bool v);

// Standard math functions
template <class T>
inline T abs(T t) {
	return (t < 0? -t : t);
}

template <class T, class U>
inline T min(T t, U u) {
	return (t < u? t : u);
}

template <class T, class U>
T max(T t, U u) {
	return (t > u? t : u);
}

template <class T, class U, class V>
inline T min(T t, U u, V v) {
	int min = t;
	if ( u < min ) min = u;
	if ( v < min ) min = v;
	return min;
}

template <class T, class U, class V>
inline T max(T t, U u, V v) {
	int max = t;
	if ( u > max ) max = u;
	if ( v > max ) max = v;
	return max;
}

#endif
