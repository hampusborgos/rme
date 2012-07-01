//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "settings.h"
#include "filehandle.h"

#include "gui.h"

#include "client_version.h"

// Static methods to load/save

ClientVersion::VersionMap ClientVersion::client_versions;
ClientVersion* ClientVersion::latest_version = NULL;
ClientVersion::OtbMap ClientVersion::otb_versions;

void ClientVersion::loadVersions()
{
	// Clean up old stuff
	for (VersionMap::iterator it = client_versions.begin(); it != client_versions.end(); ++it)
		delete it->second;
	client_versions.clear();
	otb_versions.clear();

	// Locate the clients.xml file
	wxFileName fn;
	fn.Assign(gui.GetLocalDirectory());
	fn.SetFullName(wxT("clients.xml"));
	if(!fn.FileExists())
	{
		// Try alternate filepath
		wxFileName fn2;
		fn2.Assign(gui.GetExecDirectory());
		fn2.SetFullName(wxT("clients.xml"));
		if(!fn2.FileExists()){
			wxLogError(wxString() +
				wxT("Could not load clients.xml, editor will NOT be able to load any client data files.\n") +
				wxT("Checked paths:\n") +
				fn.GetFullPath() + wxT("\n") +
				fn2.GetFullPath()
			);
			return;
		}
		fn = fn2;
	}

	// Parse the file
	xmlDocPtr doc = xmlParseFile(fn.GetFullPath().mb_str());
	if(doc)
	{
		xmlNodePtr root = xmlDocGetRootElement(doc);

		if(xmlStrcmp(root->name,(const xmlChar*)"client_config") != 0)
		{
			xmlFreeDoc(doc);
			wxLogError(wxT("Could not load clients.xml (syntax error), editor will NOT be able to load any client data files."));
			return;
		}

		
		for(xmlNodePtr childNode = root->children; childNode != NULL; childNode = childNode->next)
		{
			if(xmlStrcmp(childNode->name,(const xmlChar*)"otbs") == 0)
				loadOTBs(childNode);
			else if(xmlStrcmp(childNode->name,(const xmlChar*)"clients") == 0)
				for(xmlNodePtr versionNode = childNode->children; versionNode != NULL; versionNode = versionNode->next)
					if(xmlStrcmp(versionNode->name,(const xmlChar*)"client") == 0)
						loadVersion(versionNode);
		}
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
			if (version == NULL)
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

void ClientVersion::loadOTBs(xmlNodePtr otbsNode)
{
	for(xmlNodePtr otbNode = otbsNode->children; otbNode != NULL; otbNode = otbNode->next)
	{
		if(xmlStrcmp(otbNode->name,(const xmlChar*)"otb") == 0)
		{
			OtbVersion otb = {"", OTB_VERSION_3, CLIENT_VERSION_NONE};
			if(!readXMLString(otbNode, "client", otb.name))
			{
				wxLogError(wxT("Node 'otb' must contain 'client' tag."));
				continue;
			}

			if(!readXMLInteger(otbNode, "id", otb.id))
			{
				wxLogError(wxT("Node 'otb' must contain 'id' tag."));
				continue;
			}
			int read_int;
			if(!readXMLInteger(otbNode, "version", read_int))
			{
				wxLogError(wxT("Node 'otb' must contain 'version' tag."));
				continue;
			}

			if (read_int < OTB_VERSION_1 || read_int > OTB_VERSION_3)
			{
				wxLogError(wxT("Node 'otb' unrecognized format version (version 1..3 supported)."));
				continue;
			}

			otb.format_version = (OtbFormatVersion)read_int;
			otb_versions[otb.name] = otb;
		}
	}
}

void ClientVersion::loadVersion(xmlNodePtr versionNode)
{
	std::string versionName;
	std::string otbVersionName;
	std::string dataPath;

	if (!readXMLString(versionNode, "name", versionName) ||
		!readXMLString(versionNode, "data_directory", dataPath) ||
		!readXMLString(versionNode, "otb", otbVersionName))
	{
		wxLogError(wxT("Node 'client' must contain 'name', 'data_directory' and 'otb' tags."));
		return;
	}

	if (otb_versions.find(otbVersionName) == otb_versions.end())
	{
		wxLogError(wxT("Node 'client' 'otb' tag is invalid (couldn't find this otb version)."));
		return;
	}

	ClientVersion* version = newd ClientVersion(otb_versions[otbVersionName], wxstr(versionName), dataPath);
	
	readXMLBoolean(versionNode, "visible", version->visible);

	for(xmlNodePtr childNode = versionNode->children; childNode != NULL; childNode = childNode->next)
	{
		if(xmlStrcmp(childNode->name,(const xmlChar*)"otbm") == 0)
		{
			int otbmVersion;
			if (!readXMLInteger(childNode, "version", otbmVersion))
			{
				wxLogError(wxT("Node 'otbm' missing version."));
				continue;
			}
			otbmVersion -= 1;

			if (otbmVersion < MAP_OTBM_1 || otbmVersion > MAP_OTBM_4)
			{
				wxLogError(wxT("Node 'otbm' unsupported version."));
				continue;
			}
			
			bool preferred = false;
			if (readXMLBoolean(childNode, "preffered", preferred) && preferred)
				version->preferred_map_version = (MapVersionID)otbmVersion;
			if (version->preferred_map_version == MAP_OTBM_UNKNOWN)
				version->preferred_map_version = (MapVersionID)otbmVersion;

			version->map_versions_supported.push_back((MapVersionID)otbmVersion);
		}
		else if(xmlStrcmp(childNode->name,(const xmlChar*)"extensions") == 0)
		{
			;
		}
		else if(xmlStrcmp(childNode->name,(const xmlChar*)"fucked_up_charges") == 0)
		{
			version->usesFuckedUpCharges = true;
		}
		else if(xmlStrcmp(childNode->name,(const xmlChar*)"data") == 0)
		{
			ClientData client_data = {DAT_VERSION_740, SPR_VERSION_700, 0, 0};

			std::string datVersion, sprVersion;

			if (!readXMLString(childNode, "datversion", datVersion) || !readXMLString(childNode, "sprversion", sprVersion))
			{
				wxLogError(wxT("Node 'data' does not have 'datversion' / 'sprversion' tags."));
				continue;
			}

			if (datVersion == "7.4")
				client_data.datVersion = DAT_VERSION_740;
			else if (datVersion == "7.6")
				client_data.datVersion = DAT_VERSION_760;
			else if (datVersion == "8.6")
				client_data.datVersion = DAT_VERSION_860;
			else
			{
				wxLogError(wxT("Node 'data' 'datversion' is invalid (7.4, 7.6 and 8.6 are supported)"));
				continue;
			}

			if (sprVersion == "7.0")
				client_data.sprVersion = SPR_VERSION_700;
			else if (sprVersion == "9.6")
				client_data.sprVersion = SPR_VERSION_960;
			else
			{
				wxLogError(wxT("Node 'data' 'sprversion' is invalid (7.0 and 9.6 are supported)"));
				continue;
			}

			std::string read_string;

			if (!(readXMLString(childNode, "dat", read_string) && wxstr(read_string).ToULong((unsigned long*)&client_data.datSignature, 16)))
			{
				wxLogError(wxT("Node 'data' 'dat' tag is not hex-formatted."));
				continue;
			}
			
			if (!(readXMLString(childNode, "spr", read_string) && wxstr(read_string).ToULong((unsigned long*)&client_data.sprSignature, 16)))
			{
				wxLogError(wxT("Node 'data' 'spr' tag is not hex-formatted."));
				continue;
			}

			version->data_versions.push_back(client_data);
		}
	}

	if (client_versions[version->getID()] != NULL)
	{
		wxString error;
		error << wxT("Duplicate version id ") << version->getID() << wxT(", discarding version '") << version->name << wxT("'.");
		wxLogError(error);
		delete version;
		return;
	}

	client_versions[version->getID()] = version;
	latest_version = version;
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

ClientVersion::ClientVersion(OtbVersion otb, wxString versionName, wxString path) :
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
		return NULL;
	return i->second;
}

ClientVersion* ClientVersion::get(std::string id)
{
	for(VersionMap::iterator i = client_versions.begin(); i != client_versions.end(); ++i)
		if(i->second->getName() == id)
			return i->second;
	return NULL;
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

ClientVersion* ClientVersion::getLatestVersion()
{
	return latest_version;
}

FileName ClientVersion::getDataPath() const
{
	return gui.GetDataDirectory() + data_path + FileName::GetPathSeparator();
}

FileName ClientVersion::getLocalDataPath() const
{
	FileName f = gui.GetLocalDataDirectory() + data_path + FileName::GetPathSeparator();
	f.Mkdir(0755, wxPATH_MKDIR_FULL);
	return f;
}

bool ClientVersion::hasValidPaths() const
{
	if(client_path.DirExists() == false){
		return false;
	}

	
	FileName dat_path = client_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxT("Tibia.dat");
	FileName spr_path = client_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxT("Tibia.spr");

	if(!dat_path.FileExists() || !spr_path.FileExists())
		return false;

	if(!settings.getInteger(Config::CHECK_SIGNATURES))
		return true;

	// Peek the version of the files
	FileReadHandle dat_file(nstr(dat_path.GetFullPath()));

	if(dat_file.isOk() == false)
		return false;

	uint32_t datSignature;
	dat_file.getU32(datSignature);
	
	dat_file.close();

	FileReadHandle spr_file(nstr(spr_path.GetFullPath()));

	if(spr_file.isOk() == false)
		return false;

	uint32_t sprSignature;
	spr_file.getU32(sprSignature);
	
	spr_file.close();

	for(std::vector<ClientData>::const_iterator iter = data_versions.begin(); iter != data_versions.end(); ++iter)
	{
		if(iter->datSignature == datSignature && iter->sprSignature == sprSignature)
			return true;
	}

	return false;
}

bool ClientVersion::loadValidPaths()
{
	while(hasValidPaths() == false)
	{
		gui.PopupDialog(wxT("Error"), wxT("Could not locate Tibia.dat and/or Tibia.spr, please navigate to your Tibia ") + name + wxT(" installation folder."), wxOK);
		wxDirDialog file_dlg(NULL, wxT("Select Tibia directory..."), wxT(""), wxDD_DIR_MUST_EXIST);
		int ok = file_dlg.ShowModal();
		if(ok == wxID_CANCEL)
			return false;

		client_path.Assign(file_dlg.GetPath() + FileName::GetPathSeparator());
	}

	ClientVersion::saveVersions();

	return true;
}

std::string ClientVersion::getName() const
{
	return nstr(name);
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
	return ClientVersionList();
}