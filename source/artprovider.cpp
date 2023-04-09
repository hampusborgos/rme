//////////////////////////////////////////////////////////////////////
// This file is part of Canary Map Editor
//////////////////////////////////////////////////////////////////////
// Canary Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Canary Map Editor is distributed in the hope that it will be useful,
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

wxBitmap ArtProvider::CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& WXUNUSED(size))
{
	if (client == wxART_TOOLBAR) {
		if (id == ART_CIRCULAR)
			return wxBitmap(circular_4_xpm);
		else if (id == ART_CIRCULAR_1)
			return wxBitmap(circular_1_xpm);
		else if (id == ART_CIRCULAR_2)
			return wxBitmap(circular_2_xpm);
		else if (id == ART_CIRCULAR_3)
			return wxBitmap(circular_3_xpm);
		else if (id == ART_CIRCULAR_4)
			return wxBitmap(circular_4_xpm);
		else if (id == ART_CIRCULAR_5)
			return wxBitmap(circular_5_xpm);
		else if (id == ART_CIRCULAR_6)
			return wxBitmap(circular_6_xpm);
		else if (id == ART_CIRCULAR_7)
			return wxBitmap(circular_7_xpm);
		else if (id == ART_NOLOOUT_BRUSH)
			return wxBitmap(nologout_zone_xpm);
		else if (id == ART_NOPVP_BRUSH)
			return wxBitmap(nopvp_zone_xpm);
		else if (id == ART_POSITION_GO)
			return wxBitmap(position_go_xpm);
		else if (id == ART_PVP_BRUSH)
			return wxBitmap(pvp_zone_xpm);
		else if (id == ART_PZ_BRUSH)
			return wxBitmap(protected_zone_xpm);
		else if (id == ART_RECTANGULAR)
			return wxBitmap(rectangular_4_xpm);
		else if (id == ART_RECTANGULAR_1)
			return wxBitmap(rectangular_1_xpm);
		else if (id == ART_RECTANGULAR_2)
			return wxBitmap(rectangular_2_xpm);
		else if (id == ART_RECTANGULAR_3)
			return wxBitmap(rectangular_3_xpm);
		else if (id == ART_RECTANGULAR_4)
			return wxBitmap(rectangular_4_xpm);
		else if (id == ART_RECTANGULAR_5)
			return wxBitmap(rectangular_5_xpm);
		else if (id == ART_RECTANGULAR_6)
			return wxBitmap(rectangular_6_xpm);
		else if (id == ART_RECTANGULAR_7)
			return wxBitmap(rectangular_7_xpm);
	}

	return wxNullBitmap;
}
