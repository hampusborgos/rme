//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_EXTENSION_H_
#define RME_EXTENSION_H_

#include "tileset.h"
#include "client_version.h"

class MaterialsExtension
{
public:
	MaterialsExtension(std::string name, std::string author, std::string description);
	~MaterialsExtension();

	void addVersion(std::string str);
	bool isForVersion(uint16_t ver_id);
	std::string getVersionString();

	std::string name;
	std::string url;
	std::string author;
	std::string author_url;
	std::string description;
	bool for_all_versions;
	ClientVersionList version_list;
private:
	MaterialsExtension(const MaterialsExtension&);
	MaterialsExtension& operator=(const MaterialsExtension&);
};

typedef std::vector<MaterialsExtension*> MaterialsExtensionList;

#endif
