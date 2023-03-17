#include "ui/attribute_editor.h"
#include "attribute_editor.h"
#include "drawing/drawing_sheet.h"
#include "drawing/component_wrappers.h"
#include "core/log.h"

#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

namespace medicimage
{

void medicimage::AttributeEditor::OnImguiRender()
{
  bool open = true;
  ImGui::Begin("Properties", &open, ImGuiWindowFlags_None);

  auto selectedEntities = m_sheet->GetSelectedEntities();
  if(selectedEntities.size() != 0)
  {
    auto& entity = selectedEntities[0];
    DrawAttributeEdit(entity);
  } 

  ImGui::End();
}
template<typename T, typename UIFunction>
static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
{
  const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_None;//ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap;
  if (entity.HasComponent<T>())
  {
    auto& component = entity.GetComponent<T>();
    ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
    ImGui::Separator();
    bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
    if (open)
    {
      uiFunction(component);
      ImGui::TreePop();
    }
  }
}

void AttributeEditor::DrawAttributeEdit(Entity entity)
{
  DrawComponent<ColorComponent>("Color", entity, [&](auto& component)
  {
    ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
    ImGui::ColorEdit4("Color", glm::value_ptr(component.color), ImGuiColorEditFlags_NoInputs);
    ImGui::Separator();

    if(entity.HasComponent<SkinTemplateComponent>())
    { // TODO REFACTOR: it is right now it seems a bit hacky, need to refactor this
      SkinTemplateComponentWrapper st(entity);
      st.UpdateShapeAttributes();
    }
  });

  DrawComponent<SkinTemplateComponent>("Skin template params", entity, [&](auto& component)
  {
    SkinTemplateComponentWrapper st(entity);
    //ImGui::Unindent(ImGui::GetTreeNodeToLabelSpacing());
		auto verticalSliceCountBounds = st.GetVerticalSliceCountBounds();
    auto leftHorSliceCountBounds = st.GetLeftHorizontalSliceCountBounds();
    auto rightHorSliceCountBounds = st.GetRightHorizontalSliceCountBounds();
    ImGui::Text("Verticel slice count");
    ImGui::SliderInt("##VC", &(component.vertSliceCount), verticalSliceCountBounds.x, verticalSliceCountBounds.y, "%d");
    ImGui::Text("Left horizontal slice count");
    ImGui::SliderInt("##LHC", &(component.leftHorSliceCount), leftHorSliceCountBounds.x, leftHorSliceCountBounds.y, "%d");
    ImGui::Text("Right horizontal slice count");
    ImGui::SliderInt("##RHC", &(component.rightHorSliceCount), rightHorSliceCountBounds.x, rightHorSliceCountBounds.y, "%d");
    st.UpdateShapeAttributes();
  });
}

} // namespace medicimage
