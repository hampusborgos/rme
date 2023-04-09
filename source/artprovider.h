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

#ifndef RME_ART_PROVIDER_H_
#define RME_ART_PROVIDER_H_

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

class ArtProvider : public wxArtProvider
{
protected:
	virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size) override;
};

#endif // RME_ART_PROVIDER_H_
