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


//Server

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
	if(topic.Lower() == "rme_talk") {
		g_gui.root->Iconize(false);
		g_gui.root->Raise();
		return newd RMEProcessConnection();
	}
	return nullptr;
}


//Client

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


//Connection

RMEProcessConnection::RMEProcessConnection() : wxConnection()
{
	////
}

RMEProcessConnection::~RMEProcessConnection()
{
	////
}

bool RMEProcessConnection::OnExec(const wxString& topic, const wxString& fileName)
{
	if(topic.Lower() == "rme_talk" && fileName != wxEmptyString) {
		g_gui.LoadMap(FileName(fileName));
		return true;
	}
	return false;
}

#endif
