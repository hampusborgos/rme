//////////////////////////////////////////////////////////////////////
// Remere's Map Editor - An opensource map editor for OpenTibia
//////////////////////////////////////////////////////////////////////

#ifndef RME_CLIENT_VERSION_H_
#define RME_CLIENT_VERSION_H_

#include "main.h"
#include "settings.h"

typedef int ClientVersionID;

#define CLIENT_VERSION_NONE -1
#define CLIENT_VERSION_ALL  -2

// After this version, OTBM stores charges as an attribute
#define CLIENT_VERSION_820 10

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

enum OtbFormatVersion
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

// Formats for the .dat data file for Tibia
enum DatVersion
{
	DAT_VERSION_UNKNOWN,
	DAT_VERSION_74,
	DAT_VERSION_76,
	DAT_VERSION_78,
	DAT_VERSION_86,
	DAT_VERSION_96,
};

// Possible format for the .spr data file for Tibia
enum SprVersion
{
	SPR_VERSION_UNKNOWN,
	SPR_VERSION_70,
	SPR_VERSION_96, // 32 bit sprids
};

// Represents a client file version
struct ClientData
{
	DatVersion datVersion;
	SprVersion sprVersion;
	uint32_t datSignature;
	uint32_t sprSignature;
};

// typedef the client version
class ClientVersion;
typedef std::vector<ClientVersion*> ClientVersionList;


class ClientVersion : boost::noncopyable
{
public:
	ClientVersion(OtbVersion otb, wxString versionName, wxString path);
	~ClientVersion() {}

	static void loadVersions();
	static void saveVersions();

	static ClientVersion* get(ClientVersionID id);
	static ClientVersion* get(std::string name);
	static ClientVersionList getVisible(std::string from, std::string to);
	static ClientVersionList getAll();
	static ClientVersionList getAllVisible();
	static ClientVersionList getAllForOTBMVersion(MapVersion map_version);
	static ClientVersion* getLatestVersion();

	bool operator==(const ClientVersion& o) const {return otb.id == o.otb.id;}
	
	bool hasValidPaths() const;
	bool loadValidPaths();
	void setClientPath(const FileName& dir);
	
	bool isVisible() const;
	std::string getName() const;

	ClientVersionID getID() const;
	MapVersionID getPrefferedMapVersionID() const;
	OtbVersion getOTBVersion() const;
	DatVersion getDatVersionForSignature(uint32_t signature) const;
	SprVersion getSprVersionForSignature(uint32_t signature) const;
	ClientVersionList getExtensionsSupported() const;

	FileName getDataPath() const;
	FileName getLocalDataPath() const;
	FileName getClientPath() const;


private:
	OtbVersion otb;
	
	wxString name;
	bool visible;
	bool usesFuckedUpCharges;

	std::vector<MapVersionID> map_versions_supported;
	MapVersionID preferred_map_version;
	std::vector<ClientData> data_versions;

	wxString data_path;
	FileName client_path;

private:
	static void loadOTBs(xmlNodePtr otb_nodes);
	static void loadVersion(xmlNodePtr client_node);

	// All versions
	typedef std::map<ClientVersionID, ClientVersion*> VersionMap;
	static VersionMap client_versions;
	static ClientVersion* latest_version;

	// All otbs
	typedef std::map<std::string, OtbVersion> OtbMap;
	static OtbMap otb_versions;
};

#endif
