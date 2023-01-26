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

#ifndef RME_ITEM_H_
#define RME_ITEM_H_

#include "items.h"
#include "iomap_otbm.h"
//#include "iomap_otmm.h"
#include "item_attributes.h"

class Creature;
class Border;
class Tile;

enum ITEMPROPERTY {
	BLOCKSOLID,
	HASHEIGHT,
	BLOCKPROJECTILE,
	BLOCKPATHFIND,
	PROTECTIONZONE,
	HOOK_SOUTH,
	HOOK_EAST,
	MOVEABLE,
	BLOCKINGANDNOTMOVEABLE
};

enum SplashType
{
	LIQUID_NONE = 0,
	LIQUID_WATER = 1,
	LIQUID_BLOOD = 2,
	LIQUID_BEER = 3,
	LIQUID_SLIME = 4,
	LIQUID_LEMONADE = 5,
	LIQUID_MILK = 6,
	LIQUID_MANAFLUID = 7,
	LIQUID_INK = 8,
	LIQUID_WATER2 = 9,
	LIQUID_LIFEFLUID = 10,
	LIQUID_OIL = 11,
	LIQUID_SLIME2 = 12,
	LIQUID_URINE = 13,
	LIQUID_COCONUT_MILK = 14,
	LIQUID_WINE = 15,
	LIQUID_MUD = 19,
	LIQUID_FRUIT_JUICE = 21,
	LIQUID_LAVA = 26,
	LIQUID_RUM = 27,
	LIQUID_SWAMP = 28,
	LIQUID_TEA = 35,
	LIQUID_MEAD = 43,

	LIQUID_FIRST = LIQUID_WATER,
	LIQUID_LAST = LIQUID_MEAD
};

IMPLEMENT_INCREMENT_OP(SplashType)

class Item : public ItemAttributes
{
public:
	//Factory member to create item of right type based on type
	static Item* Create(uint16_t id, uint16_t subtype = 0xFFFF);
	static Item* Create(pugi::xml_node);
	static Item* Create_OTBM(const IOMap& maphandle, BinaryNode* stream);
	//static Item* Create_OTMM(const IOMap& maphandle, BinaryNode* stream);

protected:
	// Constructor for items
	Item(unsigned short _type, unsigned short _count);

public:
	virtual ~Item();

// Deep copy thingy
	virtual Item* deepCopy() const;

	// Get memory footprint size
	uint32_t memsize() const;
	/*
	virtual Container* getContainer() { return nullptr; }
	virtual const Container* getContainer() const { return nullptr; }
	virtual Teleport* getTeleport() { return nullptr; }
	virtual const Teleport* getTeleport() const { return nullptr; }
	virtual TrashHolder* getTrashHolder() { return nullptr; }
	virtual const TrashHolder* getTrashHolder() const { return nullptr; }
	virtual Mailbox* getMailbox() { return nullptr; }
	virtual const Mailbox* getMailbox() const { return nullptr; }
	virtual Door* getDoor() { return nullptr; }
	virtual const Door* getDoor() const { return nullptr; }
	virtual MagicField* getMagicField() { return nullptr; }
	virtual const MagicField* getMagicField() const { return nullptr; }
	*/

	// OTBM map interface
		// Serialize and unserialize (for save/load)
		// Used internally
		virtual bool readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attr, BinaryNode* stream);
		virtual bool unserializeAttributes_OTBM(const IOMap& maphandle, BinaryNode* stream);
		virtual bool unserializeItemNode_OTBM(const IOMap& maphandle, BinaryNode* node);

		// Will return a node containing this item
		virtual bool serializeItemNode_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
		// Will write this item to the stream supplied in the argument
		virtual void serializeItemCompact_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
		virtual void serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const;

	// OTMM map interface
		/*
		// Serialize and unserialize (for save/load)
		// Used internally
		virtual bool readItemAttribute_OTMM(const IOMap& maphandle, OTMM_ItemAttribute attr, BinaryNode* stream);
		virtual bool unserializeAttributes_OTMM(const IOMap& maphandle, BinaryNode* stream);
		virtual bool unserializeItemNode_OTMM(const IOMap& maphandle, BinaryNode* node);

		// Will return a node containing this item
		virtual bool serializeItemNode_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
		// Will write this item to the stream supplied in the argument
		virtual void serializeItemCompact_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
		virtual void serializeItemAttributes_OTMM(const IOMap& maphandle, NodeFileWriteHandle& f) const;
		*/

	// Static conversions
	static std::string LiquidID2Name(uint16_t id);
	static uint16_t LiquidName2ID(std::string id);

	const ItemType& getItemType() const noexcept { return g_items.getItemType(id); }

	// IDs
	uint16_t getID() const { return id; }
	uint16_t getClientID() const { return g_items.getItemType(id).clientID; }

	// NOTE: This is very volatile, do NOT use this unless you know exactly what you're doing
	// which you probably don't so avoid it like the plague!
	void setID(uint16_t new_id);

	bool isValidID() const { return g_items.isValidID(id); }

	// Usual attributes
	virtual double getWeight() const;
	int getAttack() const { return getItemType().attack; }
	int getArmor() const { return getItemType().armor; }
	int getDefense() const { return getItemType().defense; }
	//int getSlotPosition() const { return g_items.getItemType(id).slot_position; }

	// Item g_settings
	bool canHoldText() const;
	bool canHoldDescription() const;
	bool isReadable() const { return getItemType().canReadText; }
	bool canWriteText() const { return getItemType().canWriteText; }
	uint32_t getMaxWriteLength() const { return getItemType().maxTextLen; }
	Brush* getBrush() const { return getItemType().brush; }
	GroundBrush* getGroundBrush() const;
	WallBrush* getWallBrush() const;
	DoorBrush* getDoorBrush() const;
	TableBrush* getTableBrush() const;
	CarpetBrush* getCarpetBrush() const;
	Brush* getDoodadBrush() const { return getItemType().doodad_brush; } // This is not necessarily a doodad brush
	RAWBrush* getRAWBrush() const { return getItemType().raw_brush; }
	uint16_t getGroundEquivalent() const { return getItemType().ground_equivalent; }
	uint16_t hasBorderEquivalent() const { return getItemType().has_equivalent; }
	uint32_t getBorderGroup() const { return getItemType().border_group; }

	// Drawing related
	uint8_t getMiniMapColor() const;
	wxPoint getDrawOffset() const;

	// Item types
	bool hasProperty(enum ITEMPROPERTY prop) const;
	bool isBlocking() const { return getItemType().unpassable; }
	bool isStackable() const { return getItemType().stackable; }
	bool isClientCharged() const { return getItemType().isClientCharged(); }
	bool isExtraCharged() const { return getItemType().isExtraCharged(); }
	bool isCharged() const { return isClientCharged() || isExtraCharged(); }
	bool isFluidContainer() const { return (getItemType().isFluidContainer()); }
	bool isAlwaysOnBottom() const { return getItemType().alwaysOnBottom; }
	int  getTopOrder() const { return getItemType().alwaysOnTopOrder; }
	bool isGroundTile() const { return getItemType().isGroundTile(); }
	bool isSplash() const { return getItemType().isSplash(); }
	bool isMagicField() const { return getItemType().isMagicField(); }
	bool isNotMoveable() const { return !getItemType().moveable; }
	bool isMoveable() const { return getItemType().moveable; }
	bool isPickupable() const { return getItemType().pickupable; }
	//bool isWeapon() const { return (getItemType().weaponType != WEAPON_NONE && g_items[id].weaponType != WEAPON_AMMO); }
	//bool isUseable() const { return getItemType().useable; }
	bool isHangable() const { return getItemType().isHangable; }
	bool isRoteable() const { return getItemType().rotable && getItemType().rotateTo; }
	bool hasCharges() const { return getItemType().charges != 0; }
	bool isBorder() const { return getItemType().isBorder; }
	bool isOptionalBorder() const { return getItemType().isOptionalBorder; }
	bool isWall() const { return getItemType().isWall; }
	bool isDoor() const { return getItemType().isDoor(); }
	bool isOpen() const { return getItemType().isOpen; }
	bool isBrushDoor() const { return getItemType().isBrushDoor; }
	bool isTable() const { return getItemType().isTable; }
	bool isCarpet() const { return getItemType().isCarpet; }
	bool isMetaItem() const { return getItemType().isMetaItem(); }

	// Wall alignment (vertical, horizontal, pole, corner)
	BorderType getWallAlignment() const;
	// Border aligment (south, west etc.)
	BorderType getBorderAlignment() const;

	// Get the name!
	const std::string getName() const { return getItemType().name; }
	const std::string getFullName() const { return getItemType().name + getItemType().editorsuffix; }

	// Selection
	bool isSelected() const { return selected; }
	void select() {selected = true; }
	void deselect() {selected = false; }
	void toggleSelection() {selected =! selected; }

	// Item properties!
	virtual bool isComplex() const { return attributes && attributes->size(); } // If this item requires full save (not compact)

	// Weight
	bool hasWeight() { return isPickupable(); }
	virtual double getWeight();

	// Subtype (count, fluid, charges)
	int getCount() const;
	uint16_t getSubtype() const;
	void setSubtype(uint16_t subtype);
	bool hasSubtype() const;

	void setUniqueID(uint16_t n);
	uint16_t getUniqueID() const;

	void setActionID(uint16_t n);
	uint16_t getActionID() const;

	void setText(const std::string& str);
	std::string getText() const;

	void setDescription(const std::string& str);
	std::string getDescription() const;

	void animate();
	int getFrame() const { return frame; }

	void doRotate() {
		if(isRoteable()) {
			setID(getItemType().rotateTo);
		}
	}

protected:
	uint16_t id;  // the same id as in ItemType
	// Subtype is either fluid type, count, subtype or charges
	uint16_t subtype;
	bool selected;
	int frame;

private:
	Item& operator=(const Item& i);// Can't copy
	Item(const Item &i); // Can't copy-construct
	Item& operator==(const Item& i);// Can't compare
};

typedef std::vector<Item*> ItemVector;
typedef std::list<Item*> ItemList;

Item* transformItem(Item* old_item, uint16_t new_id, Tile* parent = nullptr);

inline int Item::getCount() const {
	if(isStackable() || isExtraCharged() || isClientCharged()) {
		return subtype;
	}
	return 1;
}

inline uint16_t Item::getUniqueID() const {
	const int32_t* a = getIntegerAttribute("uid");
	if(a)
		return *a;
	return 0;
}

inline uint16_t Item::getActionID() const {
	const int32_t* a = getIntegerAttribute("aid");
	if(a)
		return *a;
	return 0;
}

inline std::string Item::getText() const {
	const std::string* a = getStringAttribute("text");
	if(a)
		return *a;
	return "";
}

inline std::string Item::getDescription() const {
	const std::string* a = getStringAttribute("desc");
	if(a)
		return *a;
	return "";
}


#endif
