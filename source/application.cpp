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
#include "artprovider.h"

#include "materials.h"
#include "map.h"
#include "complexitem.h"
#include "monster.h"
#include "npc.h"

#include "../brushes/icon/rme_icon.xpm"

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

	spdlog::info("This is free software: you are free to change and redistribute it");
	spdlog::info("There is NO WARRANTY, to the extent permitted by law");
	spdlog::info("Review COPYING in RME distribution for details");
	spdlog::info("Visit our website for updates, support, and resources: https://docs.opentibiabr.com/");
	spdlog::info("Application started sucessfull!\n");

	mt_seed(time(nullptr));
	srand(time(nullptr));

	// Discover data directory
	g_gui.discoverDataDirectory("clients.xml");

	// Tell that we are the real thing
	wxAppConsole::SetInstance(this);
	wxArtProvider::Push(new ArtProvider());

#if defined(__LINUX__) || defined(__WINDOWS__)
	int argc = 1;
	char* argv[1] = { wxString(this->argv[0]).char_str() };
	glutInit(&argc, argv);
#endif

	// Load some internal stuff
	g_settings.load();
	g_gui.LoadHotkeys();
	Assets::load();

#ifdef _USE_PROCESS_COM
	m_single_instance_checker = newd wxSingleInstanceChecker; //Instance checker has to stay alive throughout the applications lifetime
	if (g_settings.getInteger(Config::ONLY_ONE_INSTANCE) && m_single_instance_checker->IsAnotherRunning()) {
		RMEProcessClient client;
		wxConnectionBase* connection = client.MakeConnection("localhost", "rme_host", "rme_talk");
		if (connection) {
			wxString fileName;
			if (ParseCommandLineMap(fileName)) {
				wxLogNull nolog; //We might get a timeout message if the file fails to open on the running instance. Let's not show that message.
				connection->Execute(fileName);
			}
			connection->Disconnect();
			wxDELETE(connection);
		}
		wxDELETE(m_single_instance_checker);
		return false; //Since we return false - OnExit is never called
	}
	// We act as server then
	m_proc_server = newd RMEProcessServer();
	if(!m_proc_server->Create("rme_host")) {
		wxLogWarning("Could not register IPC service!");
	}
#endif

	// Image handlers
	//wxImage::AddHandler(newd wxBMPHandler);
	wxImage::AddHandler(newd wxPNGHandler);
	wxImage::AddHandler(newd wxJPEGHandler);
	wxImage::AddHandler(newd wxTGAHandler);

	g_gui.gfx.loadEditorSprites();

#ifndef __DEBUG_MODE__
	//wxHandleFatalExceptions(true);
#endif

	m_file_to_open = wxEmptyString;
	ParseCommandLineMap(m_file_to_open);

	g_gui.root = newd MainFrame(__W_RME_APPLICATION_NAME__, wxDefaultPosition, wxSize(700,500));
	SetTopWindow(g_gui.root);
	g_gui.SetTitle("");

	g_gui.root->LoadRecentFiles();

	// Load palette
	g_gui.LoadPerspective();

    wxIcon icon(rme_icon);
    g_gui.root->SetIcon(icon);

    if (g_settings.getInteger(Config::WELCOME_DIALOG) == 1 && m_file_to_open == wxEmptyString) {
        g_gui.ShowWelcomeDialog(icon);
    } else {
        g_gui.root->Show();
    }

	// Set idle event handling mode
	wxIdleEvent::SetMode(wxIDLE_PROCESS_SPECIFIED);

	// Goto RME website?
	if(g_settings.getInteger(Config::GOTO_WEBSITE_ON_BOOT) == 1) {
		::wxLaunchDefaultBrowser("http://www.remeresmapeditor.com/", wxBROWSER_NEW_WINDOW);
		g_settings.setInteger(Config::GOTO_WEBSITE_ON_BOOT, 0);
	}

	// Check for updates
#ifdef _USE_UPDATER_
	if(g_settings.getInteger(Config::USE_UPDATER) == -1) {
		int ret = g_gui.PopupDialog(
			"Notice",
			"Do you want the editor to automatically check for updates?\n"
			"It will connect to the internet if you choose yes.\n"
			"You can change this setting in the preferences later.", wxYES | wxNO);
		if(ret == wxID_YES) {
			g_settings.setInteger(Config::USE_UPDATER, 1);
		} else {
			g_settings.setInteger(Config::USE_UPDATER, 0);
		}
	}
	if(g_settings.getInteger(Config::USE_UPDATER) == 1) {
		UpdateChecker updater;
		updater.connect(g_gui.root);
	}
#endif

	FileName save_failed_file = GUI::GetLocalDataDirectory();
	save_failed_file.SetName(".saving.txt");
	if(save_failed_file.FileExists()) {
		std::ifstream f(nstr(save_failed_file.GetFullPath()).c_str(), std::ios::in);

		std::string backup_otbm, backup_house, backup_spawn, backup_spawn_npc;

		getline(f, backup_otbm);
		getline(f, backup_house);
		getline(f, backup_spawn);
		getline(f, backup_spawn_npc);

		// Remove the file
		f.close();
		std::remove(nstr(save_failed_file.GetFullPath()).c_str());

		// Query file retrieval if possible
		if(!backup_otbm.empty()) {
			long ret = g_gui.PopupDialog(
				"Editor Crashed",
				wxString(
					"IMPORTANT! THE EDITOR CRASHED WHILE SAVING!\n\n"
					"Do you want to recover the lost map? (it will be opened immediately):\n") <<
					wxstr(backup_otbm) << "\n" <<
					wxstr(backup_house) << "\n" <<
					wxstr(backup_spawn) << "\n" <<
					wxstr(backup_spawn_npc) << "\n",
				wxYES | wxNO);

			if(ret == wxID_YES) {
				// Recover if the user so wishes
				std::remove(backup_otbm.substr(0, backup_otbm.size() - 1).c_str());
				std::rename(backup_otbm.c_str(), backup_otbm.substr(0, backup_otbm.size() - 1).c_str());

				if(!backup_house.empty()) {
					std::remove(backup_house.substr(0, backup_house.size() - 1).c_str());
					std::rename(backup_house.c_str(), backup_house.substr(0, backup_house.size() - 1).c_str());
				}
				if(!backup_spawn.empty()) {
					std::remove(backup_spawn.substr(0, backup_spawn.size() - 1).c_str());
					std::rename(backup_spawn.c_str(), backup_spawn.substr(0, backup_spawn.size() - 1).c_str());
				}
				if(!backup_spawn_npc.empty()) {
					std::remove(backup_spawn_npc.substr(0, backup_spawn_npc.size() - 1).c_str());
					std::rename(backup_spawn_npc.c_str(), backup_spawn_npc.substr(0, backup_spawn_npc.size() - 1).c_str());
				}

				// Load the map
				g_gui.LoadMap(wxstr(backup_otbm.substr(0, backup_otbm.size() - 1)));
				return true;
			}
		}
	}

	// Keep track of first event loop entry
	m_startup = true;
	return true;
}

void Application::OnEventLoopEnter(wxEventLoopBase* loop) {

    //First startup?
    if (!m_startup)
        return;
    m_startup = false;

    //Open a map.
    if (m_file_to_open != wxEmptyString) {
        g_gui.LoadMap(FileName(m_file_to_open));
    } else if (!g_gui.IsWelcomeDialogShown() && g_gui.NewMap()) { //Open a new empty map
        // You generally don't want to save this map...
        g_gui.GetCurrentEditor()->map.clearChanges();
    }
}

void Application::MacOpenFiles(const wxArrayString& fileNames)
{
	if (!fileNames.IsEmpty()) {
		g_gui.LoadMap(FileName(fileNames.Item(0)));
	}
}

void Application::Unload()
{
	g_gui.CloseAllEditors();
	g_gui.SaveHotkeys();
	g_gui.SavePerspective();
	g_gui.root->SaveRecentFiles();
	Assets::save();
	g_settings.save(true);
	g_gui.root = nullptr;
}

int Application::OnExit()
{
#ifdef _USE_PROCESS_COM
	wxDELETE(m_proc_server);
	wxDELETE(m_single_instance_checker);
#endif
	return 1;
}

void Application::OnFatalException()
{
	////
}

bool Application::ParseCommandLineMap(wxString& fileName)
{
	if(argc == 2) {
		fileName = wxString(argv[1]);
		return true;
	}
	return false;
}

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size) :
	wxFrame((wxFrame *)nullptr, -1, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
	// Receive idle events
	SetExtraStyle(wxWS_EX_PROCESS_IDLE);

	#if wxCHECK_VERSION(3, 1, 0) //3.1.0 or higher
		// Make sure ShowFullScreen() uses the full screen API on macOS
		EnableFullScreenView(true);
    #endif

	// Creates the file-dropdown menu
	menu_bar = newd MainMenuBar(this);
	wxArrayString warnings;
	wxString error;

	wxFileName filename;
	filename.Assign(g_gui.getFoundDataDirectory() + "menubar.xml");
	if(!filename.FileExists())
		filename = FileName(GUI::GetDataDirectory() + "menubar.xml");

	if(!menu_bar->Load(filename, warnings, error)) {
		wxLogError(wxString() + "Could not load menubar.xml, editor will NOT be able to show its menu.\n");
	}

	wxStatusBar* statusbar = CreateStatusBar();
	statusbar->SetFieldsCount(4);
	SetStatusText(wxString("Welcome to ") << __W_RME_APPLICATION_NAME__ << " " << __W_RME_VERSION__);

	// Le sizer
	g_gui.aui_manager = newd wxAuiManager(this);
	g_gui.tabbook = newd MapTabbook(this, wxID_ANY);

	tool_bar = newd MainToolBar(this, g_gui.aui_manager);

	g_gui.aui_manager->AddPane(g_gui.tabbook, wxAuiPaneInfo().CenterPane().Floatable(false).CloseButton(false).PaneBorder(false));
	g_gui.aui_manager->Update();

	UpdateMenubar();
}

MainFrame::~MainFrame() = default;

void MainFrame::OnIdle(wxIdleEvent& event)
{
	////
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

	if(update == "yes") {
		int ret = g_gui.PopupDialog(
			"Update Notice",
			wxString("There is a newd update available (") << wxstr(verstr) <<
			"). Do you want to go to the website and download it?",
			wxYES | wxNO,
			"I don't want any update notices",
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
	g_gui.UpdateMinimap(true);
	g_gui.UpdateTitle();
}

#ifdef __WINDOWS__
bool MainFrame::MSWTranslateMessage(WXMSG *msg)
{
	if(g_gui.AreHotkeysEnabled()) {
		if(wxFrame::MSWTranslateMessage(msg))
			return true;
	} else {
		if(wxWindow::MSWTranslateMessage(msg))
			return true;
	}
	return false;
}
#endif

void MainFrame::UpdateMenubar()
{
	menu_bar->Update();
	tool_bar->UpdateButtons();
}

bool MainFrame::DoQueryClose() {
	Editor* editor = g_gui.GetCurrentEditor();
	if(editor) {
		if(editor->IsLive()) {
			long ret = g_gui.PopupDialog(
				"Must Close Server",
				wxString("You are currently connected to a live server, to close this map the connection must be severed."),
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
	if(!g_gui.IsEditorOpen()) {
		return true;
	}

	Editor& editor = *g_gui.GetCurrentEditor();
	if(editor.IsLiveClient()) {
		long ret = g_gui.PopupDialog(
			"Disconnect",
			"Do you want to disconnect?",
			wxYES | wxNO
		);

		if(ret != wxID_YES) {
			return false;
		}

		editor.CloseLiveServer();
		return DoQuerySave(doclose);
	} else if(editor.IsLiveServer()) {
		long ret = g_gui.PopupDialog(
			"Shutdown",
			"Do you want to shut down the server? (any clients will be disconnected)",
			wxYES | wxNO
		);

		if(ret != wxID_YES) {
			return false;
		}

		editor.CloseLiveServer();
		return DoQuerySave(doclose);
	} else if(g_gui.ShouldSave()) {
		long ret = g_gui.PopupDialog(
			"Save changes",
			"Do you want to save your changes to \"" + wxstr(g_gui.GetCurrentMap().getName()) + "\"?",
			wxYES | wxNO | wxCANCEL
		);

		if(ret == wxID_YES) {
			if(g_gui.GetCurrentMap().hasFile()) {
				g_gui.SaveCurrentMap(true);
			} else {
				wxFileDialog file(this, "Save...", "", "", "*.otbm", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
				int32_t result = file.ShowModal();
				if(result == wxID_OK) {
					g_gui.SaveCurrentMap(file.GetPath(), true);
				} else {
					return false;
				}
			}
		} else if(ret == wxID_CANCEL) {
			return false;
		}
	}

	if(doclose) {
		UnnamedRenderingLock();
		g_gui.CloseCurrentEditor();
	}

	return true;
}

bool MainFrame::DoQueryImportCreatures()
{
	// Monsters
	if(g_monsters.hasMissing()) {
		long ret = g_gui.PopupDialog("Missing monsters", "There are missing monsters in the editor, do you want to load them from an OT monster file?", wxYES | wxNO);
		if(ret == wxID_YES) {
			do {
				wxFileDialog dlg(g_gui.root, "Import monster file", "","","*.xml", wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
				if(dlg.ShowModal() == wxID_OK) {
					wxArrayString paths;
					dlg.GetPaths(paths);
					for(uint32_t i = 0; i < paths.GetCount(); ++i) {
						wxString error;
						wxArrayString warnings;
						bool ok = g_monsters.importXMLFromOT(FileName(paths[i]), error, warnings);
						if(ok)
							g_gui.ListDialog("Monster loader errors", warnings);
						else
							wxMessageBox("Error OT data file \"" + paths[i] + "\".\n" + error, "Error", wxOK | wxICON_INFORMATION, g_gui.root);
					}
				} else {
					break;
				}
			} while(g_monsters.hasMissing());
		}
	}
	// Npcs
	if(g_npcs.hasMissing()) {
		long ret = g_gui.PopupDialog("Missing npcs", "There are missing npcs in the editor, do you want to load them from an OT npc file?", wxYES | wxNO);
		if(ret == wxID_YES) {
			do {
				wxFileDialog dlg(g_gui.root, "Import npc file", "","","*.xml", wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
				if(dlg.ShowModal() == wxID_OK) {
					wxArrayString paths;
					dlg.GetPaths(paths);
					for(uint32_t i = 0; i < paths.GetCount(); ++i) {
						wxString error;
						wxArrayString warnings;
						bool ok = g_npcs.importXMLFromOT(FileName(paths[i]), error, warnings);
						if(ok)
							g_gui.ListDialog("Npc loader errors", warnings);
						else
							wxMessageBox("Error OT data file \"" + paths[i] + "\".\n" + error, "Error", wxOK | wxICON_INFORMATION, g_gui.root);
					}
				} else {
					break;
				}
			} while(g_npcs.hasMissing());
		}
	}
	g_gui.RefreshPalettes();
	return true;
}

void MainFrame::UpdateFloorMenu()
{
	menu_bar->UpdateFloorMenu();
}

bool MainFrame::LoadMap(FileName name)
{
	return g_gui.LoadMap(name);
}

void MainFrame::OnExit(wxCloseEvent& event)
{
	while(g_gui.IsEditorOpen()) {
		if(!DoQuerySave()) {
			if(event.CanVeto()) {
				event.Veto();
				return;
			} else {
				break;
			}
		}
	}
	g_gui.aui_manager->UnInit();
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

std::vector<wxString> MainFrame::GetRecentFiles()
{
    return menu_bar->GetRecentFiles();
}

void MainFrame::PrepareDC(wxDC& dc)
{
	dc.SetLogicalOrigin( 0, 0 );
	dc.SetAxisOrientation( 1, 0);
	dc.SetUserScale( 1.0, 1.0 );
	dc.SetMapMode( wxMM_TEXT );
}

#ifdef _MSC_VER
// This is necessary for cmake with visual studio link the executable
int main(int argc, char** argv)
{
	wxEntryStart(argc, argv); // Start the wxWidgets library
	Application* app = new Application(); // Create the application object
	wxApp::SetInstance(app); // Informs wxWidgets that app is the application object
	wxEntry(); // Call the wxEntry() function to start the application execution
	wxEntryCleanup(); // Clear the wxWidgets library
	return 0;
}
#endif
