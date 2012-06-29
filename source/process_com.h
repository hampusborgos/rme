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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/process_com.h $
// $Id: process_com.h 310 2010-02-26 18:03:48Z admin $


#ifdef _USE_PROCESS_COM

#ifndef RME_PROCESS_COMMUNICATION_H_
#define RME_PROCESS_COMMUNICATION_H_

#include "wx/ipc.h"

class RMEProcessConnection : public wxConnection {
public:
	RMEProcessConnection();
	~RMEProcessConnection();

	bool OnPoke(const wxString& topic, const wxString& item, wxChar* data, int size, wxIPCFormat format);

	void AskToLoad(FileName map);
};

class RMEProcessServer : public wxServer {
public:
	RMEProcessServer();
	~RMEProcessServer();

	wxConnectionBase* OnAcceptConnection(const wxString& topic);
};

class RMEProcessClient : public wxClient {
	wxConnectionBase* proc;
public:
	RMEProcessClient();
	~RMEProcessClient();

	wxConnectionBase* OnMakeConnection();
};

#endif

#endif
