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

using json = nlohmann::json;

std::string Assets::version_name;
uint16_t Assets::client_version;
wxString Assets::data_path;
wxString Assets::assets_path;
bool Assets::loaded = false;

void Assets::load()
{
	// Load the data directory info
	try
	{
		auto dataDirs = g_settings.getString(Config::ASSETS_DATA_DIRS);
		if (!dataDirs.empty()) {
			json read_obj = json::parse(dataDirs);
			auto ver_obj = read_obj.at(0).get<json::object_t>();
			auto path = ver_obj.at("path").get<std::string>();
			setPath(wxstr(path));
		}
	}
	catch ([[maybe_unused]]const json::exception& e)
	{
		// pass
	}
}

bool Assets::loadAppearanceProtobuf(wxString& error, wxArrayString& warnings)
{
	using namespace remeres::protobuf::appearances;
	using json = nlohmann::json;
	
	auto clientDirectory = Assets::getPath().ToStdString() + "/";
	auto assetsDirectory = clientDirectory + "/assets/";
	//const std::string& catalogContentFile = assetsDirectory + "/catalog-content.json";
	//spdlog::info("assets dir: {}, catalog: {}", fullAssetsDir.ToStdString(), catalogContentFile);

	if (!g_spriteAppearances.loadCatalogContent(assetsDirectory, false)) {
		spdlog::error("[GUI::loadAppearanceProtobuf] - Cannot open catalog content file");
		return false;
	}

	using json = nlohmann::json;
	std::filesystem::path packagesPath = std::filesystem::path(clientDirectory) / std::filesystem::path("package.json");
	if (!std::filesystem::exists(packagesPath)) {
		spdlog::error("package.json is not present in given directory. {}", packagesPath.string().c_str());
		throw std::exception(fmt::format("package.json is not present in given directory: {}", packagesPath.string()).c_str());
		return false;
	}

	std::ifstream file(packagesPath, std::ios::in);
	if (!file.is_open()) {
		spdlog::error("Unable to open packages.json.");
		throw std::exception("Unable to open packages.json.");
		return false;
	}

	json document = json::parse(file, nullptr, false);

	file.close();
	// Save version from package.json
	std::string version = document.at("version").get<std::string>();
	version_name = version;

	const std::string appearanceFileName = g_spriteAppearances.getAppearanceFileName();

	//g_spriteAppearances.setSpritesCount(spritesCount + 1);

	std::fstream fileStream(assetsDirectory + appearanceFileName, std::ios::in | std::ios::binary);
	if (!fileStream.is_open()) {
		error = "Failed to load "+ appearanceFileName +", file cannot be oppened";
		spdlog::error("[GUI::loadAppearanceProtobuf] - Failed to load %s, file cannot be oppened", appearanceFileName);
		fileStream.close();
		return false;
	}

	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	g_gui.appearances = Appearances();
	if (!g_gui.appearances.ParseFromIstream(&fileStream)) {
		error = "Failed to parse binary file "+ appearanceFileName +", file is invalid";
		spdlog::error("[GUI::loadAppearanceProtobuf] - Failed to parse binary file {}, file is invalid", appearanceFileName);
		fileStream.close();
		return false;
	}

	// Parsing all items into ItemType
	bool rt = g_items.loadFromProtobuf(error, warnings, g_gui.appearances);
	if (!rt) {
		error = "Failed to parse item types from protobuf";
		spdlog::error("[GUI::loadAppearanceProtobuf] - Failed to parse item types from protobuf");
		fileStream.close();
		return false;
	}

	// Load looktypes
	for (int i = 0; i < g_gui.appearances.outfit().size(); i++) {
		const auto &outfit = g_gui.appearances.outfit().Get(i);
		if (!g_gui.gfx.loadOutfitSpriteMetadata(outfit, error, warnings)) {
			error = "Failed to parse outfit types from protobuf";
			spdlog::error("[GUI::loadAppearanceProtobuf] - Failed to parse outfit types from protobuf");
			fileStream.close();
			return false;
		}
	}
	fileStream.close();

	// Disposing allocated objects.
	google::protobuf::ShutdownProtobufLibrary();

	// Client loaded
	setLoaded(true);
	return true;
}

void Assets::save()
{
	try {
		json vers_obj;

		json ver_obj;
		ver_obj["id"] = getVersionName();
		wxFileName fileName;
		fileName.Assign(getPath());
		ver_obj["path"] = fileName.GetFullPath().ToStdString();
		auto path = fileName.GetFullPath().ToStdString();
		vers_obj.push_back(ver_obj);

		std::ostringstream out;
		out << vers_obj;
		g_settings.setString(Config::ASSETS_DATA_DIRS, out.str());
	}
	catch ([[maybe_unused]]const json::exception& e) {
		// pass
	}
}

FileName Assets::getDataPath()
{
	wxString basePath = g_gui.GetDataDirectory();
	if(!wxFileName(basePath).DirExists())
		basePath = g_gui.getFoundDataDirectory();
	return basePath + data_path + FileName::GetPathSeparator();
}

FileName Assets::getLocalPath()
{
	FileName f = g_gui.GetLocalDataDirectory() + data_path + FileName::GetPathSeparator();
	f.Mkdir(0755, wxPATH_MKDIR_FULL);
	return f;
}

wxString Assets::getPath()
{
	return assets_path;
}

std::string Assets::getVersionName()
{
	return version_name;
}
