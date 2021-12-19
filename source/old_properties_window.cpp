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

#include <wx/grid.h>

#include "tile.h"
#include "item.h"
#include "complexitem.h"
#include "town.h"
#include "house.h"
#include "map.h"
#include "editor.h"
#include "creature.h"

#include "gui.h"
#include "application.h"
#include "old_properties_window.h"
#include "container_properties_window.h"

// ============================================================================
// Old Properties Window

BEGIN_EVENT_TABLE(OldPropertiesWindow, wxDialog)
EVT_SET_FOCUS(OldPropertiesWindow::OnFocusChange)
EVT_BUTTON(wxID_OK, OldPropertiesWindow::OnClickOK)
EVT_BUTTON(wxID_CANCEL, OldPropertiesWindow::OnClickCancel)
END_EVENT_TABLE()

static constexpr int OUTFIT_COLOR_MAX = 133;

OldPropertiesWindow::OldPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Item Properties", map, tile_parent, item, pos),
	count_field(nullptr),
	direction_field(nullptr),
	action_id_field(nullptr),
	unique_id_field(nullptr),
	door_id_field(nullptr),
	tier_field(nullptr),
	depot_id_field(nullptr),
	splash_type_field(nullptr),
	text_field(nullptr),
	description_field(nullptr)
{
	ASSERT(edit_item);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	if(Container* container = dynamic_cast<Container*>(edit_item)) {
		// Container
		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Container Properties");

		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Action ID"));
		action_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
		subsizer->Add(action_id_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Unique ID"));
		unique_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
		subsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

		boxsizer->Add(subsizer, wxSizerFlags(0).Expand());

		// Now we add the subitems!
		wxSizer* contents_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Contents");

		bool use_large_sprites = g_settings.getBoolean(Config::USE_LARGE_CONTAINER_ICONS);
		wxSizer* horizontal_sizer = nullptr;
		const int additional_height_increment = (use_large_sprites? 40 : 24);
		int additional_height = 0;

		int32_t maxColumns;
		if(use_large_sprites) {
			maxColumns = 6;
		} else {
			maxColumns = 12;
		}

		for(uint32_t index = 0; index < container->getVolume(); ++index) {
			if(!horizontal_sizer) {
				horizontal_sizer = newd wxBoxSizer(wxHORIZONTAL);
			}

			Item* item = container->getItem(index);
			ContainerItemButton* containerItemButton = newd ContainerItemButton(this, use_large_sprites, index, map, item);

			container_items.push_back(containerItemButton);
			horizontal_sizer->Add(containerItemButton);

			if(((index + 1) % maxColumns) == 0) {
				contents_sizer->Add(horizontal_sizer);
				horizontal_sizer = nullptr;
				additional_height += additional_height_increment;
			}
		}

		if(horizontal_sizer != nullptr) {
			contents_sizer->Add(horizontal_sizer);
			additional_height += additional_height_increment;
		}

		boxsizer->Add(contents_sizer, wxSizerFlags(2).Expand());

		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));

		//SetSize(260, 150 + additional_height);
	} else if(edit_item->canHoldText() || edit_item->canHoldDescription()) {
		// Book
		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Writeable Properties");

		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Action ID"));
		action_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
		action_id_field->SetSelection(-1, -1);
		subsizer->Add(action_id_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Unique ID"));
		unique_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
		subsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

		boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

		wxSizer* textsizer = newd wxBoxSizer(wxVERTICAL);
		textsizer->Add(newd wxStaticText(this, wxID_ANY, "Text"), wxSizerFlags(1).Center());
		text_field = newd wxTextCtrl(this, wxID_ANY, wxstr(item->getText()), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
		textsizer->Add(text_field, wxSizerFlags(7).Expand());

		boxsizer->Add(textsizer, wxSizerFlags(2).Expand());

		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));

		//SetSize(220, 310);
	} else if (edit_item->isSplash() || edit_item->isFluidContainer()) {
		// Splash
		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Splash Properties");

		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Type"));

		// Splash types
		splash_type_field = newd wxChoice(this, wxID_ANY);
		if (edit_item->isFluidContainer()) {
			splash_type_field->Append(wxstr(Item::LiquidID2Name(LIQUID_NONE)), newd int32_t(LIQUID_NONE));
		}

		for (SplashType splashType = LIQUID_FIRST; splashType != LIQUID_LAST; ++splashType) {
			splash_type_field->Append(wxstr(Item::LiquidID2Name(splashType)), newd int32_t(splashType));
		}

		if (item->getSubtype()) {
			const std::string& what = Item::LiquidID2Name(item->getSubtype());
			if (what == "Unknown") {
				splash_type_field->Append(wxstr(Item::LiquidID2Name(LIQUID_NONE)), newd int32_t(LIQUID_NONE));
			}
			splash_type_field->SetStringSelection(wxstr(what));
		} else {
			splash_type_field->SetSelection(0);
		}

		subsizer->Add(splash_type_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Action ID"));
		action_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
		subsizer->Add(action_id_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Unique ID"));
		unique_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
		subsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

		boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));

		//SetSize(220, 190);
	} else if(Depot* depot = dynamic_cast<Depot*>(edit_item)) {
		// Depot
		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Depot Properties");
		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);

		subsizer->AddGrowableCol(1);
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

		const Towns& towns = map->towns;
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Depot ID"));
		depot_id_field = newd wxChoice(this, wxID_ANY);
		int to_select_index = 0;
		if(towns.count() > 0) {
			bool found = false;
			for(TownMap::const_iterator town_iter = towns.begin();
					town_iter != towns.end();
					++town_iter)
			{
				if(town_iter->second->getID() == depot->getDepotID()) {
					found = true;
				}
				depot_id_field->Append(wxstr(town_iter->second->getName()), newd int(town_iter->second->getID()));
				if(!found) ++to_select_index;
			}
			if(!found) {
				if(depot->getDepotID() != 0) {
					depot_id_field->Append("Undefined Town (id:" + i2ws(depot->getDepotID()) + ")", newd int(depot->getDepotID()));
				}
			}
		}
		depot_id_field->Append("No Town", newd int(0));
		if(depot->getDepotID() == 0) {
			to_select_index = depot_id_field->GetCount() - 1;
		}
		depot_id_field->SetSelection(to_select_index);

		subsizer->Add(depot_id_field, wxSizerFlags(1).Expand());

		boxsizer->Add(subsizer, wxSizerFlags(5).Expand());
		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxALL, 20));
		//SetSize(220, 140);
	} else {
		// Normal item
		Door* door = dynamic_cast<Door*>(edit_item);
		Teleport* teleport = dynamic_cast<Teleport*>(edit_item);
		Podium* podium = dynamic_cast<Podium*>(edit_item);

		wxString description;
		if(door) {
			ASSERT(tile_parent);
			description = "Door Properties";
		} else if(teleport) {
			description = "Teleport Properties";
		} else if(podium) {
			description = "Podium Properties";
		} else {
			description = "Item Properties";
		}

		wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, description);

		// unused(?)
		/*
		int num_items = 4;
		//if(item->canHoldDescription()) num_items += 1;
		if(door) num_items += 1;
		if(teleport) num_items += 1;
		*/

		wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
		subsizer->AddGrowableCol(1);

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "ID " + i2ws(item->getID())));
		subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(item->getName()) + "\""));

		subsizer->Add(newd wxStaticText(this, wxID_ANY, (item->isCharged()? "Charges" : "Count")));
		int max_count = 100;
		if(item->isClientCharged()) max_count = 250;
		if(item->isExtraCharged()) max_count = 65500;
		count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getCount()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, max_count, edit_item->getCount());
		if(!item->isStackable() && !item->isCharged()) {
			count_field->Enable(false);
		}
		subsizer->Add(count_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Action ID"));
		action_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
		subsizer->Add(action_id_field, wxSizerFlags(1).Expand());

		subsizer->Add(newd wxStaticText(this, wxID_ANY, "Unique ID"));
		unique_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
		subsizer->Add(unique_id_field, wxSizerFlags(1).Expand());


		// item classification (12.81+)
		if (g_items.MajorVersion >= 3 && g_items.MinorVersion >= 60 && (edit_item->getClassification() > 0 || edit_item->isWeapon() || edit_item->isWearableEquipment())) {
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Classification"));
			subsizer->Add(newd wxStaticText(this, wxID_ANY, i2ws(item->getClassification())));

			// item iter
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Tier"));
			tier_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_item->getTier()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFF, edit_item->getTier());
			subsizer->Add(tier_field, wxSizerFlags(1).Expand());
		}

		/*
		if(item->canHoldDescription()) {
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Description"));
			description_field = newd wxTextCtrl(this, wxID_ANY, edit_item->getText(), wxDefaultPosition, wxSize(-1, 20));
			subsizer->Add(description_field, wxSizerFlags(1).Expand());
		}
		*/

		if(door) {
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Door ID"));
			door_id_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(door->getDoorID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFF, door->getDoorID());
			if(!edit_tile || !edit_tile->isHouseTile()) {
				door_id_field->Disable();
			}
			subsizer->Add(door_id_field, wxSizerFlags(1).Expand());
		}

		if(teleport) {
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Destination"));

			wxSizer* possizer = newd wxBoxSizer(wxHORIZONTAL);
			x_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getX()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, map->getWidth(), teleport->getX());
			possizer->Add(x_field, wxSizerFlags(3).Expand());
			y_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getY()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, map->getHeight(), teleport->getY());
			possizer->Add(y_field, wxSizerFlags(3).Expand());
			z_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(teleport->getZ()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, MAP_MAX_LAYER, teleport->getZ());
			possizer->Add(z_field, wxSizerFlags(2).Expand());

			subsizer->Add(possizer, wxSizerFlags(1).Expand());
		}

		if (podium) {
			// direction
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "Direction"));
			direction_field = newd wxChoice(this, wxID_ANY);

			for (Direction dir = DIRECTION_FIRST; dir <= DIRECTION_LAST; ++dir) {
				direction_field->Append(wxstr(Creature::DirID2Name(dir)), newd int32_t(dir));
			}
			direction_field->SetSelection(static_cast<Direction>(podium->getDirection()));
			subsizer->Add(direction_field, wxSizerFlags(1).Expand());

			// checkboxes
			show_outfit = newd wxCheckBox(this, wxID_ANY, "Show outfit");
			show_outfit->SetValue(podium->hasShowOutfit());
			show_outfit->SetToolTip("Display outfit on the podium.");
			subsizer->Add(show_outfit, 0, wxLEFT | wxTOP, 5);
			subsizer->Add(newd wxStaticText(this, wxID_ANY, "")); // filler for checkboxes

			show_mount = newd wxCheckBox(this, wxID_ANY, "Show mount");
			show_mount->SetValue(podium->hasShowMount());
			show_mount->SetToolTip("Display mount on the podium.");
			subsizer->Add(show_mount, 0, wxLEFT | wxTOP, 5);
			subsizer->Add(newd wxStaticText(this, wxID_ANY, ""));

			show_platform = newd wxCheckBox(this, wxID_ANY, "Show platform");
			show_platform->SetValue(podium->hasShowPlatform());
			show_platform->SetToolTip("Display the podium platform.");
			subsizer->Add(show_platform, 0, wxLEFT | wxTOP, 5);
			subsizer->Add(newd wxStaticText(this, wxID_ANY, ""));

			// outfit container
			wxFlexGridSizer* outfitContainer = newd wxFlexGridSizer(2, 10, 10);
			const Outfit& outfit = podium->getOutfit();

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Outfit"));
			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, ""));

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "LookType"));
			look_type = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookType), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, std::numeric_limits<uint16_t>::max(), outfit.lookType);
			outfitContainer->Add(look_type, wxSizerFlags(3).Expand());

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Head"));
			look_head = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookHead), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookHead);
			outfitContainer->Add(look_head, wxSizerFlags(3).Expand());

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Body"));
			look_body = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookBody), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookBody);
			outfitContainer->Add(look_body, wxSizerFlags(3).Expand());

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Legs"));
			look_legs = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookLegs), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookLegs);
			outfitContainer->Add(look_legs , wxSizerFlags(3).Expand());

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Feet"));
			look_feet = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookFeet), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookFeet);
			outfitContainer->Add(look_feet, wxSizerFlags(3).Expand());

			outfitContainer->Add(newd wxStaticText(this, wxID_ANY, "Addons"));
			look_addon = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookAddon), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 3, outfit.lookAddon);
			outfitContainer->Add(look_addon, wxSizerFlags(3).Expand());

			// mount container
			wxFlexGridSizer* mountContainer = newd wxFlexGridSizer(2, 10, 10);

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Mount"));
			mountContainer->Add(newd wxStaticText(this, wxID_ANY, ""));

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "LookMount"));
			look_mount = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMount), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, std::numeric_limits<uint16_t>::max(), outfit.lookMount);
			mountContainer->Add(look_mount, wxSizerFlags(3).Expand());

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Head"));
			look_mounthead = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountHead), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountHead);
			mountContainer->Add(look_mounthead, wxSizerFlags(3).Expand());

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Body"));
			look_mountbody = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountBody), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountBody);
			mountContainer->Add(look_mountbody, wxSizerFlags(3).Expand());

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Legs"));
			look_mountlegs = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountLegs), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountLegs);
			mountContainer->Add(look_mountlegs, wxSizerFlags(3).Expand());

			mountContainer->Add(newd wxStaticText(this, wxID_ANY, "Feet"));
			look_mountfeet = newd wxSpinCtrl(this, wxID_ANY, i2ws(outfit.lookMountFeet), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, OUTFIT_COLOR_MAX, outfit.lookMountFeet);
			mountContainer->Add(look_mountfeet, wxSizerFlags(3).Expand());

			wxFlexGridSizer* propertiesContainer = newd wxFlexGridSizer(3, 10, 10);
			propertiesContainer->Add(subsizer, wxSizerFlags(1).Expand());
			propertiesContainer->Add(outfitContainer, wxSizerFlags(1).Expand());
			propertiesContainer->Add(mountContainer, wxSizerFlags(1).Expand());
			boxsizer->Add(propertiesContainer, wxSizerFlags(1).Expand());
		} else {
			boxsizer->Add(subsizer, wxSizerFlags(1).Expand());
		}

		topsizer->Add(boxsizer, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT, 20));
	}

	// Others attributes
	// should have an option to turn it off
	/*
	const ItemType& type = g_items.getItemType(edit_item->getID());
	wxStaticBoxSizer* others_sizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Others");
	wxFlexGridSizer* others_subsizer = newd wxFlexGridSizer(2, 5, 10);
	others_subsizer->AddGrowableCol(1);
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Stackable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.stackable)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Movable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.moveable)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Pickupable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.pickupable)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Hangable"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.isHangable)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Block Missiles"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.blockMissiles)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Block Pathfinder"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.blockPathfinder)));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, "Has Elevation"));
	others_subsizer->Add(newd wxStaticText(this, wxID_ANY, b2yn(type.hasElevation)));
	others_sizer->Add(others_subsizer, wxSizerFlags(1).Expand());
	topsizer->Add(others_sizer, wxSizerFlags(0).Expand().Border(wxLEFT | wxRIGHT | wxBOTTOM, 20));
	*/

	wxSizer* subsizer = newd wxBoxSizer(wxHORIZONTAL);
	subsizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	subsizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(subsizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

OldPropertiesWindow::OldPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Creature* creature, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Creature Properties", map, tile_parent, creature, pos),
	count_field(nullptr),
	direction_field(nullptr),
	action_id_field(nullptr),
	unique_id_field(nullptr),
	door_id_field(nullptr),
	tier_field(nullptr),
	depot_id_field(nullptr),
	splash_type_field(nullptr),
	text_field(nullptr),
	description_field(nullptr)
{
	ASSERT(edit_creature);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Creature Properties");

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Creature "));
	subsizer->Add(newd wxStaticText(this, wxID_ANY, "\"" + wxstr(edit_creature->getName()) + "\""), wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Spawn interval"));
	count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_creature->getSpawnTime()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 10, 3600, edit_creature->getSpawnTime());
	// count_field->SetSelection(-1, -1);
	subsizer->Add(count_field, wxSizerFlags(1).Expand());

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Direction"));
	direction_field = newd wxChoice(this, wxID_ANY);

	for(Direction dir = DIRECTION_FIRST; dir <= DIRECTION_LAST; ++dir) {
		direction_field->Append(wxstr(Creature::DirID2Name(dir)), newd int32_t(dir));
	}
	direction_field->SetSelection(edit_creature->getDirection());
	subsizer->Add(direction_field, wxSizerFlags(1).Expand());

	boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

	topsizer->Add(boxsizer, wxSizerFlags(3).Expand().Border(wxALL, 20));
	//SetSize(220, 0);

	wxSizer* std_sizer = newd wxBoxSizer(wxHORIZONTAL);
	std_sizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	std_sizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(std_sizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

OldPropertiesWindow::OldPropertiesWindow(wxWindow* win_parent, const Map* map, const Tile* tile_parent, Spawn* spawn, wxPoint pos) :
	ObjectPropertiesWindowBase(win_parent, "Spawn Properties", map, tile_parent, spawn, pos),
	count_field(nullptr),
	direction_field(nullptr),
	action_id_field(nullptr),
	unique_id_field(nullptr),
	door_id_field(nullptr),
	tier_field(nullptr),
	depot_id_field(nullptr),
	splash_type_field(nullptr),
	text_field(nullptr),
	description_field(nullptr)
{
	ASSERT(edit_spawn);

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* boxsizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Spawn Properties");

	//if(item->canHoldDescription()) num_items += 1;

	wxFlexGridSizer* subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	subsizer->Add(newd wxStaticText(this, wxID_ANY, "Spawn size"));
	count_field = newd wxSpinCtrl(this, wxID_ANY, i2ws(edit_spawn->getSize()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, g_settings.getInteger(Config::MAX_SPAWN_RADIUS), edit_spawn->getSize());
	// count_field->SetSelection(-1, -1);
	subsizer->Add(count_field, wxSizerFlags(1).Expand());

	boxsizer->Add(subsizer, wxSizerFlags(1).Expand());

	topsizer->Add(boxsizer, wxSizerFlags(3).Expand().Border(wxALL, 20));

	wxSizer* std_sizer = newd wxBoxSizer(wxHORIZONTAL);
	std_sizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	std_sizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center().Border(wxTOP | wxBOTTOM, 10));
	topsizer->Add(std_sizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT, 20));

	SetSizerAndFit(topsizer);
	Centre(wxBOTH);
}

OldPropertiesWindow::~OldPropertiesWindow()
{
	// Warning: edit_item may no longer be valid, DONT USE IT!
	if(splash_type_field) {
		for(uint32_t i = 0; i < splash_type_field->GetCount(); ++i) {
			delete reinterpret_cast<int*>(splash_type_field->GetClientData(i));
		}
	}
	if(direction_field) {
		for(uint32_t i = 0; i < direction_field->GetCount(); ++i) {
			delete reinterpret_cast<int*>(direction_field->GetClientData(i));
		}
	}
	if(depot_id_field) {
		for(uint32_t i = 0; i < depot_id_field->GetCount(); ++i) {
			delete reinterpret_cast<int*>(depot_id_field->GetClientData(i));
		}
	}
}

void OldPropertiesWindow::OnFocusChange(wxFocusEvent& event)
{
	wxWindow* win = event.GetWindow();
	if(wxSpinCtrl* spin = dynamic_cast<wxSpinCtrl*>(win))
		spin->SetSelection(-1, -1);
	else if(wxTextCtrl* text = dynamic_cast<wxTextCtrl*>(win))
		text->SetSelection(-1, -1);
}

void OldPropertiesWindow::OnClickOK(wxCommandEvent& WXUNUSED(event))
{
	if(edit_item) {
		if(dynamic_cast<Container*>(edit_item)) {
			// Container
			int new_uid = unique_id_field->GetValue();
			int new_aid = action_id_field->GetValue();

			if((new_uid < 1000 || new_uid > 0xFFFF) && new_uid != 0) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be between 1000 and 65535.", wxOK);
				return;
			}
			if(/* there is no item with the same UID */false) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be unique, this UID is already taken.", wxOK);
				return;
			}
			if((new_aid < 100 || new_aid > 0xFFFF) && new_aid != 0) {
				g_gui.PopupDialog(this, "Error", "Action ID must be between 100 and 65535.", wxOK);
				return;
			}

			edit_item->setUniqueID(new_uid);
			edit_item->setActionID(new_aid);
		} else if(edit_item->canHoldText() || edit_item->canHoldDescription()) {
			// Book
			int new_uid = unique_id_field->GetValue();
			int new_aid = action_id_field->GetValue();
			std::string text = nstr(text_field->GetValue());

			if((new_uid < 1000 || new_uid > 0xFFFF) && new_uid != 0) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be between 1000 and 65535.", wxOK);
				return;
			}
			if(/* there is no item with the same UID */false) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be unique, this UID is already taken.", wxOK);
				return;
			}
			if((new_aid < 100 || new_aid > 0xFFFF) && new_aid != 0) {
				g_gui.PopupDialog(this, "Error", "Action ID must be between 100 and 65535.", wxOK);
				return;
			}
			if(text.length() >= 0xFFFF) {
				g_gui.PopupDialog(this, "Error", "Text is longer than 65535 characters, this is not supported by OpenTibia. Reduce the length of the text.", wxOK);
				return;
			}
			if(edit_item->canHoldText() && text.length() > edit_item->getMaxWriteLength()) {
				int ret = g_gui.PopupDialog(this, "Error", "Text is longer than the maximum supported length of this book type, do you still want to change it?", wxYES | wxNO);
				if(ret != wxID_YES) {
					return;
				}
			}

			edit_item->setUniqueID(new_uid);
			edit_item->setActionID(new_aid);
			edit_item->setText(text);
		} else if(edit_item->isSplash() || edit_item->isFluidContainer()) {
			// Splash
			int new_uid = unique_id_field->GetValue();
			int new_aid = action_id_field->GetValue();
			int* new_type = reinterpret_cast<int*>(splash_type_field->GetClientData(splash_type_field->GetSelection()));

			if((new_uid < 1000 || new_uid > 0xFFFF) && new_uid != 0) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be between 1000 and 65535.", wxOK);
				return;
			}
			if(/* there is no item with the same UID */false) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be unique, this UID is already taken.", wxOK);
				return;
			}
			if((new_aid < 100 || new_aid > 0xFFFF) && new_aid != 0) {
				g_gui.PopupDialog(this, "Error", "Action ID must be between 100 and 65535.", wxOK);
				return;
			}
			if(new_type) {
				edit_item->setSubtype(*new_type);
			}
			edit_item->setUniqueID(new_uid);
			edit_item->setActionID(new_aid);

			// Clean up client data
		} else if(Depot* depot = dynamic_cast<Depot*>(edit_item)) {
			// Depot
			int* new_depotid = reinterpret_cast<int*>(depot_id_field->GetClientData(depot_id_field->GetSelection()));

			depot->setDepotID(*new_depotid);
		} else {
			// Normal item
			Door* door = dynamic_cast<Door*>(edit_item);
			Teleport* teleport = dynamic_cast<Teleport*>(edit_item);
			Podium* podium = dynamic_cast<Podium*>(edit_item);

			int new_uid = unique_id_field->GetValue();
			int new_aid = action_id_field->GetValue();
			int new_count = count_field ? count_field->GetValue() : 1;
			int new_tier = tier_field ? tier_field->GetValue() : 0;

			std::string new_desc;
			if(edit_item->canHoldDescription() && description_field) {
				description_field->GetValue();
			}
			Position new_dest;
			if(teleport) {
				new_dest = Position(x_field->GetValue(), y_field->GetValue(), z_field->GetValue());
			}
			uint8_t new_door_id = 0;
			if(door) {
				new_door_id = door_id_field->GetValue();
			}

			if((new_uid < 1000 || new_uid > 0xFFFF) && new_uid != 0) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be between 1000 and 65535.", wxOK);
				return;
			}
			if(/* there is no item with the same UID */false) {
				g_gui.PopupDialog(this, "Error", "Unique ID must be unique, this UID is already taken.", wxOK);
				return;
			}
			if((new_aid < 100 || new_aid > 0xFFFF) && new_aid != 0) {
				g_gui.PopupDialog(this, "Error", "Action ID must be between 100 and 65535.", wxOK);
				return;
			}
			if (new_tier < 0 || new_tier > 0xFF) {
				g_gui.PopupDialog(this, "Error", "Item tier must be between 0 and 255.", wxOK);
				return;
			}

			/*
			if(edit_item->canHoldDescription()) {
				if(new_desc.length() > 127) {
					g_gui.PopupDialog("Error", "Description must be shorter than 127 characters.", wxOK);
					return;
				}
			}
			*/

			if(door && g_settings.getInteger(Config::WARN_FOR_DUPLICATE_ID)) {
				if(edit_tile && edit_tile->isHouseTile()) {
					const House* house = edit_map->houses.getHouse(edit_tile->getHouseID());
					if(house) {
						Position pos = house->getDoorPositionByID(new_door_id);
						if(pos == Position()) {
							// Do nothing
						} else if(pos != edit_tile->getPosition()) {
							int ret = g_gui.PopupDialog(this, "Warning", "This doorid conflicts with another one in this house, are you sure you want to continue?", wxYES | wxNO);
							if(ret == wxID_NO) {
								return;
							}
						}
					}
				}
			}

			if(teleport) {
				if(edit_map->getTile(new_dest) == nullptr || edit_map->getTile(new_dest)->isBlocking()) {
					int ret = g_gui.PopupDialog(this, "Warning", "This teleport leads nowhere, or to an invalid location. Do you want to change the destination?", wxYES | wxNO);
					if(ret == wxID_YES) {
						return;
					}
				}
			}

			Outfit& newOutfit = Outfit();
			if (podium) {
				int newLookType = look_type->GetValue();
				int newMount = look_mount->GetValue();

				if (newLookType < 0 || newLookType > 0xFFFF || newMount < 0 || newMount > 0xFFFF) {
					g_gui.PopupDialog(this, "Error", "LookType and Mount must be between 0 and 65535.", wxOK);
					return;
				}

				int newHead = look_head->GetValue();
				int newBody = look_body->GetValue();
				int newLegs = look_legs->GetValue();
				int newFeet = look_feet->GetValue();
				int newMountHead = look_mounthead->GetValue();
				int newMountBody = look_mountbody->GetValue();
				int newMountLegs = look_mountlegs->GetValue();
				int newMountFeet = look_mountfeet->GetValue();

				if (newHead < 0 || newHead > OUTFIT_COLOR_MAX ||
					newBody < 0 || newBody > OUTFIT_COLOR_MAX ||
					newLegs < 0 || newLegs > OUTFIT_COLOR_MAX ||
					newFeet < 0 || newFeet > OUTFIT_COLOR_MAX ||
					newMountHead < 0 || newMountHead > OUTFIT_COLOR_MAX ||
					newMountBody < 0 || newMountBody > OUTFIT_COLOR_MAX ||
					newMountLegs < 0 || newMountLegs > OUTFIT_COLOR_MAX ||
					newMountFeet < 0 || newMountFeet > OUTFIT_COLOR_MAX
					) {
					wxString response = "Outfit and mount colors must be between 0 and ";
					response << i2ws(OUTFIT_COLOR_MAX) << ".";
					g_gui.PopupDialog(this, "Error", response, wxOK);
					return;
				}

				int newAddon = look_addon->GetValue();
				if (newAddon < 0 || newAddon > 3) {
					g_gui.PopupDialog(this, "Error", "Addons value must be between 0 and 3.", wxOK);
					return;
				}

				newOutfit.lookType = newLookType;
				newOutfit.lookHead = newHead;
				newOutfit.lookBody = newBody;
				newOutfit.lookLegs = newLegs;
				newOutfit.lookFeet = newFeet;
				newOutfit.lookAddon = newAddon;
				newOutfit.lookMount = newMount;
				newOutfit.lookMountHead = newMountHead;
				newOutfit.lookMountBody = newMountBody;
				newOutfit.lookMountLegs = newMountLegs;
				newOutfit.lookMountFeet = newMountFeet;
			}

			// Done validating, set the values.
			if(edit_item->canHoldDescription()) {
				edit_item->setText(new_desc);
			}
			if(edit_item->isStackable() || edit_item->isCharged()) {
				edit_item->setSubtype(new_count);
			}
			if(door) {
				door->setDoorID(new_door_id);
			}
			if(teleport) {
				teleport->setDestination(new_dest);
			}
			if(podium) {
				podium->setShowOutfit(show_outfit->GetValue());
				podium->setShowMount(show_mount->GetValue());
				podium->setShowPlatform(show_platform->GetValue());

				int* new_dir = reinterpret_cast<int*>(direction_field->GetClientData(direction_field->GetSelection()));
				if (new_dir) {
					podium->setDirection((Direction)*new_dir);
				}

				podium->setOutfit(newOutfit);
			}
			edit_item->setUniqueID(new_uid);
			edit_item->setActionID(new_aid);
			edit_item->setTier(new_tier);
		}
	} else if(edit_creature) {
		int new_spawntime = count_field->GetValue();
		edit_creature->setSpawnTime(new_spawntime);

		int* new_dir = reinterpret_cast<int*>(direction_field->GetClientData(
			direction_field->GetSelection()));

		if(new_dir) {
			edit_creature->setDirection((Direction)*new_dir);
		}
	} else if(edit_spawn) {
		int new_spawnsize = count_field->GetValue();
		edit_spawn->setSize(new_spawnsize);
	}
	EndModal(1);
}

void OldPropertiesWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event))
{
	// Just close this window
	EndModal(0);
}

void OldPropertiesWindow::Update()
{
	Container* container = dynamic_cast<Container*>(edit_item);
	if(container) {
		for(uint32_t i = 0; i < container->getVolume(); ++i) {
			container_items[i]->setItem(container->getItem(i));
		}
	}
	wxDialog::Update();
}
