#pragma once
#if 0

#include "drawing/drawing_sheet.h"
namespace medicimage
{

class AttributeEditor
{
public:
  AttributeEditor() = default;
  AttributeEditor(DrawingSheet* sheet) : m_sheet(sheet){}

  void OnImguiRender();
private:
  void DrawAttibuteEdit(Entity entity);

private:
  DrawingSheet* m_sheet;  // TODO: have a shared_ptr or something??
};

} // namespace medicimage
#endif