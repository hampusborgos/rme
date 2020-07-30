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

#ifdef _USE_UPDATER_

#ifndef RME_UPDATER_H_
#define RME_UPDATER_H_

#include "threads.h"

extern const wxEventType EVT_UPDATE_CHECK_FINISHED;

#define EVT_ON_UPDATE_CHECK_FINISHED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        EVT_UPDATE_CHECK_FINISHED, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) nullptr \
    ),


class wxURL;

class UpdateConnectionThread : public DetachedThread {
public:
	UpdateConnectionThread(wxEvtHandler* receiver, wxURL* url);
	virtual ~UpdateConnectionThread();
protected:
	virtual ExitCode Entry();
	wxEvtHandler* receiver;
	wxURL* url;
};

class UpdateChecker {
public:
	UpdateChecker();
	~UpdateChecker();

	void connect(wxEvtHandler* receiver);
};

#endif

#endif

