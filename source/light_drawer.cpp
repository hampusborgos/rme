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
#include "light_drawer.h"

LightDrawer::LightDrawer()
{
	texture = 0;
	buffer.resize(static_cast<size_t>(rme::ClientMapWidth * rme::ClientMapHeight * rme::PixelFormatRGBA));
	global_color = wxColor(50, 50, 50, 255);

	createGLTexture();
}

LightDrawer::~LightDrawer()
{
	unloadGLTexture();

	lights.clear();
}

void LightDrawer::draw(int map_x, int map_y, int scroll_x, int scroll_y)
{
	constexpr int half_tile_size = rme::TileSize / 2;

	for (int x = 0; x < rme::ClientMapWidth; ++x) {
		for (int y = 0; y < rme::ClientMapHeight; ++y) {
			int mx = (map_x + x);
			int my = (map_y + y);
			int px = (mx * rme::TileSize + half_tile_size);
			int py = (my * rme::TileSize + half_tile_size);
			int index = (y * rme::ClientMapWidth + x);
			int color_index = index * rme::PixelFormatRGBA;

			buffer[color_index]	 = global_color.Red();
			buffer[color_index + 1] = global_color.Green();
			buffer[color_index + 2] = global_color.Blue();
			buffer[color_index + 3] = global_color.Alpha();

			for (auto& light : lights) {
				float intensity = calculateIntensity(mx, my, light);
				if (intensity == 0.f) {
					continue;
				}
				wxColor light_color = colorFromEightBit(light.color);
				uint8_t red = static_cast<uint8_t>(light_color.Red() * intensity);
				uint8_t green = static_cast<uint8_t>(light_color.Green() * intensity);
				uint8_t blue = static_cast<uint8_t>(light_color.Blue() * intensity);
				buffer[color_index]	 = std::max(buffer[color_index], red);
				buffer[color_index + 1] = std::max(buffer[color_index + 1], green);
				buffer[color_index + 2] = std::max(buffer[color_index + 2], blue);
			}
		}
	}

	const int draw_x = map_x * rme::TileSize - scroll_x;
	const int draw_y = map_y * rme::TileSize - scroll_y;
	constexpr int draw_width = rme::ClientMapWidth * rme::TileSize;
	constexpr int draw_height = rme::ClientMapHeight * rme::TileSize;

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rme::ClientMapWidth, rme::ClientMapHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

	glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glTexCoord2f(0.f, 0.f); glVertex2f(draw_x, draw_y);
		glTexCoord2f(1.f, 0.f); glVertex2f(draw_x + draw_width, draw_y);
		glTexCoord2f(1.f, 1.f); glVertex2f(draw_x + draw_width, draw_y + draw_height);
		glTexCoord2f(0.f, 1.f); glVertex2f(draw_x, draw_y + draw_height);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void LightDrawer::setGlobalLightColor(uint8_t color)
{
	global_color = colorFromEightBit(color);
}

void LightDrawer::addLight(int map_x, int map_y, const SpriteLight& light)
{
	if (map_x <= 0 || map_x >= rme::MapMaxWidth || map_y <= 0 || map_y >= rme::MapMaxHeight) {
		return;
	}

	uint8_t intensity = std::min(light.intensity, static_cast<uint8_t>(rme::MaxLightIntensity));

	if (!lights.empty()) {
		Light& previous = lights.back();
		if (previous.map_x == map_x && previous.map_y == map_y && previous.color == light.color) {
			previous.intensity = std::max(previous.intensity, intensity);
			return;
		}
	}

	lights.push_back(Light{ static_cast<uint16_t>(map_x), static_cast<uint16_t>(map_y), light.color, intensity });
}

void LightDrawer::clear() noexcept
{
	lights.clear();
}

void LightDrawer::createGLTexture()
{
	glGenTextures(1, &texture);
}

void LightDrawer::unloadGLTexture()
{
	glDeleteTextures(1, &texture);
}
