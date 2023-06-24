#pragma once

#include <wx/wx.h>
#include <wx/graphics.h>
#include <wx/settings.h>
#include <wx/dcbuffer.h>
#include <wx/bitmap.h>

namespace app
{
class SelectablePane : public wxWindow
{
public:
    SelectablePane(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);

    wxSize DoGetBestSize() const override
    {
        return FromDIP(wxSize(45, 45));
    }

    bool selectable = true;
    bool selected = false;

protected:
    virtual void DrawContent(wxGraphicsContext *gc, const wxRect &rect, int roundness) const = 0;

private:
    void OnPaint(wxPaintEvent &event);
};

class BitmapPane : public SelectablePane
{
public:
    BitmapPane(wxBitmap bitmap, wxWindow *parent, wxWindowID id, const wxColor &paneColor, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize);

    wxColour m_color;
    wxBitmap m_bitmap;
private:
    virtual void DrawContent(wxGraphicsContext *gc, const wxRect &rect, int roundness) const override;
};

using BitmapButtonHandler = std::pair<wxBitmap, std::function<void()>>;
}
