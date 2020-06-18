#include "main.h"
#include "welcome_dialog.h"
#include "settings.h"
#include "preferences.h"

wxDEFINE_EVENT(WELCOME_DIALOG_ACTION, wxCommandEvent);

WelcomeDialog::WelcomeDialog(const wxString &title_text,
                             const wxString &version_text,
                             const wxBitmap &rme_logo,
                             const std::vector<wxString> &recent_files)
        : wxDialog(nullptr, wxID_ANY, "", wxDefaultPosition, wxSize(800, 450)) {
    Centre();
    wxColour base_colour = wxColor(250, 250, 250);
    m_welcome_dialog_panel = newd WelcomeDialogPanel(this,
                                                   GetClientSize(),
                                                   title_text,
                                                   version_text,
                                                   base_colour,
                                                   rme_logo,
                                                   recent_files);
}

void WelcomeDialog::OnButtonClicked(wxMouseEvent &event) {
    auto *button = dynamic_cast<WelcomeDialogButton *>(event.GetEventObject());
    wxSize button_size = button->GetSize();
    wxPoint click_point = event.GetPosition();
    if (click_point.x > 0 && click_point.x < button_size.x && click_point.y > 0 && click_point.y < button_size.x) {
        if (button->GetAction() == wxID_PREFERENCES) {
            PreferencesWindow preferences_window(m_welcome_dialog_panel, true);
            preferences_window.ShowModal();
        } else {
            wxCommandEvent action_event(WELCOME_DIALOG_ACTION);
            if (button->GetAction() == wxID_OPEN) {
                wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ?
                                    "(*.otbm;*.otgz)|*.otbm;*.otgz" :
                                    "(*.otbm)|*.otbm|Compressed OpenTibia Binary Map (*.otgz)|*.otgz";
                wxFileDialog file_dialog(this, "Open map file", "", "", wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
                if (file_dialog.ShowModal() == wxID_OK) {
                    action_event.SetString(file_dialog.GetPath());
                } else {
                    return;
                }
            }
            action_event.SetId(button->GetAction());
            ProcessWindowEvent(action_event);
        }
    }
}

void WelcomeDialog::OnRecentItemClicked(wxMouseEvent &event) {
    auto *recent_item = dynamic_cast<RecentItem *>(event.GetEventObject());
    wxSize button_size = recent_item->GetSize();
    wxPoint click_point = event.GetPosition();
    if (click_point.x > 0 && click_point.x < button_size.x && click_point.y > 0 && click_point.y < button_size.x) {
        wxCommandEvent action_event(WELCOME_DIALOG_ACTION);
        action_event.SetString(recent_item->GetText());
        action_event.SetId(wxID_OPEN);
        ProcessWindowEvent(action_event);
    }
}

WelcomeDialogPanel::WelcomeDialogPanel(WelcomeDialog *dialog,
                                       const wxSize &size,
                                       const wxString &title_text,
                                       const wxString &version_text,
                                       const wxColour &base_colour,
                                       const wxBitmap &rme_logo,
                                       const std::vector<wxString> &recent_files)
        : wxPanel(dialog),
          m_rme_logo(rme_logo),
          m_title_text(title_text),
          m_version_text(version_text),
          m_text_colour(base_colour.ChangeLightness(40)) {

    SetBackgroundColour(base_colour);

    newd RecentMapsPanel(this,
                         dialog,
                         wxPoint(size.x / 2, 0),
                         wxSize(size.x / 2, size.y),
                         base_colour,
                         recent_files);

    wxSize button_size(150, 35);
    wxColour button_base_colour = base_colour.ChangeLightness(90);

    int button_pos_center_x = size.x / 4 - button_size.x / 2;
    int button_pos_center_y = size.y / 2;

    wxPoint newMapButtonPoint(button_pos_center_x, button_pos_center_y);
    auto *new_map_button = newd WelcomeDialogButton(this,
                                                  newMapButtonPoint,
                                                  button_size,
                                                  button_base_colour,
                                                  "New");
    new_map_button->SetAction(wxID_NEW);
    new_map_button->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnButtonClicked, dialog);

    wxPoint open_map_button_point(button_pos_center_x, newMapButtonPoint.y + button_size.y + 10);
    auto *open_map_button = newd WelcomeDialogButton(this,
                                                   open_map_button_point,
                                                   button_size,
                                                   button_base_colour,
                                                   "Open");
    open_map_button->SetAction(wxID_OPEN);
    open_map_button->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnButtonClicked, dialog);

    wxPoint preferences_button_point(button_pos_center_x, open_map_button_point.y + button_size.y + 10);
    auto *preferences_button = newd WelcomeDialogButton(this,
                                                       preferences_button_point,
                                                       button_size,
                                                       button_base_colour,
                                                       "Preferences");
    preferences_button->SetAction(wxID_PREFERENCES);
    preferences_button->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnButtonClicked, dialog);

    Bind(wxEVT_PAINT, &WelcomeDialogPanel::OnPaint, this);
}

void WelcomeDialogPanel::OnPaint(wxPaintEvent &event) {
    wxPaintDC dc(this);

    dc.DrawBitmap(m_rme_logo, wxPoint(GetSize().x / 4 - m_rme_logo.GetWidth() / 2, 40), true);

    wxFont font = GetFont();
    font.SetPointSize(21);
    dc.SetFont(font);
    wxSize header_size = dc.GetTextExtent(m_title_text);
    wxSize header_point(GetSize().x / 4, GetSize().y / 4);
    dc.SetTextForeground(m_text_colour);
    dc.DrawText(m_title_text, wxPoint(header_point.x - header_size.x / 2, header_point.y));

    dc.SetFont(GetFont().Larger());
    wxSize version_size = dc.GetTextExtent(m_version_text);
    dc.SetTextForeground(m_text_colour.ChangeLightness(110));
    dc.DrawText(m_version_text, wxPoint(header_point.x - version_size.x / 2, header_point.y + header_size.y + 10));
}

WelcomeDialogButton::WelcomeDialogButton(wxWindow *parent,
                                         const wxPoint &pos,
                                         const wxSize &size,
                                         const wxColour &base_colour,
                                         const wxString &text)
        : wxPanel(parent, wxID_ANY, pos, size),
          m_action(wxID_CLOSE),
          m_text(text),
          m_text_colour(base_colour.ChangeLightness(40)),
          m_background(base_colour.ChangeLightness(96)),
          m_background_hover(base_colour.ChangeLightness(93)) {
    SetBackgroundColour(m_background);

    Bind(wxEVT_PAINT, &WelcomeDialogButton::OnPaint, this);
    Bind(wxEVT_ENTER_WINDOW, &WelcomeDialogButton::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &WelcomeDialogButton::OnMouseLeave, this);
}

void WelcomeDialogButton::OnPaint(wxPaintEvent &event) {
    wxPaintDC dc(this);
    dc.SetFont(GetFont());
    dc.SetTextForeground(m_text_colour);
    wxSize text_size = dc.GetTextExtent(m_text);

    dc.DrawText(m_text, wxPoint(GetSize().x / 2 - text_size.x / 2, GetSize().y / 2 - text_size.y / 2));
}

void WelcomeDialogButton::OnMouseEnter(wxMouseEvent &event) {
    SetBackgroundColour(m_background_hover);
    Refresh();
}

void WelcomeDialogButton::OnMouseLeave(wxMouseEvent &event) {
    SetBackgroundColour(m_background);
    Refresh();
}

RecentMapsPanel::RecentMapsPanel(wxWindow *parent,
                                 WelcomeDialog *dialog,
                                 const wxPoint &pos,
                                 const wxSize &size,
                                 const wxColour &base_colour,
                                 const std::vector<wxString> &recent_files)
        : wxPanel(parent, wxID_ANY, pos, size) {
    SetBackgroundColour(base_colour.ChangeLightness(99));

    int height = 40;
    int position = 0;
    for (const wxString &file : recent_files) {
        auto *recentItem = newd RecentItem(this, wxPoint(0, position), wxSize(size.x, height), base_colour, file);
        recentItem->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnRecentItemClicked, dialog);
        position += height;
    }
}

RecentItem::RecentItem(wxWindow *parent,
                       const wxPoint &pos,
                       const wxSize &size,
                       const wxColour &base_colour,
                       const wxString &item_name)
        : wxPanel(parent, wxID_ANY, pos, size),
          m_base_colour(base_colour),
          m_text_colour(base_colour.ChangeLightness(40)),
          m_background(base_colour.ChangeLightness(96)),
          m_background_hover(base_colour.ChangeLightness(93)),
          m_item_text(item_name) {
    SetBackgroundColour(m_background);

    Bind(wxEVT_PAINT, &RecentItem::OnPaint, this);
    Bind(wxEVT_ENTER_WINDOW, &RecentItem::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &RecentItem::OnMouseLeave, this);
}

void RecentItem::OnPaint(wxPaintEvent &event) {
    wxPaintDC dc(this);

    dc.SetPen(wxPen(m_base_colour.ChangeLightness(99), 1));
    dc.DrawLine(0, 0, GetSize().x, 0);

    dc.SetFont(GetFont());
    dc.SetTextForeground(m_text_colour);
    wxSize text_size = dc.GetTextExtent(m_item_text);

    int x, y = GetSize().y / 2 - text_size.y / 2, padding = GetSize().y / 4;
    if (text_size.x + padding * 2 > GetSize().x) {
        x = GetSize().x - text_size.x - padding;
    } else {
        x = padding;
    }

    dc.DrawText(m_item_text, wxPoint(x, y));
}

void RecentItem::OnMouseEnter(wxMouseEvent &event) {
    SetBackgroundColour(m_background_hover);
    Refresh();
}

void RecentItem::OnMouseLeave(wxMouseEvent &event) {
    SetBackgroundColour(m_background);
    Refresh();
}
