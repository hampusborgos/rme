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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/about_window.hpp $
// $Id: about_window.hpp 322 2010-04-12 16:58:47Z admin $

#include "main.h"

#include "gui.h"

#include "about_window.h"
#include <fstream>
#include <typeinfo>
#include <memory>

class GamePanel : public wxPanel {
public:
	GamePanel(wxWindow* parent, int width, int height);
	virtual ~GamePanel();

	void OnPaint(wxPaintEvent&);
	void OnKeyDown(wxKeyEvent&);
	void OnKeyUp(wxKeyEvent&);
	void OnIdle(wxIdleEvent&);

	void pause() {paused_val = true;}
	void unpause() {paused_val = false;}
	bool paused() const {return paused_val || dead;}
protected:
	virtual void Render(wxDC& pdc) = 0;
	virtual void GameLoop(int time) = 0;
	virtual void OnKey(wxKeyEvent& event, bool down) = 0;
	
	virtual int getFPS() const = 0;
protected:
	wxStopWatch game_timer;
private:
	bool paused_val;

	DECLARE_EVENT_TABLE()

protected:
	bool dead;
};

const int TETRIS_MAPHEIGHT = 20;
const int TETRIS_MAPWIDTH = 10;

class TetrisPanel : public GamePanel {
public:
	TetrisPanel(wxWindow* parent);
	~TetrisPanel();

protected:
	virtual void Render(wxDC& pdc);
	virtual void GameLoop(int time);
	virtual void OnKey(wxKeyEvent& event, bool down);
	
	virtual int getFPS() const {return lines / 10 + 3;}

	enum Color {
		NO_COLOR,
		RED,
		BLUE,
		GREEN,
		STEEL,
		YELLOW,
		PURPLE,
		WHITE,
	};

	enum BlockType {
		FIRST_BLOCK,
		BLOCK_TOWER = FIRST_BLOCK,
		BLOCK_SQUARE,
		BLOCK_TRIANGLE,
		BLOCK_L,
		BLOCK_J,
		BLOCK_Z,
		BLOCK_S,
		LAST_BLOCK = BLOCK_S
	};

	struct Block {
		Color structure[4][4];
		int x, y;
	} block;

	const wxBrush& GetBrush(Color color) const;
	bool BlockCollisionTest(int mx, int my) const;
	void RemoveRow(int row);
	void NewBlock();
	void MoveBlock(int x, int y);
	void RotateBlock();
	void NewGame();
	void EndGame();
	void AddScore(int lines);

	int score;
	int lines;
	Color map[TETRIS_MAPWIDTH][TETRIS_MAPHEIGHT];
};

const int SNAKE_MAPHEIGHT = 20;
const int SNAKE_MAPWIDTH = 20;

class SnakePanel : public GamePanel {
public:
	SnakePanel(wxWindow* parent);
	~SnakePanel();

protected:
	virtual void Render(wxDC& pdc);
	virtual void GameLoop(int time);
	virtual void OnKey(wxKeyEvent& event, bool down);
	
	virtual int getFPS() const {return 7;}

	enum {
		NORTH,
		SOUTH,
		WEST,
		EAST,
	};

	void NewApple();
	void Move(int dir);
	void NewGame();
	void EndGame();
	void UpdateTitle();

	// -1 is apple, 0 is nothing, >0 is snake (will decay in n rounds)
	int length;
	int last_dir;
	int map[SNAKE_MAPWIDTH][SNAKE_MAPHEIGHT];
};

//=============================================================================
// About Window - Information window about the application

BEGIN_EVENT_TABLE(AboutWindow, wxDialog)
	EVT_BUTTON(wxID_OK, AboutWindow::OnClickOK)
	EVT_BUTTON(ABOUT_VIEW_LICENSE, AboutWindow::OnClickLicense)
	EVT_MENU(ABOUT_RUN_TETRIS, AboutWindow::OnTetris)
	EVT_MENU(ABOUT_RUN_SNAKE, AboutWindow::OnSnake)
	EVT_MENU(wxID_CANCEL, AboutWindow::OnClickOK)
END_EVENT_TABLE()

AboutWindow::AboutWindow(wxWindow* parent) :
	wxDialog(parent, wxID_ANY, wxT("About"), wxDefaultPosition, wxSize(300, 320), wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX),
	game_panel(nullptr)
{
	wxString about;
	
	about << wxT("This is an OpenTibia Map Editor created by Remere.\n");
	about << wxT("Version ") << __W_RME_VERSION__ << wxT(" for ");
	about << 
#ifdef __WINDOWS__
	wxT("Windows");
#elif __LINUX__
	wxT("Linux");
#else
	wxT("Unsupported OS");
#endif
	about << wxT("\n\n");
	
	about << wxT("Using ") wxVERSION_STRING wxT(" interface\n");
	about << wxT("OpenGL version ") + wxString((char*)glGetString(GL_VERSION), wxConvUTF8) + wxT("\n");
	about << wxT("\n");
	about << wxT("This program comes with ABSOLUTELY NO WARRANTY;\n");
	about << wxT("for details see the LICENSE file.\n");
	about << wxT("This is free software, and you are welcome to redistribute it\n");
	about << wxT("under certain conditions.\n");
	about << wxT("\n");
	about << wxT("Compiled ") << __TDATE__ << wxT(" : ") << __TTIME__;

	topsizer = newd wxBoxSizer(wxVERTICAL);

	topsizer->Add(newd wxStaticText(this, wxID_ANY, about), 1, wxALL, 20);

	wxSizer* choicesizer = newd wxBoxSizer(wxHORIZONTAL);
	choicesizer->Add(newd wxButton(this, wxID_OK, wxT("OK")), wxSizerFlags(1).Center());
	topsizer->Add(choicesizer, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT | wxBOTTOM, 20);

	wxAcceleratorEntry entries[3];
	entries[0].Set(wxACCEL_NORMAL, WXK_ESCAPE, wxID_CANCEL);
	entries[1].Set(wxACCEL_NORMAL, 't', ABOUT_RUN_TETRIS);
	entries[2].Set(wxACCEL_NORMAL, 's', ABOUT_RUN_SNAKE);
	wxAcceleratorTable accel(3, entries);
	SetAcceleratorTable(accel);

	SetSizerAndFit(topsizer);
}

AboutWindow::~AboutWindow() {
}

void AboutWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) {
	EndModal(0);
}

void AboutWindow::OnClickLicense(wxCommandEvent& WXUNUSED(event)) {
	FileName path;
	try {
		path = wxStandardPaths::Get().GetExecutablePath();
	} catch(std::bad_cast&) {
		return;
	}
	path.SetFullName(wxT("COPYING.txt"));
	std::ifstream gpl(path.GetFullPath().mb_str());

	std::string gpl_str;
	char ch;
	while(gpl.get(ch)) {
		gpl_str += ch;
	}
	
	gui.ShowTextBox(this, wxT("License"), wxstr(gpl_str.size()? gpl_str : "The COPYING.txt file is not available."));
}

void AboutWindow::OnTetris(wxCommandEvent&) {
	if(!game_panel) {
		DestroyChildren();
		game_panel = newd TetrisPanel(this);
		topsizer->Add(game_panel, 1, wxALIGN_CENTER | wxALL, 7);
		Fit();
		game_panel->SetFocus();
		SetWindowStyleFlag(wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);
		Refresh();
	}
}

void AboutWindow::OnSnake(wxCommandEvent&) {
	if(!game_panel) {
		DestroyChildren();
		game_panel = newd SnakePanel(this);
		topsizer->Add(game_panel, 1, wxALIGN_CENTER | wxALL, 7);
		Fit();
		game_panel->SetFocus();
		SetWindowStyleFlag(wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);
		Refresh();
	}
}

//=============================================================================
// GamePanel - Abstract class for games

BEGIN_EVENT_TABLE(GamePanel, wxPanel)
	EVT_KEY_DOWN(GamePanel::OnKeyDown)
	EVT_KEY_UP(GamePanel::OnKeyUp)
	EVT_PAINT(GamePanel::OnPaint)
	EVT_IDLE(GamePanel::OnIdle)
END_EVENT_TABLE()

GamePanel::GamePanel(wxWindow *parent, int width, int height) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(width, height), wxWANTS_CHARS),
	paused_val(false),
	dead(false)
{
	// Receive idle events
	SetExtraStyle(wxWS_EX_PROCESS_IDLE);
	// Complete redraw
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

GamePanel::~GamePanel() {
}

void GamePanel::OnPaint(wxPaintEvent&) {
	wxBufferedPaintDC pdc(this);
	Render(pdc);
}

void GamePanel::OnKeyDown(wxKeyEvent& event) {
	switch(event.GetKeyCode()) {
		case WXK_ESCAPE: {
			if(dead) return;
			wxDialog* dlg = (wxDialog*)GetParent();
			dlg->EndModal(0);
		} break;
		default: {
			OnKey(event, true);
		} break;
	}
}

void GamePanel::OnKeyUp(wxKeyEvent& event) {
	OnKey(event, false);
}

void GamePanel::OnIdle(wxIdleEvent& event) {
	int time = game_timer.Time();
	if(time > 1000 / getFPS()) {
		game_timer.Start();
		if(!paused()) GameLoop(time);
	}
	if(!paused()) {
		event.RequestMore(true);
	}
}

//=============================================================================
// TetrisPanel - A window with a Tetris game!

TetrisPanel::TetrisPanel(wxWindow* parent) : GamePanel(parent, 16*TETRIS_MAPWIDTH, 16*TETRIS_MAPHEIGHT) {
	NewGame();
}

TetrisPanel::~TetrisPanel() {
}


const wxBrush& TetrisPanel::GetBrush(Color color) const {
	static std::unique_ptr<wxBrush> yellow_brush;
	static std::unique_ptr<wxBrush> purple_brush;

	if(yellow_brush.get() == nullptr) yellow_brush.reset(newd wxBrush(wxColor(255, 255, 0)));
	if(purple_brush.get() == nullptr) purple_brush.reset(newd wxBrush(wxColor(128, 0, 255)));

	const wxBrush* brush = nullptr;
	switch(color) {
		case RED: brush = wxRED_BRUSH; break;
		case BLUE: brush = wxCYAN_BRUSH; break;
		case GREEN: brush = wxGREEN_BRUSH; break;
		case PURPLE: brush = purple_brush.get(); break;
		case YELLOW: brush = yellow_brush.get(); break;
		case WHITE: brush = wxWHITE_BRUSH; break;
		case STEEL: brush = wxGREY_BRUSH; break;
		default: brush = wxBLACK_BRUSH; break;
	}
	return *brush;
}

void TetrisPanel::Render(wxDC& pdc) {
	pdc.SetBackground(*wxBLACK_BRUSH);
	pdc.Clear();

	for(int y = 0; y < TETRIS_MAPHEIGHT; ++y) {
		for(int x = 0; x < TETRIS_MAPWIDTH; ++x) {
			pdc.SetBrush(GetBrush(map[x][y]));
			pdc.DrawRectangle(x*16, y*16, 16, 16);
		}
	} 

	for(int y = 0; y < 4; ++y) {
		for(int x = 0; x < 4; ++x) {
			if(block.structure[x][y] != NO_COLOR) {
				pdc.SetBrush(GetBrush(block.structure[x][y]));
				pdc.DrawRectangle(
					(block.x + x)*16,
					(block.y + y)*16,
					16, 16);
			}
		}
	}
}

void TetrisPanel::OnKey(wxKeyEvent& event, bool down) {
	if(!down) return;
	if(dead) return;
	switch(event.GetKeyCode()) {
		case WXK_SPACE: {
			if(paused())
				unpause();
			else pause();
		} break;
		case WXK_NUMPAD_UP: case WXK_UP: {
			if(dead) return;
			unpause();
			RotateBlock();
		} break;
		case WXK_NUMPAD_DOWN: case WXK_DOWN: {
			if(dead) return;
			unpause();
			MoveBlock(0, 1);
		} break;
		case WXK_NUMPAD_LEFT: case WXK_LEFT: {
			if(dead) return;
			unpause();
			MoveBlock(-1, 0);
		} break;
		case WXK_NUMPAD_RIGHT: case WXK_RIGHT: {
			if(dead) return;
			unpause();
			MoveBlock(1, 0);
		} break;
	}
}

void TetrisPanel::NewGame() {
	NewBlock();
	score = 0;
	lines = 0;
	game_timer.Start();
	unpause();
	dead = false;

	// Clear map
	for(int y = 0; y < TETRIS_MAPHEIGHT; ++y) {
		for(int x = 0; x < TETRIS_MAPWIDTH; ++x) {
			map[x][y] = NO_COLOR;
		}
	}
	AddScore(0); // Update title
}

void TetrisPanel::AddScore(int lines_added) {
	lines += lines_added;
	score += lines_added*lines_added*10;
	wxString title = wxT("Remere's Tetris : ");
	title << score << wxT(" points  ");
	title << lines << wxT(" lines");
	((wxTopLevelWindow*)GetParent())->SetTitle(title);
}

void TetrisPanel::GameLoop(int time) {
	MoveBlock(0, 1);
}

bool TetrisPanel::BlockCollisionTest(int mx, int my) const {
	int nx = block.x + mx;
	int ny = block.y + my;

	for(int y = 0; y < 4; ++y) {
		for(int x = 0; x < 4; ++x) {
			if(block.structure[x][y] != NO_COLOR) {
				if(nx + x < 0 || nx + x > TETRIS_MAPWIDTH - 1 ||
					ny + y < 0 || ny + y > TETRIS_MAPHEIGHT - 1)
				{
					return true;
				}
			}
		}
	}

	for(int y = 0; y < TETRIS_MAPHEIGHT; ++y) {
		for(int x = 0; x < TETRIS_MAPWIDTH; ++x) {
			if(x >= nx && x < nx + 4 && y >= ny && y < ny + 4) {
				if(map[x][y] != NO_COLOR && block.structure[x - nx][y - ny] != NO_COLOR) {
					return true;
				}
			}
		}
	}
	return false;
}

void TetrisPanel::RemoveRow(int row) {
	for(int x = 0; x < TETRIS_MAPWIDTH; ++x) {
		for(int y = row; y > 0; --y) { //Move all above one step down
			map[x][y] = map[x][y-1];
		}
	}
}

void TetrisPanel::NewBlock() {
	block.x = TETRIS_MAPWIDTH / 2;
	block.y = -1;

	for(int y = 0; y < 4; ++y) {
		for(int x = 0; x < 4; ++x) {
			block.structure[x][y] = NO_COLOR;
		}
	}
	switch(random(FIRST_BLOCK, LAST_BLOCK)) {
		case BLOCK_TOWER: {
			block.structure[1][0] = RED;
			block.structure[1][1] = RED;
			block.structure[1][2] = RED;
			block.structure[1][3] = RED;
			block.y = 0;
		} break;
		default: case BLOCK_SQUARE: {
			block.structure[1][1] = BLUE;
			block.structure[1][2] = BLUE;
			block.structure[2][1] = BLUE;
			block.structure[2][2] = BLUE;
		} break;
		case BLOCK_TRIANGLE: {
			block.structure[1][1] = STEEL;
			block.structure[0][2] = STEEL;
			block.structure[1][2] = STEEL;
			block.structure[2][2] = STEEL;
		} break;
		case BLOCK_Z: {
			block.structure[0][1] = YELLOW;
			block.structure[1][1] = YELLOW;
			block.structure[1][2] = YELLOW;
			block.structure[2][2] = YELLOW;
		} break;
		case BLOCK_S: {
			block.structure[2][1] = GREEN;
			block.structure[1][1] = GREEN;
			block.structure[1][2] = GREEN;
			block.structure[0][2] = GREEN;
		} break;
		case BLOCK_J: {
			block.structure[1][1] = WHITE;
			block.structure[2][1] = WHITE;
			block.structure[2][2] = WHITE;
			block.structure[2][3] = WHITE;
		} break;
		case BLOCK_L: {
			block.structure[2][1] = PURPLE;
			block.structure[1][1] = PURPLE;
			block.structure[1][2] = PURPLE;
			block.structure[1][3] = PURPLE;
		}
	}
}

void TetrisPanel::MoveBlock(int x, int y) {
	if(BlockCollisionTest(x, y)) {
		if(y == 1) { // moving down...
			if(block.y < 1) { // Out of bounds!
				dead = true;
				gui.PopupDialog(wxT("Game Over"), wxT("You reached a score of ") + i2ws(score) + wxT("!"), wxOK);
				NewGame();
				SetFocus();
			} else {
				// Freeze the old block onto the map
				for(int y = 0; y < 4; ++y) {
					for(int x = 0; x < 4; ++x) {
						if(block.structure[x][y] != NO_COLOR) {
							map[block.x+x][block.y+y] = block.structure[x][y];
						}
					}
				}

				// Any cleared rows?
				int cleared = 0;
				for(int y = 0; y < TETRIS_MAPHEIGHT; ++y) {
					bool full = true;
					for(int x = 0; x < TETRIS_MAPWIDTH; ++x) {
						if(map[x][y] == NO_COLOR) {
							full = false;
						}
					}
					if(full) {
						RemoveRow(y);
						++cleared;
					}
				}
				AddScore(cleared);
				NewBlock();
			}
		} // If we're not moving down, we're not moving the block either
	} else {
		// No collision so move the block!
		block.x += x;
		block.y += y;
	}

	Refresh();
}

void TetrisPanel::RotateBlock() {
	Block temp;

	for(int y = 0; y < 4; ++y) {
		for(int x = 0; x < 4; ++x) {
			temp.structure[3-x][y] = block.structure[y][x];
		}
	}

	for(int y = 0; y < 4; ++y) {
		for(int x = 0; x < 4; ++x) {
			if(temp.structure[x][y] != NO_COLOR) {
				if(block.x + x < 0 || block.x + x > TETRIS_MAPWIDTH - 1 ||
					block.y + y < 0 || block.y + y > TETRIS_MAPHEIGHT - 1)
				{
					return;
				}
			}
		}
	}

	for(int y = 0; y < TETRIS_MAPWIDTH; ++y) {
		for(int x = 0; x < TETRIS_MAPHEIGHT; ++x) {
			if(x >= block.x && x < block.x + 4 && y >= block.y && y < block.y +4) {
				if(map[x][y] != NO_COLOR && temp.structure[x - block.x][y - block.y] != NO_COLOR) {
					return;
				}
			}
		}
	}
		
	for(int y = 0; y < 4; ++y) {
		for(int x = 0; x < 4; ++x) {
			block.structure[x][y] = temp.structure[x][y];
		}
	}
	
	Refresh();
}

//=============================================================================
// SnakePanel - A window with a Snake game!

SnakePanel::SnakePanel(wxWindow* parent) : GamePanel(parent, 16*SNAKE_MAPWIDTH, 16*SNAKE_MAPHEIGHT) {
	NewGame();
}

SnakePanel::~SnakePanel() {
}

void SnakePanel::Render(wxDC& pdc) {
	pdc.SetBackground(*wxBLACK_BRUSH);
	pdc.Clear();

	wxBrush snakebrush(wxColor(0, 0, 255));
	wxBrush applebrush(wxColor(255, 0, 0));
	
	double lblue  = 1.0;
	double lred   = 0.5;
	double lgreen = 0.0;

	for(int y = 0; y < SNAKE_MAPHEIGHT; ++y) {
		for(int x = 0; x < SNAKE_MAPWIDTH; ++x) {
			if(map[x][y] == -1) { // Apple
				pdc.SetBrush(applebrush);
				pdc.DrawRectangle(x*16, y*16, 16, 16);
			} else if(map[x][y] > 0) { // Snake
				double snook = double(map[x][y]) / length;
				snakebrush.SetColour(wxColor(
					int(255.0 * (1.0 - abs(lred - snook))),
					int(255.0 * (1.0 - abs(lgreen - snook))),
					int(255.0 * (1.0 - abs(lblue - snook)))
					)
				);
				pdc.SetBrush(snakebrush);
				pdc.DrawRectangle(x*16, y*16, 16, 16);
			}
		}
	} 
}

void SnakePanel::OnKey(wxKeyEvent& event, bool down) {
	if(!down) return;
	switch(event.GetKeyCode()) {
		case WXK_SPACE: {
			if(paused())
				unpause();
			else pause();
		} break;
		case WXK_NUMPAD_UP: case WXK_UP: {
			if(dead) return;
			unpause();
			Move(NORTH);
		} break;
		case WXK_NUMPAD_DOWN: case WXK_DOWN: {
			if(dead) return;
			unpause();
			Move(SOUTH);
		} break;
		case WXK_NUMPAD_LEFT: case WXK_LEFT: {
			if(dead) return;
			unpause();
			Move(WEST);
		} break;
		case WXK_NUMPAD_RIGHT: case WXK_RIGHT: {
			if(dead) return;
			unpause();
			Move(EAST);
		} break;
	}
}

void SnakePanel::NewGame() {
	length = 3;
	game_timer.Start();
	last_dir = NORTH;
	unpause();
	dead = false;

	// Clear map
	for(int y = 0; y < SNAKE_MAPHEIGHT; ++y) {
		for(int x = 0; x < SNAKE_MAPWIDTH; ++x) {
			map[x][y] = 0;
		}
	}
	map[SNAKE_MAPWIDTH / 2][SNAKE_MAPHEIGHT / 2] = length;
	NewApple();
	UpdateTitle(); // Update title
}

void SnakePanel::UpdateTitle() {
	wxString title = wxT("Remere's Snake : ");
	title << length << wxT(" segments");
	((wxTopLevelWindow*)GetParent())->SetTitle(title);
}

void SnakePanel::GameLoop(int time) {
	Move(last_dir);
}

void SnakePanel::NewApple() {
	bool possible = false;
	for(int y = 0; y < SNAKE_MAPHEIGHT; ++y) {
		for(int x = 0; x < SNAKE_MAPWIDTH; ++x) {
			if(map[x][y] == 0) {
				possible = true;
			}
			if(possible) break;
		}
		if(possible) break;
	}

	if(possible) {
		while(true) {
			int x = random(0, SNAKE_MAPWIDTH - 1);
			int y = random(0, SNAKE_MAPHEIGHT - 1);
			if(map[x][y] == 0) {
				map[x][y] = -1;
				break;
			}
		}
	}
}

void SnakePanel::Move(int dir) {
	if(
			(last_dir == NORTH && dir == SOUTH) ||
			(last_dir == WEST && dir == EAST) ||
			(last_dir == EAST && dir == WEST) ||
			(last_dir == SOUTH && dir == NORTH)) {
		return;
	}
	int nx = 0, ny = 0;
	int head_x = 0, head_y = 0;
	for(int y = 0; y < SNAKE_MAPHEIGHT; ++y) {
		for(int x = 0; x < SNAKE_MAPWIDTH; ++x) {
			if(map[x][y] ==  length) {
				head_x = x;
				head_y = y;
			}
		}
	}
	switch(dir) {
		case NORTH: {
			nx = head_x;
			ny = head_y - 1;
		} break;
		case SOUTH: {
			nx = head_x;
			ny = head_y + 1;
		} break;
		case WEST: {
			nx = head_x - 1;
			ny = head_y;
		} break;
		case EAST: {
			nx = head_x + 1;
			ny = head_y;
		} break;
		default: return;
	}
	
	if(map[nx][ny] > 0 || nx < 0 || ny < 0 || nx >= SNAKE_MAPWIDTH || ny >= SNAKE_MAPHEIGHT) {
		// Crash
		dead = true;
		gui.PopupDialog(wxT("Game Over"), wxT("You reached a length of ") + i2ws(length) + wxT("!"), wxOK);
		NewGame();
		SetFocus();
	} else {
		// Walk!
		if(map[nx][ny] == -1) {
			// Took apple!
			length += 1;
			UpdateTitle();
			NewApple();
		} else {
			for(int y = 0; y < SNAKE_MAPHEIGHT; ++y) {
				for(int x = 0; x < SNAKE_MAPWIDTH; ++x) {
					if(map[x][y] > 0) {
						map[x][y] -= 1;
					}
				}
			}
		}
		map[nx][ny] = length;
	}
	last_dir = dir;

	Refresh();
}
