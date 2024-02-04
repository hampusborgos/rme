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

#ifndef RME_CONST_H_
#define RME_CONST_H_

namespace rme {

constexpr double PI = 3.14159265;

// The height of the map (there should be more checks for this...)
constexpr int MapLayers = 16;

constexpr int MapMinWidth = 256;
constexpr int MapMaxWidth = 65000;
constexpr int MapMinHeight = 256;
constexpr int MapMaxHeight = 65000;
constexpr int MapMinLayer = 0;
constexpr int MapMaxLayer = 15;

// The sea layer
constexpr int MapGroundLayer = 7;

constexpr int ClientMapWidth = 18;
constexpr int ClientMapHeight = 14;

// The size of the tile in pixels
constexpr int TileSize = 32;

// The default size of sprites
constexpr int SpritePixels = 32;
constexpr int SpritePixelsSize = SpritePixels * SpritePixels;

constexpr int MaxLightIntensity = 8;

constexpr int PixelFormatRGB = 3;
constexpr int PixelFormatRGBA = 4;

constexpr int MinUniqueId = 1000;
constexpr int MaxUniqueId = 65535;
constexpr int MinActionId = 100;
constexpr int MaxActionId = 65535;

// Position indicator when using the position control
constexpr int PositionIndicatorDuration = 3000;

} // namespace rme

#endif // RME_CONST_H_
