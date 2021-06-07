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

#include "settings.h"
#include "filehandle.h"

#include "gui.h"

#include "client_version.h"
#include "otml.h"
#include <wx/dir.h>

// Static methods to load/save

ClientVersion::VersionMap ClientVersion::client_versions;
ClientVersion* ClientVersion::latest_version = nullptr;
ClientVersion::OtbMap ClientVersion::otb_versions;

void ClientVersion::loadVersions()
{
	// Clean up old stuff
	ClientVersion::unloadVersions();

	// Locate the clients.xml file
	wxFileName file_to_load;

	wxFileName exec_dir_client_xml;
	exec_dir_client_xml.Assign(g_gui.GetExecDirectory());
	exec_dir_client_xml.SetFullName("clients.xml");

	wxFileName data_dir_client_xml;
	data_dir_client_xml.Assign(g_gui.GetDataDirectory());
	data_dir_client_xml.SetFullName("clients.xml");

	wxFileName work_dir_client_xml;
	work_dir_client_xml.Assign(g_gui.getFoundDataDirectory());
	work_dir_client_xml.SetFullName("clients.xml");

	file_to_load = exec_dir_client_xml;
	if(!file_to_load.FileExists()) {
		file_to_load = data_dir_client_xml;
		if(!file_to_load.FileExists()) {
			file_to_load = work_dir_client_xml;
			if(!file_to_load.FileExists()) {
				file_to_load.Clear();
			}
		}
	}

	if(!file_to_load.FileExists()) {
		wxLogError(wxString() +
			"Could not load clients.xml, editor will NOT be able to load any client data files.\n" +
			"Checked paths:\n" +
			exec_dir_client_xml.GetFullPath() + "\n" +
			data_dir_client_xml.GetFullPath() + "\n" +
			work_dir_client_xml.GetFullPath()
		);
		return;
	}

	// Parse the file
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(file_to_load.GetFullPath().mb_str());
	if(result) {
		pugi::xml_node node = doc.child("client_config");
		if(!node) {
			wxLogError("Could not load clients.xml (syntax error), editor will NOT be able to load any client data files.");
			return;
		}

		for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
			const std::string& childName = as_lower_str(childNode.name());
			if(childName == "otbs") {
				for(pugi::xml_node otbNode = childNode.first_child(); otbNode; otbNode = otbNode.next_sibling()) {
					if(as_lower_str(otbNode.name()) == "otb") {
						loadOTBInfo(otbNode);
					}
				}
			} else if(childName == "clients") {
				for(pugi::xml_node versionNode = childNode.first_child(); versionNode; versionNode = versionNode.next_sibling()) {
					if(as_lower_str(versionNode.name()) == "client") {
						loadVersion(versionNode);
					}
				}
			}
		}

		for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
			if(as_lower_str(childNode.name()) != "clients") {
				continue;
			}

			for(pugi::xml_node versionNode = childNode.first_child(); versionNode; versionNode = versionNode.next_sibling()) {
				if(as_lower_str(versionNode.name()) == "client") {
					loadVersionExtensions(versionNode);
				}
			}
		}
	}

	// Assign a default if there isn't one.
	if(!latest_version && !client_versions.empty()) {
		latest_version = client_versions.begin()->second;
	}

	// Load the data directory info
	try
	{
		json::mValue read_obj;
		json::read_or_throw(g_settings.getString(Config::ASSETS_DATA_DIRS), read_obj);
		json::mArray& vers_obj = read_obj.get_array();
		for(json::mArray::iterator ver_iter = vers_obj.begin(); ver_iter != vers_obj.end(); ++ver_iter) {
			json::mObject& ver_obj = ver_iter->get_obj();
			ClientVersion* version = get(ver_obj["id"].get_str());
			if(version == nullptr)
				continue;
			version->setClientPath(wxstr(ver_obj["path"].get_str()));
		}
	}
	catch (std::runtime_error&)
	{
		// pass
		;
	}
}

void ClientVersion::unloadVersions()
{
	for(VersionMap::iterator it = client_versions.begin(); it != client_versions.end(); ++it)
		delete it->second;
	client_versions.clear();
	latest_version = nullptr;
	otb_versions.clear();

}

void ClientVersion::loadOTBInfo(pugi::xml_node otbNode)
{
	if(as_lower_str(otbNode.name()) != "otb") {
		return;
	}

	pugi::xml_attribute attribute;
	if(!(attribute = otbNode.attribute("client"))) {
		wxLogError("Node 'otb' must contain 'client' tag.");
		return;
	}

	OtbVersion otb = {"", OTB_VERSION_3, CLIENT_VERSION_NONE};
	otb.name = attribute.as_string();
	if(!(attribute = otbNode.attribute("id"))) {
		wxLogError("Node 'otb' must contain 'id' tag.");
		return;
	}

	otb.id = static_cast<ClientVersionID>(attribute.as_int());
	if(!(attribute = otbNode.attribute("version"))) {
		wxLogError("Node 'otb' must contain 'version' tag.");
		return;
	}

	OtbFormatVersion versionId = static_cast<OtbFormatVersion>(attribute.as_uint());
	if(versionId < OTB_VERSION_1 || versionId > OTB_VERSION_3) {
		wxLogError("Node 'otb' unrecognized format version (version 1..3 supported).");
		return;
	}

	otb.format_version = versionId;
	otb_versions[otb.name] = otb;
}

void ClientVersion::loadVersion(pugi::xml_node versionNode)
{
	pugi::xml_attribute attribute;
	if(!(attribute = versionNode.attribute("name"))) {
		wxLogError("Node 'client' must contain 'name', 'data_directory' and 'otb' tags.");
		return;
	}

	const std::string& versionName = attribute.as_string();
	if(!(attribute = versionNode.attribute("data_directory"))) {
		wxLogError("Node 'client' must contain 'name', 'data_directory' and 'otb' tags.");
		return;
	}

	const std::string& dataPath = attribute.as_string();
	if(!(attribute = versionNode.attribute("otb"))) {
		wxLogError("Node 'client' must contain 'name', 'data_directory' and 'otb' tags.");
		return;
	}

	const std::string& otbVersionName = attribute.as_string();
	if(otb_versions.find(otbVersionName) == otb_versions.end()) {
		wxLogError("Node 'client' 'otb' tag is invalid (couldn't find this otb version).");
		return;
	}

	ClientVersion* version = newd ClientVersion(otb_versions[otbVersionName], versionName, wxstr(dataPath));

	bool should_be_default = versionNode.attribute("default").as_bool();
	version->visible = versionNode.attribute("visible").as_bool();

	for(pugi::xml_node childNode = versionNode.first_child(); childNode; childNode = childNode.next_sibling()) {
		const std::string& childName = as_lower_str(childNode.name());
		if(childName == "otbm") {
			if(!(attribute = childNode.attribute("version"))) {
				wxLogError("Node 'otbm' missing version.");
				continue;
			}

			int32_t otbmVersion = attribute.as_int() - 1;
			if(otbmVersion < MAP_OTBM_1 || otbmVersion > MAP_OTBM_4) {
				wxLogError("Node 'otbm' unsupported version.");
				continue;
			}

			if(childNode.attribute("preffered").as_bool() || version->preferred_map_version == MAP_OTBM_UNKNOWN) {
				version->preferred_map_version = static_cast<MapVersionID>(otbmVersion);
			}
			version->map_versions_supported.push_back(version->preferred_map_version);
		} else if(childName == "fucked_up_charges") {
			version->usesFuckedUpCharges = true;
		} else if(childName == "data") {

			if(!(attribute = childNode.attribute("format"))) {
				wxLogError("Node 'data' does not have 'format' tag.");
				continue;
			}

			const std::string& format = attribute.as_string();
			ClientData client_data = { DAT_FORMAT_74, 0, 0 };
			if(format == "7.4") {
				client_data.datFormat = DAT_FORMAT_74;
			} else if(format == "7.55") {
				client_data.datFormat = DAT_FORMAT_755;
			} else if(format == "7.8") {
				client_data.datFormat = DAT_FORMAT_78;
			} else if(format == "8.6") {
				client_data.datFormat = DAT_FORMAT_86;
			} else if(format == "9.6") {
				client_data.datFormat = DAT_FORMAT_96;
			} else if(format == "10.10") {
				client_data.datFormat = DAT_FORMAT_1010;
			} else if(format == "10.50") {
				client_data.datFormat = DAT_FORMAT_1050;
			} else if(format == "10.57") {
				client_data.datFormat = DAT_FORMAT_1057;
			} else {
				wxLogError("Node 'data' 'format' is invalid (7.4, 7.55, 7.8, 8.6, 9.6, 10.10, 10.50, 10.57 are supported)");
				continue;
			}

			if(!(attribute = childNode.attribute("dat")) || !wxString(attribute.as_string(), wxConvUTF8).ToULong((unsigned long*)&client_data.datSignature, 16)) {
				wxLogError("Node 'data' 'dat' tag is not hex-formatted.");
				continue;
			}

			if(!(attribute = childNode.attribute("spr")) || !wxString(attribute.as_string(), wxConvUTF8).ToULong((unsigned long*)&client_data.sprSignature, 16)) {
				wxLogError("Node 'data' 'spr' tag is not hex-formatted.");
				continue;
			}

			version->data_versions.push_back(client_data);
		}
	}

	if(client_versions[version->getID()]) {
		wxLogError("Duplicate version id %i, discarding version '%s'.", version->getID(), version->name);
		delete version;
		return;
	}

	client_versions[version->getID()] = version;
	if(should_be_default) {
		latest_version = version;
	}
}

void ClientVersion::loadVersionExtensions(pugi::xml_node versionNode)
{
	pugi::xml_attribute attribute;
	if(!(attribute = versionNode.attribute("name"))) {
		// Error has already been displayed earlier, no need to show another error about the same thing
		return;
	}

	ClientVersion* version = get(attribute.as_string());
	if(!version) {
		// Same rationale as above
		return;
	}

	for(pugi::xml_node childNode = versionNode.first_child(); childNode; childNode = childNode.next_sibling()) {
		if(as_lower_str(childNode.name()) != "extensions") {
			continue;
		}

		const std::string& from = childNode.attribute("from").as_string();
		const std::string& to = childNode.attribute("to").as_string();

		ClientVersion* fromVersion = get(from);
		ClientVersion* toVersion = get(to);

		if(!fromVersion && !toVersion) {
			wxLogError("Unknown client extension data.");
			continue;
		}

		if(!fromVersion) {
			fromVersion = client_versions.begin()->second;
		}

		if(!toVersion) {
			toVersion = client_versions.rbegin()->second;
		}

		for(const auto& versionEntry : client_versions) {
			ClientVersion* version = versionEntry.second;
			if(version->getID() >= fromVersion->getID() && version->getID() <= toVersion->getID()) {
				version->extension_versions.push_back(version);
			}
		}

		std::sort(version->extension_versions.begin(), version->extension_versions.end(), VersionComparisonPredicate);
	}
}

void ClientVersion::saveVersions()
{
	json::Array vers_obj;

	for(VersionMap::iterator i = client_versions.begin(); i != client_versions.end(); ++i) {
		ClientVersion* version = i->second;
		json::Object ver_obj;
		ver_obj.push_back(json::Pair("id", version->getName()));
		ver_obj.push_back(json::Pair("path", nstr(version->getClientPath().GetFullPath())));
		vers_obj.push_back(ver_obj);
	}
	std::ostringstream out;
	json::write(vers_obj, out);
	g_settings.setString(Config::ASSETS_DATA_DIRS, out.str());
}

// Client version class

ClientVersion::ClientVersion(OtbVersion otb, std::string versionName, wxString path) :
	otb(otb),
	name(versionName),
	visible(false),
	preferred_map_version(MAP_OTBM_UNKNOWN),
	data_path(path)
{
	////
}

ClientVersion* ClientVersion::get(ClientVersionID id)
{
	VersionMap::iterator i = client_versions.find(id);
	if(i == client_versions.end())
		return nullptr;
	return i->second;
}

ClientVersion* ClientVersion::get(std::string id)
{
	for(VersionMap::iterator i = client_versions.begin(); i != client_versions.end(); ++i)
		if(i->second->getName() == id)
			return i->second;
	return nullptr;
}

ClientVersionList ClientVersion::getAll()
{
	ClientVersionList l;
	for(VersionMap::iterator i = client_versions.begin(); i != client_versions.end(); ++i)
		l.push_back(i->second);
	return l;
}

ClientVersionList ClientVersion::getAllVisible()
{
	ClientVersionList l;
	for(VersionMap::iterator i = client_versions.begin(); i != client_versions.end(); ++i)
		if(i->second->isVisible())
			l.push_back(i->second);
	return l;
}

ClientVersionList ClientVersion::getAllForOTBMVersion(MapVersionID id)
{
	ClientVersionList list;
	for(VersionMap::iterator i = client_versions.begin(); i != client_versions.end(); ++i) {
		if(i->second->isVisible()) {
			for(std::vector<MapVersionID>::iterator v = i->second->map_versions_supported.begin(); v != i->second->map_versions_supported.end(); ++v) {
				if(*v == id)
					list.push_back(i->second);
			}
		}
	}
	return list;
}

ClientVersion* ClientVersion::getLatestVersion()
{
	return latest_version;
}

FileName ClientVersion::getDataPath() const
{
	wxString basePath = g_gui.GetDataDirectory();
	if(!wxFileName(basePath).DirExists())
		basePath = g_gui.getFoundDataDirectory();
	return basePath + data_path + FileName::GetPathSeparator();
}

FileName ClientVersion::getLocalDataPath() const
{
	FileName f = g_gui.GetLocalDataDirectory() + data_path + FileName::GetPathSeparator();
	f.Mkdir(0755, wxPATH_MKDIR_FULL);
	return f;
}

bool ClientVersion::hasValidPaths()
{
	if(!client_path.DirExists()) {
		return false;
	}

	wxDir dir(client_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR));
	wxString otfi_file;
	metadata_path = wxFileName(client_path.GetFullPath(), wxString(ASSETS_NAME) + ".dat");
	sprites_path = wxFileName(client_path.GetFullPath(), wxString(ASSETS_NAME) + ".spr");

	if(dir.GetFirst(&otfi_file, "*.otfi", wxDIR_FILES)) {
		wxFileName otfi(client_path.GetFullPath(), otfi_file);
		OTMLDocumentPtr doc = OTMLDocument::parse(otfi.GetFullPath().ToStdString());
		if(doc->size() != 0 && doc->hasChildAt("DatSpr")) {
			OTMLNodePtr node = doc->get("DatSpr");
			std::string metadata = node->valueAt<std::string>("metadata-file", std::string(ASSETS_NAME) + ".dat");
			std::string sprites = node->valueAt<std::string>("sprites-file", std::string(ASSETS_NAME) + ".spr");
			metadata_path = wxFileName(client_path.GetFullPath(), wxString(metadata));
			sprites_path = wxFileName(client_path.GetFullPath(), wxString(sprites));
		}
	}

	if(!metadata_path.FileExists() || !sprites_path.FileExists()) {
		return false;
	}

	if(!g_settings.getInteger(Config::CHECK_SIGNATURES)) {
		return true;
	}

	// Peek the version of the files
	FileReadHandle dat_file(static_cast<const char*>(metadata_path.GetFullPath().mb_str()));
	if(!dat_file.isOk()) {
		wxLogError("Could not open metadata file.");
		return false;
	}

	uint32_t datSignature;
	dat_file.getU32(datSignature);
	dat_file.close();

	FileReadHandle spr_file(static_cast<const char*>(sprites_path.GetFullPath().mb_str()));
	if(!spr_file.isOk()) {
		wxLogError("Could not open sprites file.");
		return false;
	}

	uint32_t sprSignature;
	spr_file.getU32(sprSignature);
	spr_file.close();

	for(const auto& clientData : data_versions) {
		if(clientData.sprSignature == sprSignature && clientData.datSignature == datSignature) {
			return true;
		}
	}

	wxString message = "Signatures are incorrect.\n";
	message << "Metadata signature: %X\n";
	message << "Sprites signature: %X";
	wxLogError(wxString::Format(message, datSignature, sprSignature));
	return false;
}

bool ClientVersion::loadValidPaths()
{
	while(!hasValidPaths()) {
		wxString message = "Could not locate metadata and/or sprite files, please navigate to your client assets %s installation folder.\n";
		message << "Attempted metadata file: %s\n";
		message << "Attempted sprites file: %s\n";

		g_gui.PopupDialog("Error", wxString::Format(message, name, metadata_path.GetFullPath(), sprites_path.GetFullPath()), wxOK);

		wxString dirHelpText("Select assets directory.");
		wxDirDialog file_dlg(nullptr, dirHelpText, "", wxDD_DIR_MUST_EXIST);
		int ok = file_dlg.ShowModal();
		if(ok == wxID_CANCEL)
			return false;

		client_path.Assign(file_dlg.GetPath() + FileName::GetPathSeparator());
	}

	ClientVersion::saveVersions();

	return true;
}

DatFormat ClientVersion::getDatFormatForSignature(uint32_t signature) const
{
	for(std::vector<ClientData>::const_iterator iter = data_versions.begin(); iter != data_versions.end(); ++iter) {
		if(iter->datSignature == signature)
			return iter->datFormat;
	}

	return DAT_FORMAT_UNKNOWN;
}

std::string ClientVersion::getName() const
{
	return name;
}

ClientVersionID ClientVersion::getID() const
{
	return otb.id;
}

bool ClientVersion::isVisible() const
{
	return visible;
}

void ClientVersion::setClientPath(const FileName& dir)
{
	client_path.Assign(dir);
}

MapVersionID ClientVersion::getPrefferedMapVersionID() const
{
	return preferred_map_version;
}

OtbVersion ClientVersion::getOTBVersion() const
{
	return otb;
}

ClientVersionList ClientVersion::getExtensionsSupported() const
{
	return extension_versions;
}

ClientVersionList ClientVersion::getAllVersionsSupportedForClientVersion(ClientVersion* clientVersion)
{
	ClientVersionList versionList;
	for(const auto& versionEntry : client_versions) {
		ClientVersion* version = versionEntry.second;
		for(ClientVersion* checkVersion : version->getExtensionsSupported()) {
			if(clientVersion == checkVersion) {
				versionList.push_back(version);
			}
		}
	}
	std::sort(versionList.begin(), versionList.end(), VersionComparisonPredicate);
	return versionList;
}
