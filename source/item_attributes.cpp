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

#include "main.h"

#include "item_attributes.h"
#include "filehandle.h"

ItemAttributes::ItemAttributes() :
	attributes(nullptr)
{
	////
}

ItemAttributes::ItemAttributes(const ItemAttributes& o)
{
	if(o.attributes)
		attributes = newd ItemAttributeMap(*o.attributes);
}

ItemAttributes::~ItemAttributes()
{
	clearAllAttributes();
}

void ItemAttributes::createAttributes()
{
	if(!attributes)
		attributes = newd ItemAttributeMap;
}

void ItemAttributes::clearAllAttributes()
{
	if(attributes)
		delete attributes;
	attributes = nullptr;
}

ItemAttributeMap ItemAttributes::getAttributes() const
{
	if(attributes)
		return *attributes;
	return ItemAttributeMap();
}

void ItemAttributes::setAttribute(const std::string& key, const ItemAttribute& value)
{
	createAttributes();
	(*attributes)[key] = value;
}

void ItemAttributes::setAttribute(const std::string& key, const std::string& value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const std::string& key, int32_t value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const std::string& key, double value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::setAttribute(const std::string& key, bool value)
{
	createAttributes();
	(*attributes)[key].set(value);
}

void ItemAttributes::eraseAttribute(const std::string& key)
{
	if(!attributes)
		return;

	ItemAttributeMap::iterator iter = attributes->find(key);

	if(iter != attributes->end())
		attributes->erase(iter);
}

const std::string* ItemAttributes::getStringAttribute(const std::string& key) const
{
	if(!attributes)
		return nullptr;

	ItemAttributeMap::iterator iter = attributes->find(key);
	if(iter != attributes->end())
		return iter->second.getString();
	return nullptr;
}

const int32_t* ItemAttributes::getIntegerAttribute(const std::string& key) const
{
	if(!attributes)
		return nullptr;

	ItemAttributeMap::iterator iter = attributes->find(key);
	if(iter != attributes->end())
		return iter->second.getInteger();
	return nullptr;
}

const double* ItemAttributes::getFloatAttribute(const std::string& key) const
{
	if(!attributes)
		return nullptr;

	ItemAttributeMap::iterator iter = attributes->find(key);
	if(iter != attributes->end())
		return iter->second.getFloat();
	return nullptr;
}

const bool* ItemAttributes::getBooleanAttribute(const std::string& key) const
{
	if(!attributes)
		return nullptr;

	ItemAttributeMap::iterator iter = attributes->find(key);
	if(iter != attributes->end())
		return iter->second.getBoolean();
	return nullptr;
}

bool ItemAttributes::hasStringAttribute(const std::string& key) const
{
	return getStringAttribute(key) != nullptr;
}

bool ItemAttributes::hasIntegerAttribute(const std::string& key) const
{
	return getIntegerAttribute(key) != nullptr;
}

bool ItemAttributes::hasFloatAttribute(const std::string& key) const
{
	return getFloatAttribute(key) != nullptr;
}

bool ItemAttributes::hasBooleanAttribute(const std::string& key) const
{
	return getBooleanAttribute(key) != nullptr;
}


// Attribute type
// Can hold either int, bool or std::string
// Without using newd to allocate them

ItemAttribute::ItemAttribute() : type(ItemAttribute::NONE)
{
	////
}


ItemAttribute::ItemAttribute(const std::string& str) : type(ItemAttribute::STRING)
{
	new(data) std::string(str);
}

ItemAttribute::ItemAttribute(int32_t i) : type(ItemAttribute::INTEGER)
{
	*reinterpret_cast<int*>(data) = i;
}

ItemAttribute::ItemAttribute(double f) : type(ItemAttribute::DOUBLE)
{
	*reinterpret_cast<double*>(data) = f;
}

ItemAttribute::ItemAttribute(bool b)
{
	*reinterpret_cast<bool*>(data) = b;
}

ItemAttribute::ItemAttribute(const ItemAttribute& o) : type(ItemAttribute::NONE)
{
	*this = o;
}

ItemAttribute& ItemAttribute::operator=(const ItemAttribute& o)
{
	if(&o == this)
		return *this;

	clear();
	type = o.type;
	if(type == STRING)
		new(data) std::string(*reinterpret_cast<const std::string*>(&o.data));
	else if(type == INTEGER)
		*reinterpret_cast<int32_t*>(data) = *reinterpret_cast<const int32_t*>(&o.data);
	else if(type == FLOAT)
		*reinterpret_cast<float*>(data) = *reinterpret_cast<const float*>(&o.data);
	else if(type == DOUBLE)
		*reinterpret_cast<double*>(data) = *reinterpret_cast<const double*>(&o.data);
	else if(type == BOOLEAN)
		*reinterpret_cast<bool*>(data) = *reinterpret_cast<const bool*>(&o.data);
	else
		type = NONE;

	return *this;

}

ItemAttribute::~ItemAttribute()
{
	clear();
}

void ItemAttribute::clear()
{
	if(type == STRING) {
		(reinterpret_cast<std::string*>(&data))->~basic_string();
		type = NONE;
	}
}

void ItemAttribute::set(const std::string& str)
{
	clear();
	type = STRING;
	new(data) std::string(str);
}

void ItemAttribute::set(int32_t i)
{
	clear();
	type = INTEGER;
	*reinterpret_cast<int32_t*>(&data) = i;
}

void ItemAttribute::set(double y)
{
	clear();
	type = DOUBLE;
	*reinterpret_cast<double*>(&data) = y;
}

void ItemAttribute::set(bool b)
{
	clear();
	type = BOOLEAN;
	*reinterpret_cast<bool*>(&data) = b;
}

const std::string* ItemAttribute::getString() const
{
	if(type == STRING)
		return reinterpret_cast<const std::string*>(&data);
	return nullptr;
}

const int32_t* ItemAttribute::getInteger() const
{
	if(type == INTEGER)
		return reinterpret_cast<const int32_t*>(&data);
	return nullptr;
}

const double* ItemAttribute::getFloat() const
{
	if(type == DOUBLE)
		return reinterpret_cast<const double*>(&data);
	return nullptr;
}

const bool* ItemAttribute::getBoolean() const
{
	if(type == BOOLEAN)
		return reinterpret_cast<const bool*>(&data);
	return nullptr;
}

bool ItemAttributes::unserializeAttributeMap(const IOMap& maphandle, BinaryNode* stream)
{
	uint16_t n;
	if(stream->getU16(n)) {
		createAttributes();

		std::string key;
		ItemAttribute attrib;

		while(n--) {
			if(!stream->getString(key))
				return false;
			if(!attrib.unserialize(maphandle, stream))
				return false;
			(*attributes)[key] = attrib;
		}
	}
	return true;
}

void ItemAttributes::serializeAttributeMap(const IOMap& maphandle, NodeFileWriteHandle& f) const
{
	// Maximum of 65535 attributes per item
	f.addU16(std::min((size_t)0xFFFF, attributes->size()));

	ItemAttributeMap::const_iterator attribute = attributes->begin();
	int i = 0;
	while(attribute != attributes->end() && i <= 0xFFFF) {
		const std::string& key = attribute->first;
		if(key.size() > 0xFFFF)
			f.addString(key.substr(0, 65535));
		else
			f.addString(key);

		attribute->second.serialize(maphandle, f);
		++attribute, ++i;
	}
}

bool ItemAttribute::unserialize(const IOMap& maphandle, BinaryNode* stream)
{
	// Read type
	uint8_t rtype;
	stream->getU8(rtype);

	// Read contents
	switch(rtype) {
		case STRING: {
			std::string str;
			if(!stream->getLongString(str))
				return false;
			set(str);
			break;
		}
		case INTEGER: {
			uint32_t u32;
			if(!stream->getU32(u32))
				return false;
			set(*reinterpret_cast<int32_t*>(&u32));
			break;
		}
		case FLOAT: {
			uint32_t u32;
			if(!stream->getU32(u32))
				return false;
			set((double)*reinterpret_cast<float*>(&u32));
			break;
		}
		case DOUBLE: {
			uint64_t u64;
			if(!stream->getU64(u64))
				return false;
			set(*reinterpret_cast<double*>(&u64));
			break;
		}
		case BOOLEAN: {
			uint8_t b;
			if(!stream->getU8(b))
				return false;
			set(b != 0);
		}
		default:
			break;
	}
	return true;
}

void ItemAttribute::serialize(const IOMap& maphandle, NodeFileWriteHandle& f) const
{
	// Write type
	f.addU8((uint8_t)(type));

	// Write contents
	switch(type) {
		case STRING:
			f.addLongString(*getString());
			break;
		case INTEGER:
			f.addU32(*(uint32_t*)getInteger());
			break;
		case DOUBLE:
			f.addU64(*(uint64_t*)getFloat());
			break;
		case BOOLEAN:
			f.addU8(*(uint8_t*)getBoolean());
		default:
			break;
	}
}
