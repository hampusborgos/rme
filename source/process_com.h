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


#ifdef _USE_PROCESS_COM

#ifndef RME_PROCESS_COMMUNICATION_H_
#define RME_PROCESS_COMMUNICATION_H_

#include "wx/ipc.h"

class RMEProcessConnection : public wxConnection {
public:
	RMEProcessConnection();
	~RMEProcessConnection();

	bool OnExec(const wxString& topic, const wxString& fileName);
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
