//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/process_com.hpp $
// $Id: process_com.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#ifdef _USE_PROCESS_COM

#include "gui.h"
#include "process_com.h"

// Server!

RMEProcessServer::RMEProcessServer()
{
	////
}

RMEProcessServer::~RMEProcessServer()
{
	////
}

wxConnectionBase* RMEProcessServer::OnAcceptConnection(const wxString& topic)
{
	if(topic == "rme_talk") {
		return newd RMEProcessConnection();
	}
	return nullptr;
}

// Client!

RMEProcessClient::RMEProcessClient() : proc(nullptr)
{
	////
}

RMEProcessClient::~RMEProcessClient()
{
	delete proc;
}

wxConnectionBase* RMEProcessClient::OnMakeConnection()
{
	return proc = newd RMEProcessConnection();
}

// Connection

RMEProcessConnection::RMEProcessConnection() : wxConnection()
{
	////
}

RMEProcessConnection::~RMEProcessConnection()
{
	////
}

bool RMEProcessConnection::OnPoke(const wxString& topic, const wxString& item, wxChar* data, int size, wxIPCFormat format)
{
	if(topic == "rme_talk" && item == "map_open") {
		std::string s(reinterpret_cast<char*>(data), size);
		g_gui.LoadMap(FileName(wxString(s.c_str(), wxConvUTF8)));
		return true;
	}
	return false;
}

void RMEProcessConnection::AskToLoad(FileName map)
{
	std::string maps = (const char*)map.GetFullPath().mb_str(wxConvUTF8);
	char* c = newd char[maps.length()];
	memcpy(c, maps.c_str(), maps.length());
	Poke("map_open", reinterpret_cast<wxChar*>(c), maps.length());
	delete[] c;
}

#endif
