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

#ifndef RME_CLIENT_VERSION_H_
#define RME_CLIENT_VERSION_H_

#include "main.h"
#include "settings.h"

typedef int ClientVersionID;

// Client versions
enum ClientVersions {
	CLIENT_VERSION_NONE = -1,
	CLIENT_VERSION_ALL = -2,
	CLIENT_VERSION_750 = 1,
	CLIENT_VERSION_755 = 2,
	CLIENT_VERSION_760 = 3,
	CLIENT_VERSION_770 = 3,
	CLIENT_VERSION_780 = 4,
	CLIENT_VERSION_790 = 5,
	CLIENT_VERSION_792 = 6,
	CLIENT_VERSION_800 = 7,
	CLIENT_VERSION_810 = 8,
	CLIENT_VERSION_811 = 9,
	CLIENT_VERSION_820 = 10, // After this version, OTBM stores charges as an attribute
	CLIENT_VERSION_830 = 11,
	CLIENT_VERSION_840 = 12,
	CLIENT_VERSION_841 = 13,
	CLIENT_VERSION_842 = 14,
	CLIENT_VERSION_850 = 15,
	CLIENT_VERSION_854_BAD = 16,
	CLIENT_VERSION_854 = 17,
	CLIENT_VERSION_855 = 18,
	CLIENT_VERSION_860_OLD = 19,
	CLIENT_VERSION_860 = 20,
	CLIENT_VERSION_861 = 21,
	CLIENT_VERSION_862 = 22,
	CLIENT_VERSION_870 = 23,
	CLIENT_VERSION_871 = 24,
	CLIENT_VERSION_872 = 25,
	CLIENT_VERSION_873 = 26,
	CLIENT_VERSION_900 = 27,
	CLIENT_VERSION_910 = 28,
	CLIENT_VERSION_920 = 29,
	CLIENT_VERSION_940 = 30,
	CLIENT_VERSION_944_V1 = 31,
	CLIENT_VERSION_944_V2 = 32,
	CLIENT_VERSION_944_V3 = 33,
	CLIENT_VERSION_944_V4 = 34,
	CLIENT_VERSION_946 = 35,
	CLIENT_VERSION_950 = 36,
	CLIENT_VERSION_952 = 37,
	CLIENT_VERSION_953 = 38,
	CLIENT_VERSION_954 = 39,
	CLIENT_VERSION_960 = 40,
	CLIENT_VERSION_961 = 41,
	CLIENT_VERSION_963 = 42,
	CLIENT_VERSION_970 = 43,
	CLIENT_VERSION_980 = 44,
	CLIENT_VERSION_981 = 45,
	CLIENT_VERSION_982 = 46,
	CLIENT_VERSION_983 = 47,
	CLIENT_VERSION_985 = 48,
	CLIENT_VERSION_986 = 49,
	CLIENT_VERSION_1010 = 50,
	CLIENT_VERSION_1020 = 51,
	CLIENT_VERSION_1021 = 52
};

// OTBM versions
enum MapVersionID
{
	MAP_OTBM_UNKNOWN = -1,
	MAP_OTBM_1 = 0,
	MAP_OTBM_2 = 1,
	MAP_OTBM_3 = 2,
	MAP_OTBM_4 = 3,
};

// The composed version of a otbm file (otbm version, client version)
struct MapVersion
{
	MapVersion() : otbm(MAP_OTBM_1), client(CLIENT_VERSION_NONE) {}
	MapVersion(MapVersionID m, ClientVersionID c) : otbm(m), client(c) {}
	MapVersionID otbm;
	ClientVersionID client;
};

enum OtbFormatVersion : uint32_t
{
	OTB_VERSION_1 = 1,
	OTB_VERSION_2 = 2,
	OTB_VERSION_3 = 3,
};

// Represents an OTB version
struct OtbVersion
{
	// '8.60', '7.40' etc.
	std::string name;
	// What file format the OTB is in (version 1..3)
	OtbFormatVersion format_version;
	// The minor version ID of the OTB (maps to CLIENT_VERSION in OTServ)
	ClientVersionID id;
};

// Formats for the metadata file
enum DatFormat
{
	DAT_FORMAT_UNKNOWN,
	DAT_FORMAT_74,
	DAT_FORMAT_755,
	DAT_FORMAT_78,
	DAT_FORMAT_86,
	DAT_FORMAT_96,
	DAT_FORMAT_1010,
	DAT_FORMAT_1050,
	DAT_FORMAT_1057
};

enum DatFlags : uint8_t
{
	DatFlagGround = 0,
	DatFlagGroundBorder = 1,
	DatFlagOnBottom = 2,
	DatFlagOnTop = 3,
	DatFlagContainer = 4,
	DatFlagStackable = 5,
	DatFlagForceUse = 6,
	DatFlagMultiUse = 7,
	DatFlagWritable = 8,
	DatFlagWritableOnce = 9,
	DatFlagFluidContainer = 10,
	DatFlagSplash = 11,
	DatFlagNotWalkable = 12,
	DatFlagNotMoveable = 13,
	DatFlagBlockProjectile = 14,
	DatFlagNotPathable = 15,
	DatFlagPickupable = 16,
	DatFlagHangable = 17,
	DatFlagHookSouth = 18,
	DatFlagHookEast = 19,
	DatFlagRotateable = 20,
	DatFlagLight = 21,
	DatFlagDontHide = 22,
	DatFlagTranslucent = 23,
	DatFlagDisplacement = 24,
	DatFlagElevation = 25,
	DatFlagLyingCorpse = 26,
	DatFlagAnimateAlways = 27,
	DatFlagMinimapColor = 28,
	DatFlagLensHelp = 29,
	DatFlagFullGround = 30,
	DatFlagLook = 31,
	DatFlagCloth = 32,
	DatFlagMarket = 33,
	DatFlagUsable = 34,
	DatFlagWrappable = 35,
	DatFlagUnwrappable = 36,
	DatFlagTopEffect = 37,

	DatFlagFloorChange = 252,
	DatFlagNoMoveAnimation = 253, // 10.10: real value is 16, but we need to do this for backwards compatibility
	DatFlagChargeable = 254,
	DatFlagLast = 255
};

// Represents a client file version
struct ClientData
{
	DatFormat datFormat;
	uint32_t datSignature;
	uint32_t sprSignature;
};

// typedef the client version
class ClientVersion;
typedef std::vector<ClientVersion*> ClientVersionList;

class ClientVersion : boost::noncopyable
{
public:
	ClientVersion(OtbVersion otb, std::string versionName, wxString path);
	~ClientVersion() = default;

	static void loadVersions();
	static void unloadVersions();
	static void saveVersions();

	static ClientVersion* get(ClientVersionID id);
	static ClientVersion* get(std::string name);
	static ClientVersionList getVisible(std::string from, std::string to);
	static ClientVersionList getAll();
	static ClientVersionList getAllVisible();
	static ClientVersionList getAllForOTBMVersion(MapVersionID map_version);
	static ClientVersionList getAllVersionsSupportedForClientVersion(ClientVersion* v);
	static ClientVersion* getLatestVersion();

	bool operator==(const ClientVersion& o) const {return otb.id == o.otb.id;}

	bool hasValidPaths();
	bool loadValidPaths();
	void setClientPath(const FileName& dir);

	bool isVisible() const;
	std::string getName() const;

	ClientVersionID getID() const;
	MapVersionID getPrefferedMapVersionID() const;
	OtbVersion getOTBVersion() const;
	DatFormat getDatFormatForSignature(uint32_t signature) const;
	ClientVersionList getExtensionsSupported() const;

	FileName getDataPath() const;
	FileName getLocalDataPath() const;
	FileName getClientPath() const { return client_path; }
	wxFileName getMetadataPath() const { return metadata_path; }
	wxFileName getSpritesPath() const { return sprites_path; }

private:
	OtbVersion otb;

	std::string name;
	bool visible;
	bool usesFuckedUpCharges;

	std::vector<MapVersionID> map_versions_supported;
	MapVersionID preferred_map_version;
	std::vector<ClientData> data_versions;
	std::vector<ClientVersion*> extension_versions;

	wxString data_path;
	FileName client_path;
	wxFileName metadata_path;
	wxFileName sprites_path;

private:
	static void loadOTBInfo(pugi::xml_node otb_nodes);
	static void loadVersion(pugi::xml_node client_node);
	static void loadVersionExtensions(pugi::xml_node client_node);

	// All versions
	typedef std::map<ClientVersionID, ClientVersion*> VersionMap;
	static VersionMap client_versions;
	static ClientVersion* latest_version;

	// All otbs
	typedef std::map<std::string, OtbVersion> OtbMap;
	static OtbMap otb_versions;
};

inline int VersionComparisonPredicate(ClientVersion* a, ClientVersion* b)
{
	if(a->getID() < b->getID()) {
		return 1;
	}
	return 0;
}

#endif
