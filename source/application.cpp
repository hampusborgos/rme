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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/application.hpp $
// $Id: application.hpp 320 2010-03-08 16:38:07Z admin $

#include "main.h"

#include "application.h"
#include "sprites.h"
#include "editor.h"
#include "common_windows.h"
#include "palette_window.h"
#include "preferences.h"
#include "result_window.h"
#include "minimap_window.h"
#include "about_window.h"
#include "main_menubar.h"
#include "updater.h"

#include "materials.h"
#include "map.h"
#include "complexitem.h"
#include "creature.h"

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_CLOSE(MainFrame::OnExit)

	// Update check complete
#ifdef _USE_UPDATER_
	EVT_ON_UPDATE_CHECK_FINISHED(wxID_ANY, MainFrame::OnUpdateReceived)
#endif
	EVT_ON_UPDATE_MENUS(wxID_ANY, MainFrame::OnUpdateMenus)

	// Idle event handler
	EVT_IDLE(MainFrame::OnIdle)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MapWindow, wxPanel)
	EVT_SIZE(MapWindow::OnSize)

	EVT_COMMAND_SCROLL_TOP       (MAP_WINDOW_HSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_BOTTOM    (MAP_WINDOW_HSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_THUMBTRACK(MAP_WINDOW_HSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_LINEUP    (MAP_WINDOW_HSCROLL, MapWindow::OnScrollLineUp)
	EVT_COMMAND_SCROLL_LINEDOWN  (MAP_WINDOW_HSCROLL, MapWindow::OnScrollLineDown)
	EVT_COMMAND_SCROLL_PAGEUP    (MAP_WINDOW_HSCROLL, MapWindow::OnScrollPageUp)
	EVT_COMMAND_SCROLL_PAGEDOWN  (MAP_WINDOW_HSCROLL, MapWindow::OnScrollPageDown)

	EVT_COMMAND_SCROLL_TOP       (MAP_WINDOW_VSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_BOTTOM    (MAP_WINDOW_VSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_THUMBTRACK(MAP_WINDOW_VSCROLL, MapWindow::OnScroll)
	EVT_COMMAND_SCROLL_LINEUP    (MAP_WINDOW_VSCROLL, MapWindow::OnScrollLineUp)
	EVT_COMMAND_SCROLL_LINEDOWN  (MAP_WINDOW_VSCROLL, MapWindow::OnScrollLineDown)
	EVT_COMMAND_SCROLL_PAGEUP    (MAP_WINDOW_VSCROLL, MapWindow::OnScrollPageUp)
	EVT_COMMAND_SCROLL_PAGEDOWN  (MAP_WINDOW_VSCROLL, MapWindow::OnScrollPageDown)

	EVT_BUTTON(MAP_WINDOW_GEM, MapWindow::OnGem)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(MapScrollBar, wxScrollBar)
	EVT_KEY_DOWN(MapScrollBar::OnKey)
	EVT_KEY_UP(MapScrollBar::OnKey)
	EVT_CHAR(MapScrollBar::OnKey)
	EVT_SET_FOCUS(MapScrollBar::OnFocus)
	EVT_MOUSEWHEEL(MapScrollBar::OnWheel)
END_EVENT_TABLE()

wxIMPLEMENT_APP(Application);

Application::~Application()
{
	// Destroy
}

bool Application::OnInit()
{
#if defined __DEBUG_MODE__ && defined __WINDOWS__
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	std::cout << "This is free software: you are free to change and redistribute it." << std::endl;
	std::cout << "There is NO WARRANTY, to the extent permitted by law." << std::endl;
	std::cout << "Review COPYING in RME distribution for details." << std::endl;
	mt_seed(time(nullptr));
	srand(time(nullptr));

	// Discover data directory
	gui.discoverDataDirectory("clients.xml");

	// Tell that we are the real thing
	wxAppConsole::SetInstance(this);
	
	// Load some internal stuff
	settings.load();
	FixVersionDiscrapencies();
	gui.LoadHotkeys();
	ClientVersion::loadVersions();

#ifdef _USE_PROCESS_COM
	proc_server = nullptr;
	// Setup inter-process communice!
	if(settings.getInteger(Config::ONLY_ONE_INSTANCE)) 
	{
		{	
			// Prevents WX from complaining 'bout there being no server.
			wxLogNull nolog;

			RMEProcessClient client;
			wxConnectionBase* n_connection = client.MakeConnection(wxT("localhost"), wxT("rme_host"), wxT("rme_talk"));
			if(n_connection) 
			{
				RMEProcessConnection* connection = dynamic_cast<RMEProcessConnection*>(n_connection);
				ASSERT(connection);
				std::pair<bool, FileName> ff = ParseCommandLineMap();
				if(ff.first) 
				{
					connection->AskToLoad(ff.second);
					connection->Disconnect();
#if defined __DEBUG_MODE__ && defined __WINDOWS__
					gui.SaveHotkeys();
					settings.save(true);
#endif
					return false;
				}
				connection->Disconnect();
			}
		}
		// We act as server then
		proc_server = newd RMEProcessServer();
		if(!proc_server->Create(wxT("rme_host"))) 
		{
			// Another instance running!
			delete proc_server;
			proc_server = nullptr;
		}
	}
#endif

	// Image handlers
	//wxImage::AddHandler(newd wxBMPHandler);
	wxImage::AddHandler(newd wxPNGHandler);
	wxImage::AddHandler(newd wxJPEGHandler);
	wxImage::AddHandler(newd wxTGAHandler);

	gui.gfx.loadEditorSprites();

#ifndef __DEBUG_MODE__
	//wxHandleFatalExceptions(true);
#endif
	// Load all the dependency files
	std::string error;
	StringVector warnings;


	gui.root = newd MainFrame(wxT("Remere's Map Editor"), wxDefaultPosition, wxSize(700,500) );
	SetTopWindow(gui.root);
	gui.SetTitle(wxT(""));

	gui.root->LoadRecentFiles();

	// Load palette
	gui.LoadPerspective();

	// Show all windows
	gui.root->Show(true);

	// Set idle event handling mode
	wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);

	// Goto RME website?
	if(settings.getInteger(Config::GOTO_WEBSITE_ON_BOOT) == 1) 
	{
		::wxLaunchDefaultBrowser(wxT("http://www.remeresmapeditor.com/"), wxBROWSER_NEW_WINDOW);
		settings.setInteger(Config::GOTO_WEBSITE_ON_BOOT, 0);
	}

	// Check for updates
#ifdef _USE_UPDATER_
	if(settings.getInteger(Config::USE_UPDATER) == -1) 
	{
		int ret = gui.PopupDialog(
			wxT("Notice"),
			wxT("Do you want the editor to automatically check for updates?\n")
			wxT("It will connect to the internet if you choose yes.\n")
			wxT("You can change this setting in the preferences later."), wxYES | wxNO);
		if(ret == wxID_YES) 
		{
			settings.setInteger(Config::USE_UPDATER, 1);
		} 
		else 
		{
			settings.setInteger(Config::USE_UPDATER, 0);
		}
	}
	if(settings.getInteger(Config::USE_UPDATER) == 1) 
	{
		UpdateChecker updater;
		updater.connect(gui.root);	
	}
#endif

	FileName save_failed_file = gui.GetLocalDataDirectory();
	save_failed_file.SetName(wxT(".saving.txt"));
	if(save_failed_file.FileExists())
	{
		std::ifstream f(nstr(save_failed_file.GetFullPath()).c_str(), std::ios::in);

		std::string backup_otbm, backup_house, backup_spawn;

		getline(f, backup_otbm);
		getline(f, backup_house);
		getline(f, backup_spawn);

		// Remove the file
		f.close();
		std::remove(nstr(save_failed_file.GetFullPath()).c_str());

		// Query file retrieval if possible
		if(!backup_otbm.empty())
		{
			int ret = gui.PopupDialog(
				wxT("Editor Crashed"),
				wxString(
					wxT("IMPORTANT! THE EDITOR CRASHED WHILE SAVING!\n\n")
					wxT("Do you want to recover the lost map? (it will be opened immedietely):\n")) <<
					wxstr(backup_otbm) << wxT("\n") <<
					wxstr(backup_house) << wxT("\n") <<
					wxstr(backup_spawn) << wxT("\n"),
				wxYES | wxNO);

			if(ret == wxID_YES)
			{
				// Recover if the user so wishes
				std::remove(backup_otbm.substr(0, backup_otbm.size() - 1).c_str());
				std::rename(backup_otbm.c_str(), backup_otbm.substr(0, backup_otbm.size() - 1).c_str());

				if(backup_house.size())
				{
					std::remove(backup_house.substr(0, backup_house.size() - 1).c_str());
					std::rename(backup_house.c_str(), backup_house.substr(0, backup_house.size() - 1).c_str());
				}
				if(backup_spawn.size())
				{
					std::remove(backup_spawn.substr(0, backup_spawn.size() - 1).c_str());
					std::rename(backup_spawn.c_str(), backup_spawn.substr(0, backup_spawn.size() - 1).c_str());
				}
				
				// Load the map
				gui.LoadMap(wxstr(backup_otbm.substr(0, backup_otbm.size() - 1)));
				return true;
			}
		}
	}

	// Keep track of first event loop entry
	startup = true;
	return true;
}

void Application::OnEventLoopEnter(wxEventLoopBase* loop)
{
	// First startup?
	if (!startup)
		return;
	startup = false;

	// Don't try to create a map if we didn't load the client map.
	if (ClientVersion::getLatestVersion() == nullptr)
		return;

	// Handle any command line argument (open map...)
	std::pair<bool, FileName> ff = ParseCommandLineMap();
	if(ff.first) 
	{
		gui.LoadMap(ff.second);
	}
	else 
	{
		if(settings.getInteger(Config::CREATE_MAP_ON_STARTUP)) 
		{
			if(gui.NewMap()) 
			{
				// You generally don't want to save this map...
				gui.GetCurrentEditor()->map.clearChanges();
			}
		}
	}
}

void Application::FixVersionDiscrapencies() 
{
	// Here the registry should be fixed, if the version has been changed
	if(settings.getInteger(Config::VERSION_ID) < MAKE_VERSION_ID(1, 0, 5)) 
	{
		settings.setInteger(Config::USE_MEMCACHED_SPRITES_TO_SAVE, 0);
	}

	if(settings.getInteger(Config::VERSION_ID) < __RME_VERSION_ID__ && ClientVersion::getLatestVersion() != nullptr)
	{
		settings.setInteger(Config::DEFAULT_CLIENT_VERSION, ClientVersion::getLatestVersion()->getID());
	}

	wxString ss = wxstr(settings.getString(Config::SCREENSHOT_DIRECTORY));
	if(ss.empty())
	{
		ss = wxStandardPaths::Get().GetDocumentsDir();
#ifdef __WINDOWS__
		ss += wxT("/My Pictures/RME/");
#endif
	}
	settings.setString(Config::SCREENSHOT_DIRECTORY, nstr(ss));

	// Set registry to newest version
	settings.setInteger(Config::VERSION_ID, __RME_VERSION_ID__);
}

void Application::Unload() 
{
	gui.CloseAllEditors();
	gui.UnloadVersion();
	gui.SaveHotkeys();
	gui.SavePerspective();
	gui.root->SaveRecentFiles();
	ClientVersion::saveVersions();
	ClientVersion::unloadVersions();
	settings.save(true);
	gui.root = nullptr;
}

int Application::OnExit() {
#ifdef _USE_PROCESS_COM
	delete proc_server;
#endif
	return 1;
}

void Application::OnFatalException()
{
}

std::pair<bool, FileName> Application::ParseCommandLineMap() 
{
	if(argc == 2) 
	{
		FileName f = wxString(argv[1]);
		if(f.GetExt() == "otbm" || f.GetExt() == "otgz") 
		{
			return std::make_pair(true, f);
		}
	}
	return std::make_pair(false, FileName());
}

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size) :
	wxFrame((wxFrame *)nullptr, -1, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
	// Receive idle events
	SetExtraStyle(wxWS_EX_PROCESS_IDLE);
	// Creates the file-dropdown menu
	menu_bar = newd MainMenuBar(this);
	wxArrayString warnings;
	wxString error;

	wxFileName filename;
	filename.Assign(gui.getFoundDataDirectory() + wxT("menubar.xml"));
	if(!filename.FileExists())
		filename = FileName(gui.GetDataDirectory() + wxT("menubar.xml"));

	if(!menu_bar->Load(filename, warnings, error)) {
		wxLogError(wxString() +
			wxT("Could not load menubar.xml, editor will NOT be able to show its menu.\n")
		);
	}

	wxStatusBar* statusbar = CreateStatusBar();
	statusbar->SetFieldsCount(3);
	SetStatusText( wxString(wxT("Welcome to Remere's Map Editor ")) << __W_RME_VERSION__);

	// Le sizer
	gui.aui_manager = newd wxAuiManager(this);
	gui.tabbook = newd MapTabbook(this, wxID_ANY);
	
	gui.aui_manager->AddPane(gui.tabbook, wxAuiPaneInfo().CenterPane().Floatable(false).CloseButton(false).PaneBorder(false));
	//wxAuiNotebook* p = newd wxAuiNotebook(this, wxID_ANY);
	//gui.tabbook->notebook->AddPage(newd wxPanel(gui.tabbook->notebook), wxT("OMG IS TAB"));
	//p->AddPage(newd wxPanel(p), wxT("!!!"));
	//gui.aui_manager->AddPane(p, wxAuiPaneInfo());
	gui.aui_manager->Update();

	UpdateMenubar();
}

MainFrame::~MainFrame() 
{
	//wxTopLevelWindows.Erase(wxTopLevelWindows.GetFirst());
}

void MainFrame::OnIdle(wxIdleEvent& event) 
{
	// ...
}

#ifdef _USE_UPDATER_
void MainFrame::OnUpdateReceived(wxCommandEvent& event) 
{
	std::string data = *(std::string*)event.GetClientData();
	delete (std::string*)event.GetClientData();
	size_t first_colon = data.find(':');
	size_t second_colon = data.find(':', first_colon+1);

	if(first_colon == std::string::npos || second_colon == std::string::npos) 
		return;

	std::string update = data.substr(0, first_colon);
	std::string verstr = data.substr(first_colon+1, second_colon-first_colon-1);
	std::string url = (second_colon == data.size()? "" : data.substr(second_colon+1));

	if(update == "yes") 
	{
		int ret = gui.PopupDialog(
			wxT("Update Notice"),
			wxString(wxT("There is a newd update available (")) << wxstr(verstr) << 
			wxT("). Do you want to go to the website and download it?"),
			wxYES | wxNO,
			wxT("I don't want any update notices"),
			Config::AUTOCHECK_FOR_UPDATES
			);
		if(ret == wxID_YES)
			::wxLaunchDefaultBrowser(wxstr(url),  wxBROWSER_NEW_WINDOW);
	}
}
#endif

void MainFrame::OnUpdateMenus(wxCommandEvent&)
{
	UpdateMenubar();
	gui.UpdateMinimap(true);
	gui.UpdateTitle();
}

#ifdef __WINDOWS__
bool MainFrame::MSWTranslateMessage(WXMSG *msg)
{
	if(gui.AreHotkeysEnabled())
	{
		if(wxFrame::MSWTranslateMessage(msg))
			return true;
	}
	else
	{
		if(wxWindow::MSWTranslateMessage(msg))
			return true;
	}
	return false;
}
#endif

void MainFrame::UpdateMenubar()
{
	menu_bar->Update();
}

bool MainFrame::DoQueryClose() {
	Editor* editor = gui.GetCurrentEditor();
	if(editor) {
		if(editor->IsLive()) {
			long ret = gui.PopupDialog(
				wxT("Must Close Server"),
				wxString(wxT("You are currently connected to a live server, to close this map the connection must be severed.")),
				wxOK | wxCANCEL);
			if(ret == wxID_OK) {
				editor->CloseLiveServer();
			} else {
				return false;
			}
		}
	}
	return true;
}

bool MainFrame::DoQuerySave(bool doclose) 
{
	if (!gui.IsEditorOpen()) {
		return true;
	}
	
	Editor& editor = *gui.GetCurrentEditor();
	if (editor.IsLiveClient()) {
		long ret = gui.PopupDialog(
			wxT("Disconnect"),
			wxT("Do you want to disconnect?"),
			wxYES | wxNO
		);

		if (ret != wxID_YES) {
			return false;
		}

		editor.CloseLiveServer();
		return DoQuerySave(doclose);
	} else if (editor.IsLiveServer()) {
		long ret = gui.PopupDialog(
			wxT("Shutdown"),
			wxT("Do you want to shut down the server? (any clients will be disconnected)"),
			wxYES | wxNO
		);

		if (ret != wxID_YES) {
			return false;
		}

		editor.CloseLiveServer();
		return DoQuerySave(doclose);
	} else if (gui.ShouldSave()) {
		long ret = gui.PopupDialog(
			wxT("Save changes"),
			wxT("Do you want to save your changes to \"") + wxstr(gui.GetCurrentMap().getName()) + wxT("\"?"),
			wxYES | wxNO | wxCANCEL
		);

		if (ret == wxID_YES) {
			if (gui.GetCurrentMap().hasFile()) {
				gui.SaveCurrentMap(true);
			} else {
				wxFileDialog file(
					this, wxT("Save..."),
					wxT(""), wxT(""),
					wxT("*.otbm"), wxFD_SAVE | wxFD_OVERWRITE_PROMPT
				);

				int32_t result = file.ShowModal();
				if (result == wxID_OK) {
					gui.SaveCurrentMap(file.GetPath(), true);
				} else {
					return false;
				}
			}
		} else if (ret == wxID_CANCEL) {
			return false;
		}
	}

	if (doclose) {
		UnnamedRenderingLock();
		gui.CloseCurrentEditor();
	}

	return true;
}

bool MainFrame::DoQueryImportCreatures() 
{
	if(creature_db.hasMissing()) 
	{
		int ret = gui.PopupDialog(wxT("Missing creatures"), wxT("There are missing creatures and/or NPC in the editor, do you want to load them from an OT monster/npc file?"), wxYES | wxNO);
		if(ret == wxID_YES) 
		{
			do 
			{
				wxFileDialog dlg(gui.root, wxT("Import monster/npc file"), wxT(""),wxT(""),wxT("*.xml"), wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
				if(dlg.ShowModal() == wxID_OK) 
				{
					wxArrayString paths;
					dlg.GetPaths(paths);
					for(uint i = 0; i < paths.GetCount(); ++i) 
					{
						wxString error;
						wxArrayString warnings;
						bool ok = creature_db.importXMLFromOT(FileName(paths[i]), error, warnings);
						if(ok)
							gui.ListDialog(wxT("Monster loader errors"), warnings);
						else 
							wxMessageBox(wxT("Error OT data file \"") + paths[i] + wxT("\".\n") + error, wxT("Error"), wxOK | wxICON_INFORMATION, gui.root);
					}
				}
				else 
				{
					break;
				}
			} while(creature_db.hasMissing());
		}
	}
	gui.RefreshPalettes();
	return true;
}

void MainFrame::UpdateFloorMenu() 
{
	menu_bar->UpdateFloorMenu();
}

bool MainFrame::LoadMap(FileName name) 
{
	return gui.LoadMap(name);
}

void MainFrame::OnExit(wxCloseEvent& event) 
{
	while(gui.IsEditorOpen()) 
	{
		if(!DoQuerySave()) 
		{
			if(event.CanVeto()) 
			{
				event.Veto();
				return;
			}
			else
			{
				break;
			}
		}
	}
	gui.aui_manager->UnInit();
	((Application&)wxGetApp()).Unload();
#ifdef __RELEASE__
	// Hack, "crash" gracefully in release builds, let OS handle cleanup of windows
	exit(0);
#endif
	Destroy();
}

void MainFrame::AddRecentFile(const FileName& file)
{
	menu_bar->AddRecentFile(file);
}

void MainFrame::LoadRecentFiles()
{
	menu_bar->LoadRecentFiles();
}

void MainFrame::SaveRecentFiles() 
{
	menu_bar->SaveRecentFiles();
}

void MainFrame::PrepareDC(wxDC& dc)
{
	dc.SetLogicalOrigin( 0, 0 );
	dc.SetAxisOrientation( 1, 0);
	dc.SetUserScale( 1.0, 1.0 );
	dc.SetMapMode( wxMM_TEXT );
}

#if !defined __DEBUG__ && defined __WXOSX__

void wxOnAssert(wchar_t const*, int, char const*, wchar_t const*, wchar_t const*) {
    ;
}

void wxAppConsole::OnAssertFailure(wchar_t const*, int, wchar_t const*, wchar_t const*, wchar_t const*) {
    ;
}

void wxAppConsole::OnAssert(wchar_t const*, int, wchar_t const*, wchar_t const*) {
    
}

#endif
