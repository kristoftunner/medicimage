#include "bitmappane.h"

namespace app
{

SelectablePane::SelectablePane(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, bool highlightable)
    : wxWindow(parent, id, pos, size, wxFULL_REPAINT_ON_RESIZE), m_highlightable(highlightable)
{
    this->SetBackgroundStyle(wxBG_STYLE_PAINT);

    Bind(wxEVT_PAINT, &SelectablePane::OnPaint, this);
}

void SelectablePane::OnPaint(wxPaintEvent &event)
{
    wxAutoBufferedPaintDC dc(this);
    dc.SetBackground(wxBrush(GetParent()->GetBackgroundColour()));
    dc.Clear();

    auto gc = wxGraphicsContext::Create(dc);
    gc->SetCompositionMode(wxCOMPOSITION_OVER);
    if (gc)
    {
      wxRect selectionRect{ 0, 0, GetSize().GetWidth(), GetSize().GetHeight() };
      selectionRect.Deflate(FromDIP(1));

      wxRect contentRect = selectionRect;
      contentRect.Deflate(FromDIP(2));

      const auto roundness = FromDIP(4);

      DrawContent(gc, contentRect, roundness);

      if (!m_disabled)
      {
        if (m_selected)
        {
          wxBrush brush(*wxTRANSPARENT_BRUSH);
          if(m_highlightable)
            brush = wxBrush(wxColor{ 57, 200, 32, 128 }, wxBRUSHSTYLE_SOLID);
          gc->SetPen(*wxBLACK_PEN);
          gc->SetBrush(brush);
          gc->DrawRoundedRectangle(selectionRect.GetX(), selectionRect.GetY(), selectionRect.GetWidth(), selectionRect.GetHeight(), roundness);
        }

      }
      else
      {
        wxBrush brush(*wxTRANSPARENT_BRUSH);
        if(m_highlightable)
          brush = wxBrush(wxColor{ 128, 128, 128, 128 }, wxBRUSHSTYLE_SOLID);
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(brush);
        gc->DrawRoundedRectangle(selectionRect.GetX(), selectionRect.GetY(), selectionRect.GetWidth(), selectionRect.GetHeight(), roundness);
      }

      delete gc;
    }
}

BitmapPane::BitmapPane(wxBitmap bitmap, wxWindow *parent, wxWindowID id, const wxColor &paneColor,bool higlightable, const wxPoint &pos, const wxSize &size )
    : SelectablePane(parent, id, pos, size, higlightable), m_color(paneColor), m_bitmap(bitmap)
{
}

void BitmapPane::DrawContent(wxGraphicsContext *gc, const wxRect &rect, int roundness) const
{
    gc->SetPen(wxPen(m_color));
    gc->SetBrush(wxBrush(m_color));
    if(m_bitmap.IsOk())
      gc->DrawBitmap(m_bitmap, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
    else
      gc->DrawRoundedRectangle(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight(), roundness);
}


}