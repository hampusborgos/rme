//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/complexitem.h $
// $Id: complexitem.h 298 2010-02-23 17:09:13Z admin $

#ifndef RME_CONTAINER_H_
#define RME_CONTAINER_H_

#include "position.h"
#include "item.h"

class Container : public Item {
public:
	Container(const unsigned short _type);
	virtual ~Container();

	virtual Item* deepCopy() const;
	
	Item* getItem(uint index);
	uint getItemCount() const {return contents.size();}
	uint getVolume() const {return item_db[id].volume;}
	ItemVector& getVector();
	virtual double getWeight();

	virtual bool unserializeItemNode_OTBM(const IOMap& maphandle, BinaryNode* node);
	virtual bool serializeItemNode_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	//virtual bool unserializeItemNode_OTMM(const IOMap& maphandle, BinaryNode* node);
	//virtual bool serializeItemNode_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;

protected:
	ItemVector contents;
};

#pragma pack(1)

struct OTBM_TeleportDestination {
	uint16_t x;
	uint16_t y;
	uint8_t z;
};

#pragma pack()


class Teleport : public Item {
public:
	Teleport(const unsigned short _type);
	virtual ~Teleport();

	virtual Item* deepCopy() const;

	virtual void serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	virtual bool readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attr, BinaryNode* node);
	//virtual void serializeItemAttributes_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	//virtual bool readItemAttribute_OTMM(const IOMap& maphandle, OTMM_ItemAttribute attr, BinaryNode* node);

	int getX() const;
	int getY() const;
	int getZ() const;
	
	bool noDestination() const {return destination != Position();}
	Position getDestination() const {return destination;}
	void setDestination(const Position& p);

protected:
	// We could've made this public and skip the functions, but that would
	// make the handling of aid/uid/text different from handling teleports,
	// which would be weird.
	Position destination;
};

class Door : public Item {
public:
	Door(const unsigned short _type);
	virtual ~Door();

	virtual Item* deepCopy() const;

	uint8_t getDoorID() const;
	void setDoorID(uint8_t id);

	virtual void serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	virtual bool readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attr, BinaryNode* node);
	//virtual void serializeItemAttributes_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	//virtual bool readItemAttribute_OTMM(const IOMap& maphandle, OTMM_ItemAttribute attr, BinaryNode* node);

protected:
	uint8_t doorid;
};

class Depot : public Item {
public:
	Depot(const unsigned short _type);
	virtual ~Depot();

	virtual Item* deepCopy() const;

	uint8_t getDepotID() const;
	void setDepotID(uint8_t id);

	virtual void serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	virtual bool readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attr, BinaryNode* node);
	//virtual void serializeItemAttributes_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	//virtual bool readItemAttribute_OTMM(const IOMap& maphandle, OTMM_ItemAttribute attr, BinaryNode* node);

protected:
	uint8_t depotid;
};

inline int Teleport::getX() const {
	return destination.x;
}

inline int Teleport::getY() const {
	return destination.y;
}

inline int Teleport::getZ() const {
	return destination.z;
}

inline void Teleport::setDestination(const Position& p) {
	destination = p;
}

inline uint8_t Door::getDoorID() const {
	return doorid;
}

inline void Door::setDoorID(uint8_t id) {
	doorid = id;
}

inline uint8_t Depot::getDepotID() const {
	return depotid;
}

inline void Depot::setDepotID(uint8_t id) {
	depotid = id;
}

#endif
