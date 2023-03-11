#pragma once

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
  void DrawAttributeEdit(Entity entity);

private:
  DrawingSheet* m_sheet;  // TODO: have a shared_ptr or something??
};

} // namespace medicimage
