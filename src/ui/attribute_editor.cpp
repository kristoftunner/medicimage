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
    static ImVec4 backup_color;
    auto& color = component.color;
    if (ImGui::Button("palette"))
    {
      ImGui::OpenPopup("mypicker");
    }
    if (ImGui::BeginPopup("mypicker"))
    {
      ImGui::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
      ImGui::Separator();
      ImGui::ColorPicker3("##picker", glm::value_ptr(component.color), ImGuiColorEditFlags_NoSidePreview);
      ImGui::SameLine();

      ImGui::BeginGroup(); // Lock X position
      ImGui::Text("Current");
      ImGui::ColorButton("##current", ImVec4{ color.r, color.g, color.b, color.a }, ImGuiColorEditFlags_NoPicker, ImVec2(60, 40));
      ImGui::Separator();
      ImGui::Text("Palette");
      static ImVec4 saved_palette[] =
      {
        { 0.0, 0.0, 0.0, 1.0 },
        { 1.0, 0.0, 0.0, 1.0 },
        {0.0, 1.0, 0.0, 1.0},
        {0.0, 0.0, 1.0, 1.0},
        {1.0, 1.0, 1.0, 1.0},
        {1.0, 0.0, 1.0, 1.0}
      };
      for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
      {
        ImGui::PushID(n);
        if ((n % 8) != 0)
          ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);

        ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
        if (ImGui::ColorButton("##palette", saved_palette[n], palette_button_flags, ImVec2(20, 20)))
          component.color = glm::vec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, component.color.w); // Preserve alpha!

        ImGui::PopID();
      }
      ImGui::EndGroup();
      ImGui::EndPopup();
    }
    ImGui::Separator();

    if (entity.HasComponent<SkinTemplateComponent>())
    { // TODO REFACTOR: it is right now it seems a bit hacky, need to refactor this
      SkinTemplateComponentWrapper st(entity);
      st.UpdateShapeAttributes();
    }
  });

  DrawComponent<ThicknessComponent>("Thickness", entity, [&](auto& component)
  {
    ImGui::Text("Thickness");
    ImGui::SliderInt("##T", &(component.thickness), 2, 10, "%d");
    if (entity.HasComponent<SkinTemplateComponent>())
    { // TODO REFACTOR: it is right now it seems a bit hacky, need to refactor this
      SkinTemplateComponentWrapper st(entity);
      st.UpdateShapeAttributes();
    }
  });
  
  DrawComponent<TextComponent>("Font size", entity, [&](auto& component)
  {
    ImGui::Text("FontSize");
    ImGui::SliderInt("##F", &(component.fontSize), 2, 10, "%d");
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
    ImGui::Text("Ellipse");
    ImGui::Checkbox("Ellipse", &(component.drawSpline));
    st.UpdateShapeAttributes();
  });
}

} // namespace medicimage
