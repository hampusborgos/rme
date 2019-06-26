#include "main.h"
#include "welcome_dialog.h"
#include "settings.h"
#include "preferences.h"

WelcomeDialog::WelcomeDialog(const wxString &titleText,
                             const wxString &versionText,
                             const wxBitmap &rmeLogo,
                             const std::vector<wxString> &recentFiles)
        : wxDialog(nullptr, wxID_ANY, "", wxDefaultPosition, wxSize(800, 450)),
          m_recentMapPath("") {
    Centre();
    wxColour baseColour = wxColor(250, 250, 250);
    m_welcomeDialogPanel = newd WelcomeDialogPanel(this,
                                                   GetClientSize(),
                                                   titleText,
                                                   versionText,
                                                   baseColour,
                                                   rmeLogo,
                                                   recentFiles);
}

void WelcomeDialog::OnButtonClicked(wxMouseEvent &event) {
    auto *button = dynamic_cast<WelcomeDialogButton *>(event.GetEventObject());
    wxSize buttonSize = button->GetSize();
    wxPoint clickPoint = event.GetPosition();
    if (clickPoint.x > 0 && clickPoint.x < buttonSize.x && clickPoint.y > 0 && clickPoint.y < buttonSize.x) {
        if (button->GetAction() == wxID_OPEN) {
            wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ?
                                "(*.otbm;*.otgz)|*.otbm;*.otgz" :
                                "(*.otbm)|*.otbm|Compressed OpenTibia Binary Map (*.otgz)|*.otgz";
            wxFileDialog fileDialog(this, "Open map file", "", "", wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
            if (fileDialog.ShowModal() == wxID_OK) {
                m_recentMapPath = fileDialog.GetPath();
                EndModal(wxID_FILE);
            }
        } else if (button->GetAction() == wxID_PREFERENCES) {
            PreferencesWindow preferencesWindow(m_welcomeDialogPanel, true);
            preferencesWindow.ShowModal();
        } else {
            EndModal(button->GetAction());
        }
    }
}

void WelcomeDialog::OnRecentItemClicked(wxMouseEvent &event) {
    auto *recentItem = dynamic_cast<RecentItem *>(event.GetEventObject());
    wxSize buttonSize = recentItem->GetSize();
    wxPoint clickPoint = event.GetPosition();
    if (clickPoint.x > 0 && clickPoint.x < buttonSize.x && clickPoint.y > 0 && clickPoint.y < buttonSize.x) {
        m_recentMapPath = recentItem->GetText();
        EndModal(wxID_FILE);
    }
}

WelcomeDialogPanel::WelcomeDialogPanel(WelcomeDialog *dialog,
                                       const wxSize &size,
                                       const wxString &titleText,
                                       const wxString &versionText,
                                       const wxColour &baseColour,
                                       const wxBitmap &rmeLogo,
                                       const std::vector<wxString> &recentFiles)
        : wxPanel(dialog),
          m_rmeLogo(rmeLogo),
          m_titleText(titleText),
          m_versionText(versionText),
          m_textColour(baseColour.ChangeLightness(40)) {

    SetBackgroundColour(baseColour);

    newd RecentMapsPanel(this,
                         dialog,
                         wxPoint(size.x / 2, 0),
                         wxSize(size.x / 2, size.y),
                         baseColour,
                         recentFiles);

    wxSize buttonSize(150, 35);
    wxColour buttonBaseColour = baseColour.ChangeLightness(90);

    int buttonPosCenterX = size.x / 4 - buttonSize.x / 2;
    int buttonPosCenterY = size.y / 2;

    wxPoint newMapButtonPoint(buttonPosCenterX, buttonPosCenterY);
    auto *newMapButton = newd WelcomeDialogButton(this,
                                                  newMapButtonPoint,
                                                  buttonSize,
                                                  buttonBaseColour,
                                                  "New");
    newMapButton->SetAction(wxID_NEW);
    newMapButton->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnButtonClicked, dialog);

    wxPoint openMapButtonPoint(buttonPosCenterX, newMapButtonPoint.y + buttonSize.y + 10);
    auto *openMapButton = newd WelcomeDialogButton(this,
                                                   openMapButtonPoint,
                                                   buttonSize,
                                                   buttonBaseColour,
                                                   "Open");
    openMapButton->SetAction(wxID_OPEN);
    openMapButton->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnButtonClicked, dialog);

    wxPoint preferencesButtonPoint(buttonPosCenterX, openMapButtonPoint.y + buttonSize.y + 10);
    auto *preferencesButton = newd WelcomeDialogButton(this,
                                                       preferencesButtonPoint,
                                                       buttonSize,
                                                       buttonBaseColour,
                                                       "Preferences");
    preferencesButton->SetAction(wxID_PREFERENCES);
    preferencesButton->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnButtonClicked, dialog);

    Bind(wxEVT_PAINT, &WelcomeDialogPanel::OnPaint, this);
}

void WelcomeDialogPanel::OnPaint(wxPaintEvent &event) {
    wxPaintDC dc(this);

    dc.DrawBitmap(m_rmeLogo, wxPoint(GetSize().x / 4 - m_rmeLogo.GetWidth() / 2, 40), true);

    wxFont font = GetFont();
    font.SetPointSize(21);
    dc.SetFont(font);
    wxSize headerSize = dc.GetTextExtent(m_titleText);
    wxSize headerPoint(GetSize().x / 4, GetSize().y / 4);
    dc.SetTextForeground(m_textColour);
    dc.DrawText(m_titleText, wxPoint(headerPoint.x - headerSize.x / 2, headerPoint.y));

	dc.SetFont(GetFont().Larger());
    wxSize versionSize = dc.GetTextExtent(m_versionText);
    dc.SetTextForeground(m_textColour.ChangeLightness(110));
    dc.DrawText(m_versionText, wxPoint(headerPoint.x - versionSize.x / 2, headerPoint.y + headerSize.y + 10));
}

WelcomeDialogButton::WelcomeDialogButton(wxWindow *parent,
                                         const wxPoint &pos,
                                         const wxSize &size,
                                         const wxColour &baseColour,
                                         const wxString &text)
        : wxPanel(parent, wxID_ANY, pos, size),
          m_action(wxID_CLOSE),
          m_text(text),
          m_textColour(baseColour.ChangeLightness(40)),
          m_background(baseColour.ChangeLightness(96)),
          m_backgroundHover(baseColour.ChangeLightness(93)) {
    SetBackgroundColour(m_background);

    Bind(wxEVT_PAINT, &WelcomeDialogButton::OnPaint, this);
    Bind(wxEVT_ENTER_WINDOW, &WelcomeDialogButton::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &WelcomeDialogButton::OnMouseLeave, this);
}

void WelcomeDialogButton::OnPaint(wxPaintEvent &event) {
    wxPaintDC dc(this);
    dc.SetFont(GetFont());
    dc.SetTextForeground(m_textColour);
    wxSize textSize = dc.GetTextExtent(m_text);

    dc.DrawText(m_text, wxPoint(GetSize().x / 2 - textSize.x / 2, GetSize().y / 2 - textSize.y / 2));
}

void WelcomeDialogButton::OnMouseEnter(wxMouseEvent &event) {
    SetBackgroundColour(m_backgroundHover);
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
                                 const wxColour &baseColour,
                                 const std::vector<wxString> &recentFiles)
        : wxPanel(parent, wxID_ANY, pos, size) {
    SetBackgroundColour(baseColour.ChangeLightness(99));

    int height = 40;
    int position = 0;
    for (const wxString &file : recentFiles) {
        auto *recentItem = newd RecentItem(this, wxPoint(0, position), wxSize(size.x, height), baseColour, file);
        recentItem->Bind(wxEVT_LEFT_UP, &WelcomeDialog::OnRecentItemClicked, dialog);
        position += height;
    }
}

RecentItem::RecentItem(wxWindow *parent,
                       const wxPoint &pos,
                       const wxSize &size,
                       const wxColour &baseColour,
                       const wxString &itemName)
        : wxPanel(parent, wxID_ANY, pos, size),
          m_baseColour(baseColour),
          m_textColour(baseColour.ChangeLightness(40)),
          m_background(baseColour.ChangeLightness(96)),
          m_backgroundHover(baseColour.ChangeLightness(93)),
          m_itemText(itemName) {
    SetBackgroundColour(m_background);

    Bind(wxEVT_PAINT, &RecentItem::OnPaint, this);
    Bind(wxEVT_ENTER_WINDOW, &RecentItem::OnMouseEnter, this);
    Bind(wxEVT_LEAVE_WINDOW, &RecentItem::OnMouseLeave, this);
}

void RecentItem::OnPaint(wxPaintEvent &event) {
    wxPaintDC dc(this);

    dc.SetPen(wxPen(m_baseColour.ChangeLightness(99), 1));
    dc.DrawLine(0, 0, GetSize().x, 0);

    dc.SetFont(GetFont());
    dc.SetTextForeground(m_textColour);
    wxSize textSize = dc.GetTextExtent(m_itemText);

    int x, y = GetSize().y / 2 - textSize.y / 2, padding = GetSize().y / 4;
    if (textSize.x + padding * 2 > GetSize().x) {
        x = GetSize().x - textSize.x - padding;
    } else {
        x = padding;
    }

    dc.DrawText(m_itemText, wxPoint(x, y));
}

void RecentItem::OnMouseEnter(wxMouseEvent &event) {
    SetBackgroundColour(m_backgroundHover);
    Refresh();
}

void RecentItem::OnMouseLeave(wxMouseEvent &event) {
    SetBackgroundColour(m_background);
    Refresh();
}
