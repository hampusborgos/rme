//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "settings.h"
#include "filehandle.h"

#include "gui.h"

#include "client_version.h"
#include "pugicast.h"

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
	exec_dir_client_xml.Assign(gui.GetExecDirectory());
	exec_dir_client_xml.SetFullName(wxT("clients.xml"));
	
	wxFileName data_dir_client_xml;
	data_dir_client_xml.Assign(gui.GetDataDirectory());
	data_dir_client_xml.SetFullName(wxT("clients.xml"));

	wxFileName work_dir_client_xml;
	work_dir_client_xml.Assign(gui.getFoundDataDirectory());
	work_dir_client_xml.SetFullName(wxT("clients.xml"));

	file_to_load = exec_dir_client_xml;
	if (!file_to_load.FileExists()) {
		file_to_load = data_dir_client_xml;
		if (!file_to_load.FileExists()) {
			file_to_load = work_dir_client_xml;
			if(!file_to_load.FileExists()) {
				file_to_load.Clear();
			}
		}
	}

	if (!file_to_load.FileExists()) {
		wxLogError(wxString() +
			wxT("Could not load clients.xml, editor will NOT be able to load any client data files.\n") +
			wxT("Checked paths:\n") +
			exec_dir_client_xml.GetFullPath() + "\n" +
			data_dir_client_xml.GetFullPath() + "\n" +
			work_dir_client_xml.GetFullPath()
		);
		return;
	}

	// Parse the file
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(file_to_load.GetFullPath().mb_str());
	if (result) {
		pugi::xml_node node = doc.child("client_config");
		if (!node) {
			wxLogError(wxT("Could not load clients.xml (syntax error), editor will NOT be able to load any client data files."));
			return;
		}

		for (pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
			const std::string& childName = as_lower_str(childNode.name());
			if (childName == "otbs") {
				for (pugi::xml_node otbNode = childNode.first_child(); otbNode; otbNode = otbNode.next_sibling()) {
					if (as_lower_str(otbNode.name()) == "otb") {
						loadOTBInfo(otbNode);
					}
				}
			} else if (childName == "clients") {
				for (pugi::xml_node versionNode = childNode.first_child(); versionNode; versionNode = versionNode.next_sibling()) {
					if (as_lower_str(versionNode.name()) == "client") {
						loadVersion(versionNode);
					}
				}
			}
		}

		for (pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
			if (as_lower_str(childNode.name()) != "clients") {
				continue;
			}

			for (pugi::xml_node versionNode = childNode.first_child(); versionNode; versionNode = versionNode.next_sibling()) {
				if (as_lower_str(versionNode.name()) == "client") {
					loadVersionExtensions(versionNode);
				}
			}
		}
	}

	// Assign a default if there isn't one.
	if (!latest_version && !client_versions.empty()) {
		latest_version = client_versions.begin()->second;
	}

	// Load the data directory info
	try
	{
		json::mValue read_obj;
		json::read_or_throw(settings.getString(Config::TIBIA_DATA_DIRS), read_obj);
		json::mArray& vers_obj = read_obj.get_array();
		for(json::mArray::iterator ver_iter = vers_obj.begin(); ver_iter != vers_obj.end(); ++ver_iter)
		{
			json::mObject& ver_obj = ver_iter->get_obj();
			ClientVersion* version = get(ver_obj["id"].get_str());
			if (version == nullptr)
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
	for (VersionMap::iterator it = client_versions.begin(); it != client_versions.end(); ++it)
		delete it->second;
	client_versions.clear();
	latest_version = nullptr;
	otb_versions.clear();

}

void ClientVersion::loadOTBInfo(pugi::xml_node otbNode)
{
	if (as_lower_str(otbNode.name()) != "otb") {
		return;
	}

	pugi::xml_attribute attribute;
	if (!(attribute = otbNode.attribute("client"))) {
		wxLogError(wxT("Node 'otb' must contain 'client' tag."));
		return;
	}

	OtbVersion otb = {"", OTB_VERSION_3, CLIENT_VERSION_NONE};
	otb.name = attribute.as_string();
	if (!(attribute = otbNode.attribute("id"))) {
		wxLogError(wxT("Node 'otb' must contain 'id' tag."));
		return;
	}

	otb.id = pugi::cast<int32_t>(attribute.value());
	if (!(attribute = otbNode.attribute("version"))) {
		wxLogError(wxT("Node 'otb' must contain 'version' tag."));
		return;
	}

	OtbFormatVersion versionId = static_cast<OtbFormatVersion>(pugi::cast<uint32_t>(attribute.value()));
	if (versionId < OTB_VERSION_1 || versionId > OTB_VERSION_3) {
		wxLogError(wxT("Node 'otb' unrecognized format version (version 1..3 supported)."));
		return;
	}

	otb.format_version = versionId;
	otb_versions[otb.name] = otb;
}

void ClientVersion::loadVersion(pugi::xml_node versionNode)
{
	pugi::xml_attribute attribute;
	if (!(attribute = versionNode.attribute("name"))) {
		wxLogError(wxT("Node 'client' must contain 'name', 'data_directory' and 'otb' tags."));
		return;
	}

	const std::string& versionName = attribute.as_string();
	if (!(attribute = versionNode.attribute("data_directory"))) {
		wxLogError(wxT("Node 'client' must contain 'name', 'data_directory' and 'otb' tags."));
		return;
	}

	const std::string& dataPath = attribute.as_string();
	if (!(attribute = versionNode.attribute("otb"))) {
		wxLogError(wxT("Node 'client' must contain 'name', 'data_directory' and 'otb' tags."));
		return;
	}

	const std::string& otbVersionName = attribute.as_string();
	if (otb_versions.find(otbVersionName) == otb_versions.end()) {
		wxLogError(wxT("Node 'client' 'otb' tag is invalid (couldn't find this otb version)."));
		return;
	}

	ClientVersion* version = newd ClientVersion(otb_versions[otbVersionName], versionName, wxstr(dataPath));

	bool should_be_default = versionNode.attribute("default").as_bool();
	version->visible = versionNode.attribute("visible").as_bool();

	for (pugi::xml_node childNode = versionNode.first_child(); childNode; childNode = childNode.next_sibling()) {
		const std::string& childName = as_lower_str(childNode.name());
		if (childName == "otbm") {
			if (!(attribute = childNode.attribute("version"))) {
				wxLogError(wxT("Node 'otbm' missing version."));
				continue;
			}

			int32_t otbmVersion = pugi::cast<int32_t>(attribute.value()) - 1;
			if (otbmVersion < MAP_OTBM_1 || otbmVersion > MAP_OTBM_4) {
				wxLogError(wxT("Node 'otbm' unsupported version."));
				continue;
			}

			if (childNode.attribute("preffered").as_bool() || version->preferred_map_version == MAP_OTBM_UNKNOWN) {
				version->preferred_map_version = static_cast<MapVersionID>(otbmVersion);
			}
			version->map_versions_supported.push_back(version->preferred_map_version);
		} else if (childName == "fucked_up_charges") {
			version->usesFuckedUpCharges = true;
		} else if (childName == "data") {
			if (!(attribute = childNode.attribute("sprversion"))) {
				wxLogError(wxT("Node 'data' does not have 'datversion' / 'sprversion' tags."));
				continue;
			}

			const std::string& sprVersion = attribute.as_string();
			if (!(attribute = childNode.attribute("datversion"))) {
				wxLogError(wxT("Node 'data' does not have 'datversion' / 'sprversion' tags."));
				continue;
			}

			const std::string& datVersion = attribute.as_string();
			ClientData client_data = {DAT_VERSION_74, SPR_VERSION_70, 0, 0};
			if (datVersion == "7.4") {
				client_data.datVersion = DAT_VERSION_74;
			} else if (datVersion == "7.6") {
				client_data.datVersion = DAT_VERSION_76;
			} else if (datVersion == "7.8") {
				client_data.datVersion = DAT_VERSION_78;
			} else if (datVersion == "8.6") {
				client_data.datVersion = DAT_VERSION_86;
			} else if (datVersion == "9.6") {
				client_data.datVersion = DAT_VERSION_96;
			} else if (datVersion == "10.10") {
				client_data.datVersion = DAT_VERSION_1010;
			} else {
				wxLogError(wxT("Node 'data' 'datversion' is invalid (7.4, 7.6, 7.8, 8.6, 9.6 and 10.10 are supported)"));
				continue;
			}

			if (sprVersion == "7.0") {
				client_data.sprVersion = SPR_VERSION_70;
			} else if (sprVersion == "9.6") {
				client_data.sprVersion = SPR_VERSION_96;
			} else {
				wxLogError(wxT("Node 'data' 'sprversion' is invalid (7.0 and 9.6 are supported)"));
				continue;
			}

			if (!(attribute = childNode.attribute("dat")) || !wxString(attribute.as_string(), wxConvUTF8).ToULong((unsigned long*)&client_data.datSignature, 16)) {
				wxLogError(wxT("Node 'data' 'dat' tag is not hex-formatted."));
				continue;
			}

			if (!(attribute = childNode.attribute("spr")) || !wxString(attribute.as_string(), wxConvUTF8).ToULong((unsigned long*)&client_data.sprSignature, 16)) {
				wxLogError(wxT("Node 'data' 'spr' tag is not hex-formatted."));
				continue;
			}

			version->data_versions.push_back(client_data);
		}
	}

	if (client_versions[version->getID()]) {
		wxLogError(wxT("Duplicate version id ") + std::to_string(version->getID()) + wxT(", discarding version '") + version->name + wxT("'."));
		delete version;
		return;
	}

	client_versions[version->getID()] = version;
	if (should_be_default) {
		latest_version = version;
	}
}

void ClientVersion::loadVersionExtensions(pugi::xml_node versionNode)
{
	pugi::xml_attribute attribute;
	if (!(attribute = versionNode.attribute("name"))) {
		// Error has already been displayed earlier, no need to show another error about the same thing
		return;
	}

	ClientVersion* version = get(attribute.as_string());
	if (!version) {
		// Same rationale as above
		return;
	}

	for (pugi::xml_node childNode = versionNode.first_child(); childNode; childNode = childNode.next_sibling()) {
		if (as_lower_str(childNode.name()) != "extensions") {
			continue;
		}

		const std::string& from = childNode.attribute("from").as_string();
		const std::string& to = childNode.attribute("to").as_string();
			
		ClientVersion* fromVersion = get(from);
		ClientVersion* toVersion = get(to);

		if (!fromVersion && !toVersion) {
			wxLogError(wxT("Unknown client extension data."));
			continue;
		}
			
		if (!fromVersion) {
			fromVersion = client_versions.begin()->second;
		}
		
		if (!toVersion) {
			toVersion = client_versions.rbegin()->second;
		}

		for (const auto& versionEntry : client_versions) {
			ClientVersion* version = versionEntry.second;
			if (version->getID() >= fromVersion->getID() && version->getID() <= toVersion->getID()) {
				version->extension_versions.push_back(version);
			}
		}

		std::sort(version->extension_versions.begin(), version->extension_versions.end(), VersionComparisonPredicate);
	}
}

void ClientVersion::saveVersions()
{
	json::Array vers_obj;

	for(VersionMap::iterator i = client_versions.begin(); i != client_versions.end(); ++i)
	{
		ClientVersion* version = i->second;
		json::Object ver_obj;
		ver_obj.push_back(json::Pair("id", version->getName()));
		ver_obj.push_back(json::Pair("path", nstr(version->getClientPath().GetFullPath())));
		vers_obj.push_back(ver_obj);
	}
	std::ostringstream out;
	json::write(vers_obj, out);
	settings.setString(Config::TIBIA_DATA_DIRS, out.str());
}

// Client version class

ClientVersion::ClientVersion(OtbVersion otb, std::string versionName, wxString path) :
	otb(otb),
	name(versionName),
	visible(false),
	preferred_map_version(MAP_OTBM_UNKNOWN),
	data_path(path)
{
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
		if (i->second->isVisible())
			l.push_back(i->second);
	return l;
}

ClientVersionList ClientVersion::getAllForOTBMVersion(MapVersionID id)
{
	ClientVersionList l;
	for(VersionMap::iterator i = client_versions.begin(); i != client_versions.end(); ++i)
		if (i->second->isVisible())
			for(std::vector<MapVersionID>::iterator v = i->second->map_versions_supported.begin(); v != i->second->map_versions_supported.end(); ++v)
				if (*v == id)
					l.push_back(i->second);
	return l;
}

ClientVersion* ClientVersion::getLatestVersion()
{
	return latest_version;
}

FileName ClientVersion::getDataPath() const
{
	wxString basePath = gui.GetDataDirectory();
	if(!wxFileName(basePath).DirExists())
		basePath = gui.getFoundDataDirectory();
	return basePath + data_path + FileName::GetPathSeparator();
}

FileName ClientVersion::getLocalDataPath() const
{
	FileName f = gui.GetLocalDataDirectory() + data_path + FileName::GetPathSeparator();
	f.Mkdir(0755, wxPATH_MKDIR_FULL);
	return f;
}

bool ClientVersion::hasValidPaths() const
{
	if (client_path.DirExists() == false) {
		return false;
	}
	
	FileName dat_path = client_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxT("Tibia.dat");
	FileName spr_path = client_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxT("Tibia.spr");
	if (!dat_path.FileExists() || !spr_path.FileExists()) {
		return false;
	}

	if (!settings.getInteger(Config::CHECK_SIGNATURES)) {
		return true;
	}

	// Peek the version of the files
	FileReadHandle dat_file(static_cast<const char*>(dat_path.GetFullPath().mb_str()));
	if (!dat_file.isOk()) {
		wxLogError(wxT("Could not open Tibia.dat."));
		return false;
	}

	uint32_t datSignature;
	dat_file.getU32(datSignature);
	dat_file.close();

	FileReadHandle spr_file(static_cast<const char*>(spr_path.GetFullPath().mb_str()));
	if (!spr_file.isOk()) {
		wxLogError(wxT("Could not open Tibia.spr."));
		return false;
	}

	uint32_t sprSignature;
	spr_file.getU32(sprSignature);
	spr_file.close();

	for (const auto& clientData : data_versions) {
		if (clientData.sprSignature == sprSignature && clientData.datSignature == datSignature) {
			return true;
		}
	}
	wxLogError(wxString::Format(
		wxT("Spr(%d) or Dat(%d) signatures are incorrect."), sprSignature, datSignature
	));
	return false;
}

bool ClientVersion::loadValidPaths()
{
	while(hasValidPaths() == false)
	{
		gui.PopupDialog(
			wxT("Error"),
			wxT("Could not locate Tibia.dat and/or Tibia.spr, please navigate to your Tibia ") +
				name + wxT(" installation folder."),
			wxOK);

		wxString dirHelpText(wxT("Select Tibia "));
		dirHelpText << name << " directory.";

		wxDirDialog file_dlg(nullptr, dirHelpText, wxT(""), wxDD_DIR_MUST_EXIST);
		int ok = file_dlg.ShowModal();
		if(ok == wxID_CANCEL)
			return false;

		client_path.Assign(file_dlg.GetPath() + FileName::GetPathSeparator());
	}

	ClientVersion::saveVersions();

	return true;
}

DatVersion ClientVersion::getDatVersionForSignature(uint32_t signature) const
{
	for(std::vector<ClientData>::const_iterator iter = data_versions.begin(); iter != data_versions.end(); ++iter)
	{
		if(iter->datSignature == signature)
			return iter->datVersion;
	}

	return DAT_VERSION_UNKNOWN;
}

SprVersion ClientVersion::getSprVersionForSignature(uint32_t signature) const
{
	for(std::vector<ClientData>::const_iterator iter = data_versions.begin(); iter != data_versions.end(); ++iter)
	{
		if(iter->sprSignature == signature)
			return iter->sprVersion;
	}

	return SPR_VERSION_UNKNOWN;
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

FileName ClientVersion::getClientPath() const
{
	return client_path;
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
	for (const auto& versionEntry : client_versions) {
		ClientVersion* version = versionEntry.second;
		for (ClientVersion* checkVersion : version->getExtensionsSupported()) {
			if (clientVersion == checkVersion) {
				versionList.push_back(version);
			}
		}
	}
	std::sort(versionList.begin(), versionList.end(), VersionComparisonPredicate);
	return versionList;
}
