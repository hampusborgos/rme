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
#include "artprovider.h"

#include "../icons/circular_1.xpm"
#include "../icons/circular_2.xpm"
#include "../icons/circular_3.xpm"
#include "../icons/circular_4.xpm"
#include "../icons/circular_5.xpm"
#include "../icons/circular_6.xpm"
#include "../icons/circular_7.xpm"
#include "../icons/nologout_zone.xpm"
#include "../icons/nopvp_zone.xpm"
#include "../icons/position_go.xpm"
#include "../icons/protected_zone.xpm"
#include "../icons/pvp_zone.xpm"
#include "../icons/rectangular_1.xpm"
#include "../icons/rectangular_2.xpm"
#include "../icons/rectangular_3.xpm"
#include "../icons/rectangular_4.xpm"
#include "../icons/rectangular_5.xpm"
#include "../icons/rectangular_6.xpm"
#include "../icons/rectangular_7.xpm"
#include "../icons/toolbar_hooks.xpm"
#include "../icons/toolbar_pickupables.xpm"
#include "../icons/toolbar_moveables.xpm"
#include "../icons/spawns.xpm"
#include "../icons/house_exit.xpm"
#include "../icons/pickupable.xpm"
#include "../icons/moveable.xpm"
#include "../icons/pickupable_moveable.xpm"
#include "../icons/mini_move.xpm"
#include "../icons/mini_remote.xpm"
#include "../icons/mini_select.xpm"
#include "../icons/mini_delete.xpm"
#include "../icons/mini_cut.xpm"
#include "../icons/mini_paste.xpm"
#include "../icons/mini_randomize.xpm"
#include "../icons/mini_borderize.xpm"
#include "../icons/mini_draw.xpm"
#include "../icons/mini_erase.xpm"
#include "../icons/mini_switch.xpm"
#include "../icons/mini_rotate.xpm"
#include "../icons/mini_replace.xpm"
#include "../icons/mini_change.xpm"

wxBitmap ArtProvider::CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& WXUNUSED(size))
{
	if(client == wxART_TOOLBAR) {
		if(id == ART_CIRCULAR)
			return wxBitmap(circular_4_xpm);
		else if(id == ART_CIRCULAR_1)
			return wxBitmap(circular_1_xpm);
		else if(id == ART_CIRCULAR_2)
			return wxBitmap(circular_2_xpm);
		else if(id == ART_CIRCULAR_3)
			return wxBitmap(circular_3_xpm);
		else if(id == ART_CIRCULAR_4)
			return wxBitmap(circular_4_xpm);
		else if(id == ART_CIRCULAR_5)
			return wxBitmap(circular_5_xpm);
		else if(id == ART_CIRCULAR_6)
			return wxBitmap(circular_6_xpm);
		else if(id == ART_CIRCULAR_7)
			return wxBitmap(circular_7_xpm);
		else if(id == ART_NOLOOUT_BRUSH)
			return wxBitmap(nologout_zone_xpm);
		else if(id == ART_NOPVP_BRUSH)
			return wxBitmap(nopvp_zone_xpm);
		else if(id == ART_POSITION_GO)
			return wxBitmap(position_go_xpm);
		else if(id == ART_PVP_BRUSH)
			return wxBitmap(pvp_zone_xpm);
		else if(id == ART_PZ_BRUSH)
			return wxBitmap(protected_zone_xpm);
		else if(id == ART_RECTANGULAR)
			return wxBitmap(rectangular_4_xpm);
		else if(id == ART_RECTANGULAR_1)
			return wxBitmap(rectangular_1_xpm);
		else if(id == ART_RECTANGULAR_2)
			return wxBitmap(rectangular_2_xpm);
		else if(id == ART_RECTANGULAR_3)
			return wxBitmap(rectangular_3_xpm);
		else if(id == ART_RECTANGULAR_4)
			return wxBitmap(rectangular_4_xpm);
		else if(id == ART_RECTANGULAR_5)
			return wxBitmap(rectangular_5_xpm);
		else if(id == ART_RECTANGULAR_6)
			return wxBitmap(rectangular_6_xpm);
		else if(id == ART_RECTANGULAR_7)
			return wxBitmap(rectangular_7_xpm);
		else if(id == ART_HOOKS_TOOLBAR)
			return wxBitmap(toolbar_hooks_xpm);
		else if(id == ART_PICKUPABLE_TOOLBAR)
			return wxBitmap(toolbar_pickupables_xpm);
		else if(id == ART_MOVEABLE_TOOLBAR)
			return wxBitmap(toolbar_moveable_xpm);
	} else if(client == wxART_LIST) {
		if(id == ART_MOVE)
			return wxBitmap(mini_move_xpm);
		else if(id == ART_REMOTE)
			return wxBitmap(mini_remote_xpm);
		else if(id == ART_SELECT )
			return wxBitmap(mini_select_xpm);
		else if(id == ART_DELETE)
			return wxBitmap(mini_delete_xpm);
		else if(id == ART_CUT)
			return wxBitmap(mini_cut_xpm);
		else if(id == ART_PASTE)
			return wxBitmap(mini_paste_xpm);
		else if(id == ART_RANDOMIZE)
			return wxBitmap(mini_randomize_xpm);
		else if(id == ART_BORDERIZE)
			return wxBitmap(mini_borderize_xpm);
		else if(id == ART_DRAW)
			return wxBitmap(mini_draw_xpm);
		else if(id == ART_ERASE)
			return wxBitmap(mini_erase_xpm);
		else if(id == ART_SWITCH)
			return wxBitmap(mini_switch_xpm);
		else if(id == ART_ROTATE)
			return wxBitmap(mini_rotate_xpm);
		else if(id == ART_REPLACE)
			return wxBitmap(mini_replace_xpm);
		else if(id == ART_CHANGE)
			return wxBitmap(mini_change_xpm);
	} else if(client == wxART_OTHER) {
		if(id == ART_SPAWNS)
			return wxBitmap(spawns_xpm);
		else if(id == ART_HOUSE_EXIT)
			return wxBitmap(house_exit_xpm);
		else if(id == ART_PICKUPABLE)
			return wxBitmap(pickupable_xpm);
		else if(id == ART_MOVEABLE)
			return wxBitmap(moveable_xpm);
		else if(id == ART_PICKUPABLE_MOVEABLE)
			return wxBitmap(pickupable_moveable_xpm);
	}
	return wxNullBitmap;
}
