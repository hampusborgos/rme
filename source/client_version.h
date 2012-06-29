//////////////////////////////////////////////////////////////////////
// Remere's Map Editor - An opensource map editor for OpenTibia
//////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/client_version.h $
// $Id: client_version.h 306 2010-02-24 10:38:25Z admin $

#ifndef RME_CLIENT_VERSION_H_
#define RME_CLIENT_VERSION_H_

#include "main.h"
#include "settings.h"

enum ClientVersionID {
	CLIENT_VERSION_NONE= 0,
	CLIENT_VERSION_740 = 1, // HACK
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
	CLIENT_VERSION_820 = 10,
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
	CLIENT_VERSION_874 = 26,
	CLIENT_VERSION_900 = 27,
	CLIENT_VERSION_910 = 28,
	CLIENT_VERSION_920 = 29
};

enum MapVersionID
{
	MAP_OTBM_1 = 0,
	MAP_OTBM_2 = 1,
	MAP_OTBM_3 = 2,
	MAP_OTBM_4 = 3,
};

struct MapVersion
{
	MapVersion() : otbm(MAP_OTBM_1), client(CLIENT_VERSION_NONE) {}
	MapVersion(MapVersionID m, ClientVersionID c) : otbm(m), client(c) {}
	MapVersionID otbm;
	ClientVersionID client;
};

class ClientVersion;
typedef std::vector<ClientVersion*> ClientVersionList;

class ClientVersion {
public:
	ClientVersion() : ver_id(CLIENT_VERSION_NONE) {}
	ClientVersion(ClientVersionID id, wxString versionName, wxString path, bool hidden = false);
	ClientVersion(const ClientVersion& other);
	~ClientVersion() {}

	static void loadVersions();
	static void saveVersions();
	static void addVersion(const ClientVersion& ver);

	static ClientVersion* get(ClientVersionID id);
	static ClientVersion* get(std::string name);
	static ClientVersionList getAll();

	bool operator==(const ClientVersion& o) const {return ver_id == o.ver_id;}
	
	bool hasValidPaths() const;
	bool loadValidPaths();
	void setClientPath(const FileName& dir);
	
	bool isHidden() const;
	void setHidden(bool hidden);
	std::string getName() const;
	ClientVersionID getID() const;
	FileName getDataPath() const;
	FileName getLocalDataPath() const;
	FileName getClientPath() const;
	MapVersionID getMapVersionID() const;

private:
	ClientVersionID ver_id;
	bool hidden;
	wxString name;
	std::vector<std::pair<uint32_t, uint32_t> > version_id_list;
	wxString data_path;
	FileName client_path;

	// All versions
	typedef std::map<ClientVersionID, ClientVersion> VersionMap;
	static VersionMap versions;
};

#endif
