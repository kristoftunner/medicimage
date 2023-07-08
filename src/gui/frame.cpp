#include <wx/button.h>
#include <wx/log.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/print.h>
#include <wx/printdlg.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "frame.h"
#include "gui/editor/canvas.h"
#include "gui/editor/editor.h"
#include "gui/thumbnails/thumbnails.h"
#include "gui/toolbox/attribute_editor.h"
#include "gui/toolbox/attribute_editor_events.h"
#include "gui/toolbox/toolbox.h"
#include "gui/toolbox/toolbox_events.h"

namespace app
{

enum
{
    ID_Hello = 1
};

// TODO REFACTOR: refactor this ctor
MyFrame::MyFrame()
{
    Create(nullptr, wxID_ANY, "Frame");
    m_logger = new wxLogWindow(this, "Log", false, false);
    wxLog::SetActiveTarget(m_logger);
    bool isDark = wxSystemSettings::GetAppearance().IsDark();
    const auto margin = FromDIP(5);

    wxIcon icon;
    icon.LoadFile("icon.png", wxBITMAP_TYPE_PNG);
    SetIcon(icon);

    auto bitmap = wxBitmap("assets/wallpaper.png", wxBITMAP_TYPE_PNG);
    SetBackgroundBitmap(bitmap);
    auto mainSplitter =
        new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH);
    auto nestedSplitter =
        new wxSplitterWindow(mainSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH);
    auto toolboxSplitter = new wxSplitterWindow(nestedSplitter, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                                wxSP_LIVE_UPDATE | wxSP_3DSASH);
    mainSplitter->SetSashSize(FromDIP(50));
    nestedSplitter->SetSashSize(FromDIP(50));
    toolboxSplitter->SetSashSize(FromDIP(50));

    auto toolbox = new Toolbox(toolboxSplitter, wxID_ANY);
    m_editorPanel = new EditorPanel(nestedSplitter, wxID_ANY);
    auto thumbnails = new Thumbnails(mainSplitter, wxID_ANY);
    auto attributeEditor = new AttributeEditor(toolboxSplitter, thumbnails->GetDocumentController());

    // TODO: make styling somehow more centralized
    toolboxSplitter->SplitHorizontally(toolbox, attributeEditor, 350);
    toolboxSplitter->SetMinimumPaneSize(100);
    toolboxSplitter->SetDoubleBuffered(true);
    toolboxSplitter->SetSashGravity(0.7);

    nestedSplitter->SplitVertically(toolboxSplitter, m_editorPanel, 150);
    nestedSplitter->SetMinimumPaneSize(150);
    nestedSplitter->SetDoubleBuffered(true);
    nestedSplitter->SetSashGravity(0.0);

    // thumbnails->SetBackgroundColour(wxColour(isDark ? m_darkBackground : m_lightBackground));

    mainSplitter->SplitVertically(nestedSplitter, thumbnails, -200);
    mainSplitter->SetMinimumPaneSize(200);
    mainSplitter->SetDoubleBuffered(true);
    mainSplitter->SetSashGravity(1.0);

    wxBoxSizer *topsizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(mainSplitter, 1, wxEXPAND);

    wxMenu *menuFile = new wxMenu;
    menuFile->AppendSeparator();
    menuFile->Append(wxID_PRINT, "&Print...", "Print");
    menuFile->Append(wxID_EXIT);
    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenu *cameraSelectionMenu = new wxMenu;
    auto camera = m_editorPanel->GetCamera();
    auto &mutex = m_editorPanel->GetCameraMutex();
    for (int i = 0; i < camera->GetNumberOfDevices(); ++i)
    {
        auto item = cameraSelectionMenu->AppendCheckItem(i, camera->GetDeviceName(i));
        if (camera->GetSelectedDevices() && (i == camera->GetSelectedDevices().value()))
            cameraSelectionMenu->Check(i, true);

        Bind(
            wxEVT_MENU,
            [this, camera, cameraSelectionMenu, &mutex](wxCommandEvent &event) {
                auto id = event.GetId();
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    camera->Open(id);
                }

                for (int i = 0; i < cameraSelectionMenu->GetMenuItemCount(); i++)
                    cameraSelectionMenu->Check(i, false);
                cameraSelectionMenu->Check(id, true);
            },
            item->GetId());
    }

    wxMenu *settings = new wxMenu;
    wxMenuItem *cameraSelection = new wxMenuItem(settings, wxID_ANY, "Camera selection");
    cameraSelection->SetSubMenu(cameraSelectionMenu);
    settings->Append(cameraSelection);
    wxMenuItem *appFolderConfig = new wxMenuItem(settings, wxID_ANY, "Application folder");
    Bind(
        wxEVT_MENU,
        [this, thumbnails](wxCommandEvent &event) {
            wxDirDialog dlg(this, "Choose application folder", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
            if (dlg.ShowModal() == wxID_OK)
            {
                auto folder = dlg.GetPath().ToStdString();
                wxLogDebug("Selected path: %s", dlg.GetPath());
                AppFolderUpdateEvent updateEvent(EVT_THUMBNAILS_APP_FOLDER_UPDATE, wxID_ANY);
                updateEvent.SetData(folder);
                wxPostEvent(thumbnails, updateEvent);
            }
        },
        appFolderConfig->GetId());
    settings->Append(appFolderConfig);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    menuBar->Append(settings, "&Settings");
    SetMenuBar(menuBar);

    Bind(wxEVT_MENU, &MyFrame::OnPrint, this, wxID_PRINT);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);

    // Events generated by toolbox and relayed to canvas
    auto relayToolboxButtonEvent = [this](ToolboxButtonEvent &event) { wxPostEvent(this->m_editorPanel, event); };
    Bind(TOOLBOX_BUTTON_PUSHED, relayToolboxButtonEvent);

    // Events generated by the attribute editor and relayed to the editor
    Bind(EVT_ENTITY_ATTRIBUTE_EDITED, [this](EntityEvent &event) { wxPostEvent(this->m_editorPanel, event); });
    Bind(EVT_PATIENT_SELECTED, [thumbnails, this](PatientSelectedEvent &event) {
        wxPostEvent(thumbnails, event);
        wxPostEvent(m_editorPanel, event);
    });

    // Events generated by canvas/editorpanel and relayed to thumbnails
    auto relayCanvasEvent = [this, thumbnails](ImageDocumentEvent &event) { wxPostEvent(thumbnails, event); };
    Bind(EDITOR_DELETE_DOCUMENT, relayCanvasEvent);
    Bind(EDITOR_SAVE_DOCUMENT, relayCanvasEvent);
    Bind(EDITOR_ADD_DOCUMENT, relayCanvasEvent);
    Bind(EVT_THUMBNAILS_ADD_PATIENT,
         [this, thumbnails](PatientSelectedEvent &event) { wxPostEvent(thumbnails, event); });
    Bind(EDITOR_ENTITY_CHANGED, [this, attributeEditor](EntityEvent &event) { wxPostEvent(attributeEditor, event); });

    // events generated by canvas/editorpanel and relayed to toolbox
    auto relayCanvasButtonEvent = [this, toolbox](ToolboxButtonEvent &event) { wxPostEvent(toolbox, event); };
    auto relayCanvasButtonDisableEvent = [this, toolbox](ToolboButtonStateUpdateEvent &event) {
        wxPostEvent(toolbox, event);
    };
    Bind(TOOLBOX_BUTTON_COMMAND_DONE, relayCanvasButtonEvent);
    Bind(TOOLBOX_BUTTTON_DISABLED, relayCanvasButtonDisableEvent);

    // Events generated by thumbnails and relayed to canvas
    auto relayThumbnailEvent = [this](ImageDocumentEvent &event) { wxPostEvent(this->m_editorPanel, event); };
    Bind(EVT_THUMBNAILS_DOCUMENT_PICK, relayThumbnailEvent);
    Bind(EVT_PATIENT_UPDATED,
         [this, attributeEditor](UpdatePatientsEvent &event) { wxPostEvent(attributeEditor, event); });
}

void MyFrame::OnExit(wxCommandEvent &event)
{
    Close(true);
}

void MyFrame::OnAbout(wxCommandEvent &event)
{
    wxMessageBox("This is a wxWidgets Hello World example", "About Hello World", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnHello(wxCommandEvent &event)
{
    wxLogMessage("Hello world from wxWidgets!");
}

void MyFrame::OnPrint(wxCommandEvent &event)
{
    std::unique_ptr<wxPrintData> printData = std::make_unique<wxPrintData>();
    printData->SetPaperId(wxPAPER_A4);

    wxPrintDialogData printDialogData(*(printData.get()));

    wxPrinter printer(&printDialogData);
    Printout printout = Printout(m_editorPanel->GetCanvas(), "My printout");
    if (!printer.Print(this, &printout, true /*prompt*/))
    {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
        {
            wxLogError("There was a problem printing. Perhaps your current printer is not set correctly?");
        }
        else
        {
            wxLogMessage("You canceled printing");
        }
    }
    else
    {
        auto newPrintData = printer.GetPrintDialogData().GetPrintData();
    }
}

bool Printout::OnPrintPage(int page)
{
    wxDC *dc = GetDC();
    if (dc)
    {
        DrawPage();

        // Draw page numbers at top left corner of printable area, sized so that
        // screen size of text matches paper size.
        MapScreenSizeToPage();
        return true;
    }
    else
        return false;
}

bool Printout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return false;

    return true;
}

void Printout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = 1;
    *selPageFrom = 1;
    *selPageTo = 1;
}

bool Printout::HasPage(int pageNum)
{
    return pageNum == 1;
}

void Printout::DrawPage()
{
    // We know the graphic is 230x350. If we didn't know this, we'd need to
    // calculate it.
    wxCoord maxX = GetDC()->FromDIP(800);
    wxCoord maxY = GetDC()->FromDIP(600);

    // This sets the user scale and origin of the DC so that the image fits
    // within the page rectangle, which is the printable area on Mac and MSW
    // and is the entire page on other platforms.
    FitThisSizeToPage(wxSize(maxX, maxY));
    wxRect fitRect = GetLogicalPageRect();

    // This offsets the image so that it is centered within the reference
    // rectangle defined above.
    wxCoord xoff = (fitRect.width - maxX) / 2;
    wxCoord yoff = (fitRect.height - maxY) / 2;
    OffsetLogicalOrigin(xoff, yoff);

    m_canvas->Draw(*GetDC());
}
} // namespace app