
#ifndef RME_WELCOME_DIALOG_H
#define RME_WELCOME_DIALOG_H

class WelcomeDialog : public wxDialog
{
public:
    WelcomeDialog(const wxString& titleText,
            const wxString& versionText,
            const wxBitmap& rmeLogo,
            const std::vector<wxString> &recentFiles);
    wxString GetChosenMapPath(){ return m_recentMapPath; };
    void OnButtonClicked(wxMouseEvent& event);
    void OnRecentItemClicked(wxMouseEvent& event);
private:
    wxString m_recentMapPath;
    wxPanel* m_welcomeDialogPanel;
};

class WelcomeDialogPanel : public wxPanel
{
public:
    WelcomeDialogPanel(WelcomeDialog* parent,
            const wxSize& size,
            const wxString& titleText,
            const wxString& versionText,
            const wxColour& baseColour,
            const wxBitmap& rmeLogo,
            const std::vector<wxString> &recentFiles);
    void OnPaint(wxPaintEvent& event);
private:
    wxBitmap m_rmeLogo;
    wxFont m_font;
    wxString m_titleText;
    wxString m_versionText;
    wxColour m_textColour;
};

class WelcomeDialogButtonButton : public wxPanel
{
public:
    WelcomeDialogButtonButton(wxWindow* parent, const wxPoint& pos, const wxSize& size, const wxColour& baseColour, const wxString &text);
    void OnPaint(wxPaintEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    wxStandardID GetAction() { return m_action; };
    void SetAction(wxStandardID action) { m_action = action; };
private:
    wxStandardID m_action;
    wxFont m_font;
    wxString m_text;
    wxColour m_textColour;
    wxColour m_background;
    wxColour m_backgroundHover;
};

class RecentMapsPanel : public wxPanel
{
public:
    RecentMapsPanel(wxWindow* parent,
            WelcomeDialog* dialog,
            const wxPoint& pos,
            const wxSize& size,
            const wxColour& baseColour,
            const std::vector<wxString> &recentFiles);
};

class RecentItem : public wxPanel
{
public:
    RecentItem(wxWindow* parent, const wxPoint& pos, const wxSize& size, const wxColour& baseColour, const wxString &itemName);
    void OnPaint(wxPaintEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    wxString GetText() { return m_itemText; };
private:
    wxColour m_baseColour;
    wxColour m_textColour;
    wxColour m_background;
    wxColour m_backgroundHover;
    wxString m_itemText;
};

#endif //RME_WELCOME_DIALOG_H
