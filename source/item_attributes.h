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

#ifndef RME_ITEM_ATTRIBUTES_H_
#define RME_ITEM_ATTRIBUTES_H_

#include <string>
#include <map>

#include "filehandle.h"

#include <boost/static_assert.hpp>

class IOMap;
class ItemAttribute;

class PropWriteStream;
class PropStream;

class ItemAttribute
{
public:
	ItemAttribute();
	ItemAttribute(const std::string& str);
	ItemAttribute(int32_t i);
	ItemAttribute(double f);
	ItemAttribute(bool b);
	ItemAttribute(const ItemAttribute& o);
	ItemAttribute& operator=(const ItemAttribute& o);
	~ItemAttribute();

	enum Type {
		STRING = 1,
		INTEGER = 2,
		FLOAT = 3,
		BOOLEAN = 4,
		DOUBLE = 5,
		NONE = 0
	} type;

	void serialize(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	bool unserialize(const IOMap& maphandle, BinaryNode* f);

	void clear();

	void set(const std::string& str);
	void set(int32_t i);
	void set(double f);
	void set(bool b);

	const std::string* getString() const;
	const int32_t* getInteger() const;
	const double* getFloat() const;
	const bool* getBoolean() const;

private:
	char data[sizeof(std::string) > sizeof(double) ? sizeof(std::string) : sizeof(double)];
};

typedef std::map<std::string, ItemAttribute> ItemAttributeMap;

class ItemAttributes
{
public:
	ItemAttributes();
	ItemAttributes(const ItemAttributes &i);
	virtual ~ItemAttributes();

	// Save / load
	void serializeAttributeMap(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	bool unserializeAttributeMap(const IOMap& maphandle, BinaryNode* node);

public:
	void setAttribute(const std::string& key, const ItemAttribute& attr);
	void setAttribute(const std::string& key, const std::string& value);
	void setAttribute(const std::string& key, int32_t value);
	void setAttribute(const std::string& key, double value);
	void setAttribute(const std::string& key, bool set);

	// returns nullptr if the attribute is not set
	const std::string* getStringAttribute(const std::string& key) const;
	const int32_t* getIntegerAttribute(const std::string& key) const;
	const double* getFloatAttribute(const std::string& key) const;
	const bool* getBooleanAttribute(const std::string& key) const;

	// Returns true if the attribute (of that type) exists
	bool hasStringAttribute(const std::string& key) const;
	bool hasIntegerAttribute(const std::string& key) const;
	bool hasFloatAttribute(const std::string& key) const;
	bool hasBooleanAttribute(const std::string& key) const;

	void eraseAttribute(const std::string& key);

	void clearAllAttributes();
	ItemAttributeMap getAttributes() const;

protected:
	ItemAttributeMap* attributes;

	void createAttributes();
};

#endif
