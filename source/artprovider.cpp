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

#include "main.h"
#include "artprovider.h"

#include "../icons/nologout_zone.xpm"
#include "../icons/nopvp_zone.xpm"
#include "../icons/position_go.xpm"
#include "../icons/protected_zone.xpm"
#include "../icons/pvp_zone.xpm"

wxBitmap ArtProvider::CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& WXUNUSED(size))
{
	if (client == wxART_TOOLBAR) {
		if (id == ART_POSITION_GO)
			return wxBitmap(position_go_xpm);
		else if (id == ART_NOLOOUT_BRUSH)
			return wxBitmap(nologout_zone_xpm);
		else if (id == ART_NOPVP_BRUSH)
			return wxBitmap(nopvp_zone_xpm);
		else if (id == ART_PZ_BRUSH)
			return wxBitmap(protected_zone_xpm);
		else if (id == ART_PVP_BRUSH)
			return wxBitmap(pvp_zone_xpm);
	}

	return wxNullBitmap;
}
