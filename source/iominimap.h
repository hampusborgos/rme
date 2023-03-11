#ifndef RME_OTMM_H_
#define RME_OTMM_H_

#include "map.h"

enum class MinimapExportFormat {
	Otmm,
	Png,
	Bmp
};

enum class MinimapExportMode {
	AllFloors,
	GroundFloor,
	SpecificFloor,
	SelectedArea
};

enum {
	MMBLOCK_SIZE = 64,
	OTMM_SIGNATURE = 0x4D4d544F,
	OTMM_VERSION = 1
};

enum MinimapTileFlags {
    MinimapTileWasSeen = 1,
    MinimapTileNotPathable = 2,
    MinimapTileNotWalkable = 4
};

#pragma pack(push,1) // disable memory alignment
struct MinimapTile
{
    uint8_t flags = 0;
    uint8_t color = INVALID_MINIMAP_COLOR;
    uint8_t speed = 10;
};

class MinimapBlock
{
public:
    void updateTile(int x, int y, const MinimapTile& tile);
    MinimapTile& getTile(int x, int y) { return m_tiles[getTileIndex(x,y)]; }
    inline uint32_t getTileIndex(int x, int y) const noexcept { return ((y % MMBLOCK_SIZE) * MMBLOCK_SIZE) + (x % MMBLOCK_SIZE); }
    const std::array<MinimapTile, MMBLOCK_SIZE * MMBLOCK_SIZE>& getTiles() const noexcept { return m_tiles; }

private:
    std::array<MinimapTile, MMBLOCK_SIZE * MMBLOCK_SIZE> m_tiles;
};
#pragma pack(pop)

class IOMinimap
{
public:
	IOMinimap(Editor* editor, MinimapExportFormat format, MinimapExportMode mode, bool updateLoadbar);

	bool saveMinimap(const std::string& directory, const std::string& name, int floor = -1);

	const std::string& getError() const noexcept { return m_error; }

private:
	bool saveOtmm(const wxFileName& file);
	bool saveImage(const std::string& directory, const std::string& name);
	bool exportMinimap(const std::string& directory);
	bool exportSelection(const std::string& directory, const std::string& name);
	void readBlocks();
	inline uint32_t getBlockIndex(const Position& pos) {
		return ((pos.y / MMBLOCK_SIZE) * (65536 / MMBLOCK_SIZE)) + (pos.x / MMBLOCK_SIZE);
	}

	Editor* m_editor;
	MinimapExportFormat m_format;
	MinimapExportMode m_mode;
	bool m_updateLoadbar = false;
	int m_floor = -1;
	std::unordered_map<uint32_t, MinimapBlock> m_blocks[rme::MapLayers];
	std::string m_error;
};

#endif
