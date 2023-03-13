#include "ui/attribute_editor.h"
#include "attribute_editor.h"
#include "drawing/drawing_sheet.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

namespace medicimage
{

void medicimage::AttributeEditor::OnImguiRender()
{
  ImGui::Begin("Properties");

  auto selectedEntities = m_sheet->GetSelectedEntities();
  if(selectedEntities.size() != 0)
  {
    for(auto& entity : selectedEntities)
    {
      DrawAttributeEdit(entity);
    }
  } 

  ImGui::End();
}

void AttributeEditor::DrawAttributeEdit(Entity entity)
{
  if(entity.HasComponent<ColorComponent>())
  {
    auto& component = entity.GetComponent<ColorComponent>(); 
    ImGui::ColorEdit4("Color", glm::value_ptr(component.color));
  }
  else if(entity.HasComponent<SkinTemplateComponent>())
  {
    //SkinTemplateWrapper st(entity);
    //auto verticalSliceWidthBounds = st.GetVerticalSliceWidthBounds();
    //auto verticalSliceHeightBounds = st.GetVerticalSliceHeightBounds();
    //auto horizontalSliceWidthBounds = st.GetHorizontalSliceWidthBounds();
    //auto horizontalSliceHeightBounds = st.GetHorizontalSliceHeightBounds(); 
  }
}

} // namespace medicimage
