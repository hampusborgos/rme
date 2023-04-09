//////////////////////////////////////////////////////////////////////
// This file is part of Canary Map Editor
//////////////////////////////////////////////////////////////////////
// Canary Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Canary Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_APPLICATION_H_
#define RME_APPLICATION_H_

#include "gui.h"
#include "main_toolbar.h"
#include "action.h"
#include "settings.h"

#include "process_com.h"
#include "map_display.h"
#include "welcome_dialog.h"

class Item;
class Monster;
class Npc;

class MainFrame;
class MapWindow;
class wxEventLoopBase;
class wxSingleInstanceChecker;

class Application : public wxApp
{
public:
	~Application();
	virtual bool OnInit();
	virtual void OnEventLoopEnter(wxEventLoopBase* loop);
	virtual void MacOpenFiles(const wxArrayString& fileNames);
	virtual int OnExit();
	void Unload();

private:
	bool m_startup;
	wxString m_file_to_open;
	bool ParseCommandLineMap(wxString& fileName);

	virtual void OnFatalException();

#ifdef _USE_PROCESS_COM
	RMEProcessServer* m_proc_server;
	wxSingleInstanceChecker* m_single_instance_checker;
#endif

};

class MainMenuBar;

class MainFrame : public wxFrame
{
public:
	MainFrame(const wxString& title,
		const wxPoint& pos, const wxSize& size);
	~MainFrame();

	void UpdateMenubar();
	bool DoQueryClose();
	bool DoQuerySave(bool doclose = true);
	bool DoQueryImportCreatures();
	bool LoadMap(FileName name);

	void AddRecentFile(const FileName& file);
	void LoadRecentFiles();
	void SaveRecentFiles();
	std::vector<wxString> GetRecentFiles();

	MainToolBar* GetAuiToolBar() const { return tool_bar; }

	void OnUpdateMenus(wxCommandEvent& event);
	void UpdateFloorMenu();
	void OnIdle(wxIdleEvent& event);
	void OnExit(wxCloseEvent& event);

#ifdef _USE_UPDATER_
	void OnUpdateReceived(wxCommandEvent& event);
#endif

#ifdef __WINDOWS__
	virtual bool MSWTranslateMessage(WXMSG *msg);
#endif

	void PrepareDC(wxDC& dc);
protected:
	MainMenuBar* menu_bar;
	MainToolBar* tool_bar;

	friend class Application;
	friend class GUI;

	DECLARE_EVENT_TABLE()
};

#endif
