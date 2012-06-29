//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/client_version.hpp $
// $Id: client_version.hpp 315 2010-02-28 15:30:22Z admin $

#include "main.h"

#include "settings.h"
#include "filehandle.h"

#include "gui.h"

#include "client_version.h"

// Static methods to load/save

ClientVersion::VersionMap ClientVersion::versions;

void ClientVersion::loadVersions()
{
	versions.clear();

	addVersion(ClientVersion(CLIENT_VERSION_740, wxT("7.40"), wxT("740")));
	addVersion(ClientVersion(CLIENT_VERSION_760, wxT("7.60"), wxT("760")));
	addVersion(ClientVersion(CLIENT_VERSION_800, wxT("8.00"), wxT("800")));
	addVersion(ClientVersion(CLIENT_VERSION_810, wxT("8.10"), wxT("810")));
	addVersion(ClientVersion(CLIENT_VERSION_820, wxT("8.20 - 8.31"), wxT("820")));
	addVersion(ClientVersion(CLIENT_VERSION_840, wxT("8.40"), wxT("840")));
	addVersion(ClientVersion(CLIENT_VERSION_850, wxT("8.50"), wxT("850")));
	addVersion(ClientVersion(CLIENT_VERSION_854_BAD, wxT("8.54 (bad)"), wxT("854")));
	addVersion(ClientVersion(CLIENT_VERSION_854, wxT("8.54"), wxT("854")));
	addVersion(ClientVersion(CLIENT_VERSION_860_OLD, wxT("8.60 (old)"), wxT("860")));
	addVersion(ClientVersion(CLIENT_VERSION_860, wxT("8.60"), wxT("860")));
	addVersion(ClientVersion(CLIENT_VERSION_870, wxT("8.70"), wxT("870")));
	addVersion(ClientVersion(CLIENT_VERSION_910, wxT("9.10"), wxT("910")));
	addVersion(ClientVersion(CLIENT_VERSION_920, wxT("9.20 - 9.31"), wxT("920")));

	wxFileName fn;
	fn.Assign(gui.GetLocalDirectory());
	fn.SetFullName(wxT("clients.xml"));
	if(!fn.FileExists())
	{
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

	xmlDocPtr doc = xmlParseFile(fn.GetFullPath().mb_str());

	if(doc)
	{
		xmlNodePtr root = xmlDocGetRootElement(doc);

		if(xmlStrcmp(root->name,(const xmlChar*)"clients") != 0)
		{
			xmlFreeDoc(doc);
			wxLogError(wxT("Could not load clients.xml (syntax error), editor will NOT be able to load any client data files."));
			return;
		}

		
		for(xmlNodePtr versionNode = root->children; versionNode != NULL; versionNode = versionNode->next)
		{
			if(xmlStrcmp(versionNode ->name,(const xmlChar*)"client") == 0)
			{
				std::string versionName;
				readXMLString(versionNode, "version", versionName);
				ClientVersion* version = const_cast<ClientVersion*>(get(versionName));
				
				if(!version)
					continue;

				std::string hidden;
				readXMLString(versionNode, "hidden", hidden);
				version->setHidden(isTrueString(hidden));

				for(xmlNodePtr dataNode = versionNode->children; dataNode != NULL; dataNode = dataNode->next)
				{
					if(xmlStrcmp(dataNode ->name,(const xmlChar*)"data") == 0)
					{
						std::string id;
						wxString xc;
						long datID, sprID;

						readXMLString(dataNode, "dat", id);
						xc = wxstr(id);
						xc.ToLong(&datID, 16);

						readXMLString(dataNode, "spr", id);
						xc = wxstr(id);
						xc.ToLong(&sprID, 16);

						version->version_id_list.push_back(std::make_pair(datID, sprID));
					}
				}
			}
		}
	}

	// TODO LOAD
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

void ClientVersion::saveVersions()
{
	json::Array vers_obj;

	for(VersionMap::iterator i = versions.begin(); i != versions.end(); ++i)
	{
		ClientVersion* version = &i->second;
		json::Object ver_obj;
		ver_obj.push_back(json::Pair("id", version->getName()));
		ver_obj.push_back(json::Pair("path", nstr(version->getClientPath().GetFullPath())));
		vers_obj.push_back(ver_obj);
	}
	std::ostringstream out;
	json::write(vers_obj, out);
	settings.setString(Config::TIBIA_DATA_DIRS, out.str());
}

void ClientVersion::addVersion(const ClientVersion& ver) {
	ASSERT(ver.ver_id != CLIENT_VERSION_NONE);
	versions[ver.ver_id] = ver;
}

// Client version class

ClientVersion::ClientVersion(ClientVersionID id, wxString versionName, wxString path, bool hidden) :
	ver_id(id),
	hidden(hidden),
	name(versionName),
	data_path(path)
{
}

ClientVersion::ClientVersion(const ClientVersion& other) :
	ver_id(other.ver_id),
	name(other.name),
	version_id_list(other.version_id_list),
	data_path(other.data_path),
	client_path(other.client_path)
{
}

ClientVersion* ClientVersion::get(ClientVersionID id)
{
	VersionMap::iterator i = versions.find(id);
	if(i == versions.end())
		return NULL;
	return &i->second;
}

ClientVersion* ClientVersion::get(std::string id)
{
	for(VersionMap::iterator i = versions.begin(); i != versions.end(); ++i)
		if(i->second.getName() == id)
			return &i->second;
	return NULL;
}

ClientVersionList ClientVersion::getAll()
{
	ClientVersionList l;
	for(VersionMap::iterator i = versions.begin(); i != versions.end(); ++i)
		l.push_back(&i->second);
	return l;
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

	uint32_t datVersionR;
	dat_file.getU32(datVersionR);
	
	dat_file.close();

	FileReadHandle spr_file(nstr(spr_path.GetFullPath()));

	if(spr_file.isOk() == false)
		return false;

	uint32_t sprVersionR;
	spr_file.getU32(sprVersionR);
	
	spr_file.close();

	for(std::vector<std::pair<uint32_t, uint32_t> >::const_iterator iter = version_id_list.begin(); iter != version_id_list.end(); ++iter)
	{
		if(iter->first == datVersionR && iter->second == sprVersionR)
		{
			return true;
		}
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
	return ver_id;
}

void ClientVersion::setHidden(bool hidden)
{
	this->hidden = hidden;
}

bool ClientVersion::isHidden() const
{
	return hidden;
}

FileName ClientVersion::getClientPath() const
{
	return client_path;
}

void ClientVersion::setClientPath(const FileName& dir)
{
	client_path.Assign(dir);
}

MapVersionID ClientVersion::getMapVersionID() const
{
	switch(ver_id)
	{
		case CLIENT_VERSION_740:
		//case CLIENT_VERSION_750:
		case CLIENT_VERSION_755:
		case CLIENT_VERSION_760:
		//case CLIENT_VERSION_770:
		case CLIENT_VERSION_780:
		case CLIENT_VERSION_790:
		case CLIENT_VERSION_792:
			return MAP_OTBM_1;
		case CLIENT_VERSION_800:
		case CLIENT_VERSION_810:
		case CLIENT_VERSION_811:
		case CLIENT_VERSION_820:
		case CLIENT_VERSION_830:
			return MAP_OTBM_2;
		case CLIENT_VERSION_840:
		case CLIENT_VERSION_841:
		case CLIENT_VERSION_842:
		case CLIENT_VERSION_850:
		case CLIENT_VERSION_854_BAD:
		case CLIENT_VERSION_854:
		case CLIENT_VERSION_855:
		case CLIENT_VERSION_860_OLD:
		case CLIENT_VERSION_860:
		case CLIENT_VERSION_861:
		case CLIENT_VERSION_862:
		case CLIENT_VERSION_870:
		case CLIENT_VERSION_871:
		case CLIENT_VERSION_872:
		case CLIENT_VERSION_873:
		// case CLIENT_VERSION_870: // same as 873
		case CLIENT_VERSION_900:
		case CLIENT_VERSION_910:
		case CLIENT_VERSION_920:
		default:
			// OTServ 0.7.0 is not default yet for 8.50+
			return MAP_OTBM_3;
	}
}
