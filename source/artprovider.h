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

#ifndef RME_ART_PROVIDER_H_
#define RME_ART_PROVIDER_H_

#include <wx/artprov.h>

#define ART_CIRCULAR wxART_MAKE_ART_ID(ART_CIRCULAR)
#define ART_CIRCULAR_1 wxART_MAKE_ART_ID(ART_CIRCULAR_1)
#define ART_CIRCULAR_2 wxART_MAKE_ART_ID(ART_CIRCULAR_2)
#define ART_CIRCULAR_3 wxART_MAKE_ART_ID(ART_CIRCULAR_3)
#define ART_CIRCULAR_4 wxART_MAKE_ART_ID(ART_CIRCULAR_4)
#define ART_CIRCULAR_5 wxART_MAKE_ART_ID(ART_CIRCULAR_5)
#define ART_CIRCULAR_6 wxART_MAKE_ART_ID(ART_CIRCULAR_6)
#define ART_CIRCULAR_7 wxART_MAKE_ART_ID(ART_CIRCULAR_7)
#define ART_NOLOOUT_BRUSH wxART_MAKE_ART_ID(ART_NOLOOUT_BRUSH)
#define ART_NOPVP_BRUSH wxART_MAKE_ART_ID(ART_NOPVP_BRUSH)
#define ART_POSITION_GO wxART_MAKE_ART_ID(ART_POSITION_GO)
#define ART_PVP_BRUSH wxART_MAKE_ART_ID(ART_PVP_BRUSH)
#define ART_PZ_BRUSH wxART_MAKE_ART_ID(ART_PZ_BRUSH)
#define ART_RECTANGULAR wxART_MAKE_ART_ID(ART_RECTANGULAR)
#define ART_RECTANGULAR_1 wxART_MAKE_ART_ID(ART_RECTANGULAR_1)
#define ART_RECTANGULAR_2 wxART_MAKE_ART_ID(ART_RECTANGULAR_2)
#define ART_RECTANGULAR_3 wxART_MAKE_ART_ID(ART_RECTANGULAR_3)
#define ART_RECTANGULAR_4 wxART_MAKE_ART_ID(ART_RECTANGULAR_4)
#define ART_RECTANGULAR_5 wxART_MAKE_ART_ID(ART_RECTANGULAR_5)
#define ART_RECTANGULAR_6 wxART_MAKE_ART_ID(ART_RECTANGULAR_6)
#define ART_RECTANGULAR_7 wxART_MAKE_ART_ID(ART_RECTANGULAR_7)

#define ART_HOOKS_TOOLBAR wxART_MAKE_ART_ID(ART_HOOKS_TOOLBAR)
#define ART_PICKUPABLE_TOOLBAR wxART_MAKE_ART_ID(ART_PICKUPABLE_TOOLBAR)
#define ART_MOVEABLE_TOOLBAR wxART_MAKE_ART_ID(ART_MOVEABLE_TOOLBAR)

#define ART_SPAWNS wxART_MAKE_ART_ID(ART_SPAWNS)
#define ART_HOUSE_EXIT wxART_MAKE_ART_ID(ART_HOUSE_EXIT)
#define ART_PICKUPABLE wxART_MAKE_ART_ID(ART_PICKUPABLE)
#define ART_MOVEABLE wxART_MAKE_ART_ID(ART_MOVEABLE)
#define ART_PICKUPABLE_MOVEABLE wxART_MAKE_ART_ID(ART_PICKUPABLE_MOVEABLE)

#define ART_MOVE wxART_MAKE_ART_ID(ART_MOVE)
#define ART_REMOTE wxART_MAKE_ART_ID(ART_REMOTE)
#define ART_SELECT wxART_MAKE_ART_ID(ART_SELECT)
#define ART_UNSELECT wxART_MAKE_ART_ID(ART_UNSELECT)
#define ART_DELETE wxART_MAKE_ART_ID(ART_DELETE)
#define ART_CUT wxART_MAKE_ART_ID(ART_CUT)
#define ART_PASTE wxART_MAKE_ART_ID(ART_PASTE)
#define ART_RANDOMIZE wxART_MAKE_ART_ID(ART_RANDOMIZE)
#define ART_BORDERIZE wxART_MAKE_ART_ID(ART_BORDERIZE)
#define ART_DRAW wxART_MAKE_ART_ID(ART_DRAW)
#define ART_ERASE wxART_MAKE_ART_ID(ART_ERASE)
#define ART_SWITCH wxART_MAKE_ART_ID(ART_SWITCH)
#define ART_ROTATE wxART_MAKE_ART_ID(ART_ROTATE)
#define ART_REPLACE wxART_MAKE_ART_ID(ART_REPLACE)
#define ART_CHANGE wxART_MAKE_ART_ID(ART_CHANGE)

class ArtProvider : public wxArtProvider
{
protected:
	virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size) override;
};

#endif // RME_ART_PROVIDER_H_
