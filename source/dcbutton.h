//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_DC_BUTTON_H
#define RME_DC_BUTTON_H

class Sprite;
class GameSprite;
class EditorSprite;

enum {
	DC_BTN_NORMAL,
	DC_BTN_TOGGLE,
};

enum RenderSize {
	RENDER_SIZE_16x16,
	RENDER_SIZE_32x32,
	RENDER_SIZE_64x64,
};

class DCButton : public wxPanel
{
public:
	DCButton();
	DCButton(wxWindow* parent, wxWindowID id, wxPoint pos, int type, RenderSize sz, int sprite_id);
	virtual ~DCButton();

	void SetValue(bool val);
	bool GetValue() const;

	void SetSprite(int id);

	void OnPaint(wxPaintEvent&);
	void OnClick(wxMouseEvent&);
protected:
	void SetOverlay(Sprite* espr);

	int type;
	bool state; // pressed/unpressed
	RenderSize size;
	Sprite* sprite;
	Sprite* overlay;

	DECLARE_DYNAMIC_CLASS(DCButton)
	DECLARE_EVENT_TABLE()
};

#endif
