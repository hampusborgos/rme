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

#ifndef RME_ART_PROVIDER_H_
#define RME_ART_PROVIDER_H_

#include <wx/artprov.h>

#define ART_POSITION_GO wxART_MAKE_ART_ID(ART_POSITION_GO)
#define ART_NOLOOUT_BRUSH wxART_MAKE_ART_ID(ART_NOLOOUT_BRUSH)
#define ART_NOPVP_BRUSH wxART_MAKE_ART_ID(ART_NOPVP_BRUSH)
#define ART_PZ_BRUSH wxART_MAKE_ART_ID(ART_PZ_BRUSH)
#define ART_PVP_BRUSH wxART_MAKE_ART_ID(ART_PVP_BRUSH)

class ArtProvider : public wxArtProvider
{
protected:
	virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size) wxOVERRIDE;
};

#endif // RME_ART_PROVIDER_H_
