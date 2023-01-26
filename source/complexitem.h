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

#ifndef RME_CONTAINER_H_
#define RME_CONTAINER_H_

#include "position.h"
#include "item.h"

#pragma pack(1)

struct OTBM_TeleportDestination
{
	uint16_t x;
	uint16_t y;
	uint8_t z;
};

#pragma pack()

class Container : public Item
{
public:
	Container(const uint16_t type);
	~Container();

	Item* deepCopy() const override;
	Container* getContainer() override { return this; }

	Item* getItem(size_t index) const;

	ItemVector& getVector() noexcept { return contents; }
	size_t getItemCount() const noexcept { return contents.size(); }
	size_t getVolume() const noexcept { return getItemType().volume; }
	double getWeight() noexcept { return getItemType().weight; }

	virtual bool unserializeItemNode_OTBM(const IOMap& maphandle, BinaryNode* node);
	virtual bool serializeItemNode_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	//virtual bool unserializeItemNode_OTMM(const IOMap& maphandle, BinaryNode* node);
	//virtual bool serializeItemNode_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;

protected:
	ItemVector contents;
};

class Teleport : public Item
{
public:
	Teleport(const uint16_t type);

	Item* deepCopy() const override;
	Teleport* getTeleport() override { return this; }

	virtual void serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	virtual bool readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attr, BinaryNode* node);
	//virtual void serializeItemAttributes_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	//virtual bool readItemAttribute_OTMM(const IOMap& maphandle, OTMM_ItemAttribute attr, BinaryNode* node);

	const Position& getDestination() const noexcept { return destination; }
	int getX() const noexcept { return destination.x; }
	int getY() const noexcept { return destination.y; }
	int getZ() const noexcept { return destination.z; }
	void setDestination(const Position& position) noexcept { destination = position; }
	bool hasDestination() const noexcept { return destination.isValid(); }

protected:
	Position destination;
};

class Door : public Item
{
public:
	Door(const uint16_t type);

	Item* deepCopy() const override;
	Door* getDoor() override { return this; }

	uint8_t getDoorID() const { return doorId; }
	void setDoorID(uint8_t id) { doorId = id; }

	virtual void serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	virtual bool readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attr, BinaryNode* node);
	//virtual void serializeItemAttributes_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	//virtual bool readItemAttribute_OTMM(const IOMap& maphandle, OTMM_ItemAttribute attr, BinaryNode* node);

protected:
	uint8_t doorId;
};

class Depot : public Item
{
public:
	Depot(const uint16_t _type);

	Item* deepCopy() const override;
	Depot* getDepot() override { return this; }

	uint8_t getDepotID() const { return depotId; }
	void setDepotID(uint8_t id) { depotId = id; }

	virtual void serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	virtual bool readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attr, BinaryNode* node);
	//virtual void serializeItemAttributes_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
	//virtual bool readItemAttribute_OTMM(const IOMap& maphandle, OTMM_ItemAttribute attr, BinaryNode* node);

protected:
	uint8_t depotId;
};

#endif
