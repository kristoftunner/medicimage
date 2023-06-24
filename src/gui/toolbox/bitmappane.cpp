#include "bitmappane.h"

namespace app
{

SelectablePane::SelectablePane(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
    : wxWindow(parent, id, pos, size, wxFULL_REPAINT_ON_RESIZE)
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
      wxRect selectionRect{ 0, 0, GetSize().GetWidth(), GetSize().GetWidth() };
      selectionRect.Deflate(FromDIP(1));

      wxRect contentRect = selectionRect;
      contentRect.Deflate(FromDIP(2));

      const auto roundness = FromDIP(4);

      DrawContent(gc, contentRect, roundness);

      if (selectable)
      {
        if (selected)
        {
          wxBrush brush(wxColor{ 57, 200, 32, 128 }, wxBRUSHSTYLE_SOLID);
          gc->SetPen(*wxBLACK_PEN);
          gc->SetBrush(brush);
          gc->DrawRoundedRectangle(selectionRect.GetX(), selectionRect.GetY(), selectionRect.GetWidth(), selectionRect.GetHeight(), roundness);
        }

      }
      else
      {
        wxBrush brush(wxColor{ 128, 128, 128, 128 }, wxBRUSHSTYLE_SOLID);
        gc->SetPen(*wxTRANSPARENT_PEN);
        gc->SetBrush(brush);
        gc->DrawRoundedRectangle(selectionRect.GetX(), selectionRect.GetY(), selectionRect.GetWidth(), selectionRect.GetHeight(), roundness);
      }

      delete gc;
    }
}

BitmapPane::BitmapPane(wxBitmap bitmap, wxWindow *parent, wxWindowID id, const wxColor &paneColor, const wxPoint &pos, const wxSize &size)
    : SelectablePane(parent, id, pos, size), m_color(paneColor), m_bitmap(bitmap)
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