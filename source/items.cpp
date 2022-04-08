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

#include "materials.h"
#include "gui.h"
#include <string.h> // memcpy
#include <spdlog/spdlog.h>

#include "items.h"
#include "item.h"
#include "pugicast.h"
#include "sprite_appearances.h"

ItemDatabase g_items;

ItemType::ItemType() :
	sprite(nullptr),
	id(0),
	brush(nullptr),
	doodad_brush(nullptr),
	raw_brush(nullptr),
	is_metaitem(false),
	has_raw(false),
	in_other_tileset(false),
	group(ITEM_GROUP_NONE),
	type(ITEM_TYPE_NONE),
	volume(0),
	maxTextLen(0),
	//writeOnceItemID(0),
	ground_equivalent(0),
	border_group(0),
	has_equivalent(false),
	wall_hate_me(false),
	name(""),
	description(""),
	weight(0.0f),
	attack(0),
	defense(0),
	armor(0),
	charges(0),
	client_chargeable(false),
	extra_chargeable(false),
	ignoreLook(false),

	isHangable(false),
	hookEast(false),
	hookSouth(false),
	canReadText(false),
	canWriteText(false),
	replaceable(true),
	decays(false),
	stackable(false),
	moveable(true),
	alwaysOnBottom(false),
	pickupable(false),
	rotable(false),
	isBorder(false),
	isOptionalBorder(false),
	isWall(false),
	isBrushDoor(false),
	isOpen(false),
	isTable(false),
	isCarpet(false),

	floorChangeDown(false),
	floorChangeNorth(false),
	floorChangeSouth(false),
	floorChangeEast(false),
	floorChangeWest(false),
	floorChange(false),

	blockSolid(false),
	blockPickupable(false),
	blockProjectile(false),
	blockPathFind(false),
	hasElevation(false),

	alwaysOnTopOrder(0),
	rotateTo(0),
	border_alignment(BORDER_NONE)
{
	////
}

ItemType::~ItemType()
{
	////
}

bool ItemType::isFloorChange() const
{
	return floorChange || floorChangeDown || floorChangeNorth || floorChangeSouth || floorChangeEast || floorChangeWest;
}

ItemDatabase::ItemDatabase() :
	// Version information
	MajorVersion(0),
	MinorVersion(0),
	BuildNumber(0),

	// Count of GameSprite types
	item_count(0),
	effect_count(0),
	monster_count(0),
	distance_count(0),

	max_item_id(0)
{
	////
}

ItemDatabase::~ItemDatabase()
{
	clear();
}

void ItemDatabase::clear()
{
	for(uint32_t i = 0; i < items.size(); i++) {
		delete items[i];
		items.set(i, nullptr);
	}
}

bool ItemDatabase::loadFromProtobuf(wxString &error, wxArrayString &warnings, remeres::protobuf::appearances::Appearances appearances)
{
	using namespace remeres::protobuf::appearances;

	for (uint32_t it = 0; it < static_cast<uint32_t>(appearances.object_size()); ++it) {
		Appearance object = appearances.object(it);

		// This scenario should never happen but on custom assets this can break the loader.
		if (!object.has_flags()) {
			spdlog::error("[ItemDatabase::loadFromProtobuf] - Item with id {} is invalid and was ignored.", object.id());
			wxLogError("[ItemDatabase::loadFromProtobuf] - Item with id %i is invalid and was ignored.", object.id());
			continue;
		}

		if (object.id() >= items.size()) {
			items.resize(object.id() + 1);
		}

		ItemType *t = newd ItemType();
		t->id = static_cast<uint16_t>(object.id());
		t->name = object.name();
		t->description = object.description();

		if (object.flags().container()) {
			t->type = ITEM_TYPE_CONTAINER;
			t->group = ITEM_GROUP_CONTAINER;
		} else if (object.flags().has_bank()) {
			t->group = ITEM_GROUP_GROUND;
		} else if (object.flags().liquidcontainer()) {
			t->group = ITEM_GROUP_FLUID;
		} else if (object.flags().liquidpool()) {
			t->group = ITEM_GROUP_SPLASH;
		}

		if (object.flags().clip()) {
			t->alwaysOnTopOrder = 1;
		} else if (object.flags().top()) {
			t->alwaysOnTopOrder = 3;
		} else if (object.flags().bottom()) {
			t->alwaysOnTopOrder = 2;
		}

		// now lets parse sprite data
		t->m_animationPhases.clear();

		for (const auto &framegroup : object.frame_group()) {
			const auto &frameGroupType = framegroup.fixed_frame_group();
			const auto &spriteInfo = framegroup.sprite_info();
			const auto &animation = spriteInfo.animation();
			const auto &sprites = spriteInfo.sprite_id();

			t->pattern_width = spriteInfo.pattern_width();
			t->pattern_height = spriteInfo.pattern_height();
			t->pattern_depth = spriteInfo.pattern_depth();
			t->layers = spriteInfo.layers();

			if (animation.sprite_phase().size() > 0) {
				const auto &spritesPhases = animation.sprite_phase();
				t->start_frame = animation.default_start_phase();
				t->loop_count = animation.loop_count();
				//animation.loop_type();
				//animation.random_start_phase();
				t->async_animation = !animation.synchronized();
				for (int k = 0; k < spritesPhases.size(); k++) {
					t->m_animationPhases.push_back(std::pair<int,int>(static_cast<int>(spritesPhases[k].duration_min()),
												static_cast<int>(spritesPhases[k].duration_max())));
				}
			}

			t->sprite_id = spriteInfo.sprite_id(0);

			t->m_sprites.clear();
			t->m_sprites.resize(sprites.size());
			for (int i = 0; i < sprites.size(); i++)
				t->m_sprites[i] = sprites[i];

			/*// animations
			if (spritesPhases.size() > 1)
			{
				auto animator = AnimatorPtr(new Animator);
				animator->unserializeAppearance(animation);

				if (frameGroupType == FrameGroupMoving)
					m_animator = animator;
				else if (frameGroupType == FrameGroupIdle || frameGroupType == FrameGroupInitial)
					m_idleAnimator = animator;
			}*/
		}

		if (object.flags().has_clothes()) {
			// t->slotPosition |= static_cast<SlotPositionBits>(1 << (object.flags().clothes().slot() - 1));
		}

		if (object.flags().has_market()) {
			t->type = static_cast<ItemTypes_t>(object.flags().market().category());
		}

		// t->upgradeClassification = object.flags().has_upgradeclassification() ? static_cast<uint8_t>(object.flags().upgradeclassification().upgrade_classification()) : 0;
		// t->lightLevel = object.flags().has_light() ? static_cast<uint8_t>(object.flags().light().brightness()) : 0;
		// t->lightColor = object.flags().has_light() ? static_cast<uint8_t>(object.flags().light().color()) : 0;

		// t->speed = object.flags().has_bank() ? static_cast<uint16_t>(object.flags().bank().waypoints()) : 0;
		// t->wareId = object.flags().has_market() ? static_cast<uint16_t>(object.flags().market().trade_as_object_id()) : 0;

		t->noMoveAnimation = object.flags().no_movement_animation();
		t->isCorpse = object.flags().corpse() || object.flags().player_corpse();
		t->forceUse = object.flags().forceuse();
		t->hasHeight = object.flags().has_height();
		t->blockSolid = object.flags().unpass();
		t->blockProjectile = object.flags().unsight();
		t->blockPathFind = object.flags().avoid();
		t->pickupable = object.flags().take();
		// t->rotatable = object.flags().rotate();
		// t->wrapContainer = object.flags().wrap() || object.flags().unwrap();
		// t->multiUse = object.flags().multiuse();
		t->moveable = object.flags().unmove() == false;
		t->canReadText = (object.flags().has_lenshelp() && object.flags().lenshelp().id() == 1112) || (object.flags().has_write() && object.flags().write().max_text_length() != 0) || (object.flags().has_write_once() && object.flags().write_once().max_text_length_once() != 0);
		t->canReadText = object.flags().has_write() || object.flags().has_write_once();
		t->isVertical = object.flags().has_hook() && object.flags().hook().south();
		t->isHorizontal = object.flags().has_hook() && object.flags().hook().east();
		t->isHangable = object.flags().hang();
		// t->lookThrough = object.flags().ignore_look();
		t->stackable = object.flags().cumulative();
		t->isPodium = object.flags().show_off_socket();

		g_gui.gfx.loadItemSpriteMetadata(t, error, warnings);
		t->sprite = static_cast<GameSprite*>(g_gui.gfx.getSprite(t->id));
		if (t->sprite) {
			t->sprite->minimap_color = object.flags().has_automap() ? static_cast<uint16_t>(object.flags().automap().color()) : 0;
			t->sprite->draw_height = object.flags().has_height() ? static_cast<uint16_t>(object.flags().height().elevation()) : 0;
			if (object.flags().has_shift()) {
				t->sprite->drawoffset_x = static_cast<uint16_t>(object.flags().shift().x());
				t->sprite->drawoffset_y = static_cast<uint16_t>(object.flags().shift().y());
			}
		}

		if (t) {
			if (items[t->id]) {
				wxLogWarning("appearances.dat: Duplicate items");
				delete items[t->id];
			}
			items.set(t->id, t);
		}
	}
	return true;
}

bool ItemDatabase::loadItemFromGameXml(pugi::xml_node itemNode, int id)
{
	ClientVersionID clientVersion = g_gui.GetCurrentVersionID();
	if(clientVersion < CLIENT_VERSION_980 && id > 20000 && id < 20100) {
		itemNode = itemNode.next_sibling();
		return true;
	} else if(id > 30000 && id < 30100) {
		itemNode = itemNode.next_sibling();
		return true;
	}

	ItemType& it = getItemType(id);

	it.name = itemNode.attribute("name").as_string();
	it.editorsuffix = itemNode.attribute("editorsuffix").as_string();

	pugi::xml_attribute attribute;
	for(pugi::xml_node itemAttributesNode = itemNode.first_child(); itemAttributesNode; itemAttributesNode = itemAttributesNode.next_sibling()) {
		if(!(attribute = itemAttributesNode.attribute("key"))) {
			continue;
		}

		std::string key = attribute.as_string();
		to_lower_str(key);
		if(key == "type") {
			if(!(attribute = itemAttributesNode.attribute("value"))) {
				continue;
			}

			std::string typeValue = attribute.as_string();
			to_lower_str(key);
			if(typeValue == "depot") {
				it.type = ITEM_TYPE_DEPOT;
			} else if(typeValue == "mailbox") {
				it.type = ITEM_TYPE_MAILBOX;
			} else if(typeValue == "trashholder") {
				it.type = ITEM_TYPE_TRASHHOLDER;
			} else if (typeValue == "container") {
				it.type = ITEM_TYPE_CONTAINER;
			} else if (typeValue == "door") {
				it.type = ITEM_TYPE_DOOR;
			} else if (typeValue == "magicfield") {
				it.group = ITEM_GROUP_MAGICFIELD;
				it.type = ITEM_TYPE_MAGICFIELD;
			} else if (typeValue == "teleport") {
				it.type = ITEM_TYPE_TELEPORT;
			} else if (typeValue == "bed") {
				it.type = ITEM_TYPE_BED;
			} else if (typeValue == "key") {
				it.type = ITEM_TYPE_KEY;
			}
		} else if(key == "name") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.name = attribute.as_string();
			}
		} else if(key == "description") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.description = attribute.as_string();
			}
		}else if(key == "runespellName") {
			/*if((attribute = itemAttributesNode.attribute("value"))) {
				it.runeSpellName = attribute.as_string();
			}*/
		} else if(key == "weight") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.weight = pugi::cast<int32_t>(attribute.value()) / 100.f;
			}
		} else if(key == "armor") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.armor = pugi::cast<int32_t>(attribute.value());
			}
		} else if(key == "defense") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.defense = pugi::cast<int32_t>(attribute.value());
			}
		} else if(key == "rotateto") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.rotateTo = pugi::cast<int32_t>(attribute.value());
			}
		} else if(key == "containersize") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.volume = pugi::cast<int32_t>(attribute.value());
			}
		} else if(key == "readable") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.canReadText = attribute.as_bool();
			}
		} else if(key == "writeable") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.canWriteText = it.canReadText = attribute.as_bool();
			}
		} else if(key == "decayto") {
			it.decays = true;
		} else if(key == "maxtextlen" || key == "maxtextlength") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.maxTextLen = pugi::cast<int32_t>(attribute.value());
				it.canReadText = it.maxTextLen > 0;
			}
		} else if(key == "writeonceitemid") {
			/*if((attribute = itemAttributesNode.attribute("value"))) {
				it.writeOnceItemId = pugi::cast<int32_t>(attribute.value());
			}*/
		} else if(key == "allowdistread") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.allowDistRead = attribute.as_bool();
			}
		} else if(key == "charges") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				it.charges = pugi::cast<int32_t>(attribute.value());
				it.extra_chargeable = true;
			}
		} else if(key == "floorchange") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				std::string value = attribute.as_string();
				if(value == "down") {
					it.floorChangeDown = true;
					it.floorChange = true;
				} else if (value == "north") {
					it.floorChangeNorth = true;
					it.floorChange = true;
				} else if (value == "south") {
					it.floorChangeSouth = true;
					it.floorChange = true;
				} else if (value == "west") {
					it.floorChangeWest = true;
					it.floorChange = true;
				} else if (value == "east") {
					it.floorChangeEast = true;
					it.floorChange = true;
				} else if(value == "northex")
					it.floorChange = true;
				else if(value == "southex")
					it.floorChange = true;
				else if(value == "westex")
					it.floorChange = true;
				else if(value == "eastex")
					it.floorChange = true;
				else if (value == "southalt")
					it.floorChange = true;
				else if (value == "eastalt")
					it.floorChange = true;
			}
		}
	}
	return true;
}

bool ItemDatabase::loadFromGameXml(const FileName& identifier, wxString& error, wxArrayString& warnings)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(identifier.GetFullPath().mb_str());
	if(!result) {
		error = "Could not load items.xml (Syntax error?)";
		return false;
	}

	pugi::xml_node node = doc.child("items");
	if(!node) {
		error = "items.xml, invalid root node.";
		return false;
	}

	for(pugi::xml_node itemNode = node.first_child(); itemNode; itemNode = itemNode.next_sibling()) {
		if(as_lower_str(itemNode.name()) != "item") {
			continue;
		}

		int32_t fromId = pugi::cast<int32_t>(itemNode.attribute("fromid").value());
		int32_t toId = pugi::cast<int32_t>(itemNode.attribute("toid").value());
		if(pugi::xml_attribute attribute = itemNode.attribute("id")) {
			fromId = toId = pugi::cast<int32_t>(attribute.value());
		}

		if(fromId == 0 || toId == 0) {
			error = "Could not read item id from item node, fromid "+ std::to_string(fromId) +", toid "+ std::to_string(toId) +".";
			return false;
		}

		for(int32_t id = fromId; id <= toId; ++id) {
			if(!loadItemFromGameXml(itemNode, id)) {
				return false;
			}
		}
	}
	return true;
}

bool ItemDatabase::loadMetaItem(pugi::xml_node node)
{
	if(pugi::xml_attribute attribute = node.attribute("id")) {
		int32_t id = pugi::cast<int32_t>(attribute.value());
		if(items[id]) {
			//std::cout << "Occupied ID " << id << " : " << items[id]->id << ":" << items[id]->name << std::endl;
			return false;
		}
		items.set(id, newd ItemType());
		items[id]->is_metaitem = true;
		items[id]->id = id;
	} else {
		return false;
	}
	return true;
}

ItemType& ItemDatabase::getItemType(int id)
{
	ItemType* it = items[id];
	if(it)
		return *it;
	else {
		static ItemType dummyItemType; // use this for invalid ids
		return dummyItemType;
	}
}

bool ItemDatabase::typeExists(int id) const
{
	ItemType* it = items[id];
	return it != nullptr;
}
