#ifndef WELCOME_DIALOG_H
#define WELCOME_DIALOG_H

#include <wx/wx.h>

wxDECLARE_EVENT(WELCOME_DIALOG_ACTION, wxCommandEvent);

class WelcomeDialogPanel;

class WelcomeDialog : public wxDialog
{
public:
    WelcomeDialog(const wxString& titleText,
            const wxString& versionText,
            const wxBitmap& rmeLogo,
            const std::vector<wxString> &recentFiles);
    void OnButtonClicked(wxMouseEvent& event);
    void OnCheckboxClicked(const wxCommandEvent& event);
    void OnRecentItemClicked(wxMouseEvent& event);
private:
    WelcomeDialogPanel* m_welcome_dialog_panel;
};

class WelcomeDialogPanel : public wxPanel
{
public:
    WelcomeDialogPanel(WelcomeDialog* parent,
            const wxSize& size,
            const wxString& title_text,
            const wxString& version_text,
            const wxColour& base_colour,
            const wxBitmap& rme_logo,
            const std::vector<wxString> &recent_files);
    void OnPaint(wxPaintEvent& event);
    void updateInputs();
private:
    wxBitmap m_rme_logo;
    wxString m_title_text;
    wxString m_version_text;
    wxColour m_text_colour;
    wxCheckBox* m_show_welcome_window_checkbox;
};

class WelcomeDialogButton : public wxPanel
{
public:
    WelcomeDialogButton(wxWindow* parent, const wxPoint& pos, const wxSize& size, const wxColour& base_colour, const wxString &text);
    void OnPaint(wxPaintEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    wxStandardID GetAction() { return m_action; };
    void SetAction(wxStandardID action) { m_action = action; };
private:
    wxStandardID m_action;
    wxString m_text;
    wxColour m_text_colour;
    wxColour m_background;
    wxColour m_background_hover;
};

class RecentMapsPanel : public wxPanel
{
public:
    RecentMapsPanel(wxWindow* parent,
            WelcomeDialog* dialog,
            const wxPoint& pos,
            const wxSize& size,
            const wxColour& base_colour,
            const std::vector<wxString> &recent_files);
};

class RecentItem : public wxPanel
{
public:
    RecentItem(wxWindow* parent, const wxPoint& pos, const wxSize& size, const wxColour& base_colour, const wxString &item_name);
    void OnPaint(wxPaintEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    wxString GetText() { return m_item_text; };
private:
    wxColour m_base_colour;
    wxColour m_text_colour;
    wxColour m_background;
    wxColour m_background_hover;
    wxString m_item_text;
};

#endif //WELCOME_DIALOG_H
