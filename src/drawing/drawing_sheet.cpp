#include "drawing/drawing_sheet.h"
#include "drawing/components.h"
#include "drawing/drawing_sheet.h"
#include "drawing/component_wrappers.h"
#include "core/log.h"
#include "image_handling/image_editor.h"
#include <algorithm>
#include <chrono>

#include <glm/gtx/perpendicular.hpp>
#include "drawing_sheet.h"

namespace medicimage
{
  DrawingSheet::Entity::Entity(entt::entity handle, DrawingSheet* sheet)
  	: m_entityHandle(handle), m_sheet(sheet)
  {
  }

  void DrawingSheet::SetDocument(std::unique_ptr<ImageDocument> doc, glm::vec2 viewportSize)
  {
    // clear the registry for clear drawing
    auto objects = m_registry.view<TransformComponent>();
    for(auto e : objects)
    {
      Entity entity = {e, this};
      DestroyEntity(entity);
    }

    m_sheetSize = viewportSize;
    m_originalDoc = std::move(doc);
    m_drawing = std::make_unique<Texture2D>(m_originalDoc->texture->GetTexturePtr(), "texture");
  }
  
  void DrawingSheet::SetDrawCommand(const DrawCommand command)
  {
    m_currentDrawCommand = command;

    // set the initial command state according to the draw command
    switch(command)
    {
      case DrawCommand::DRAW_CIRCLE:
      case DrawCommand::DRAW_RECTANGLE:
      case DrawCommand::DRAW_ELLIPSE:
      case DrawCommand::DRAW_ARROW:
      case DrawCommand::DRAW_SKIN_TEMPLATE:
      {
        m_drawState = std::make_unique<InitialObjectDrawState>(this);
        break;
      }
      case DrawCommand::DO_NOTHING:
      {
        m_drawState = std::make_unique<BaseDrawState>(this);
        break;
      }
    }
  }

  std::unique_ptr<Texture2D> DrawingSheet::Draw()
  {
    std::stringstream ss;
    ss << std::put_time(std::localtime(&(m_originalDoc->timestamp)), "%d-%b-%Y %X");
    std::string footerText = m_originalDoc->documentId + " - " + ss.str();
    m_drawing = ImageEditor::ReplaceImageFooter(footerText, m_originalDoc->texture.get());

    ImageEditor::Begin(m_drawing.get());
    auto circles = m_registry.view<CircleComponent>();
    for(auto e : circles)
    {
      Entity entity = {e, this};
      CircleComponentWrapper cw(entity);
      cw.Draw();
    }

    auto rectangles = m_registry.view<RectangleComponent>();
    for(auto e : rectangles)
    {
      Entity entity = {e, this};
      RectangleComponentWrapper rw(entity);
      rw.Draw(); 
    }
    
    auto arrows = m_registry.view<ArrowComponent>();
    for(auto e : arrows)
    {
      Entity entity = {e, this};
      ArrowComponentWrapper aw(entity);
      aw.Draw();
    }

    ImageEditor::End(m_drawing.get());

    std::for_each(circles.begin(), circles.end(), m_drawState->DeleteTemporaries());
    std::for_each(rectangles.begin(), rectangles.end(), m_drawState->DeleteTemporaries());
    std::for_each(arrows.begin(), arrows.end(), m_drawState->DeleteTemporaries());

    return std::move(std::make_unique<Texture2D>(*m_drawing.get()));
  }

  void DrawingSheet::ChangeDrawState(std::unique_ptr<BaseDrawState> newState)
  {
    m_drawState = std::move(newState);
  }

  void DrawingSheet::SetDrawingSheetSize(glm::vec2 size)
  {
    m_sheetSize = size;
  }

  void DrawingSheet::OnMouseHovered(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnMouseHovered(pos);
  }

  void DrawingSheet::OnMouseButtonPressed(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnMouseButtonPressed(pos);
  }

  void DrawingSheet::OnMouseButtonDown(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnMouseButtonDown(pos);
  }

  void DrawingSheet::OnMouseButtonReleased(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnMouseButtonReleased(pos);
  }

  void DrawingSheet::OnTextInput(const std::string &inputText)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnTextInput(inputText);
  }


  std::optional<Entity> DrawingSheet::GetHoveredEntity(const glm::vec2 pos)
  {
    // TODO: may want to move this into editor ui, so here only relative coordinates are handled
    const glm::vec2 relPos = pos / m_sheetSize;
    auto view = m_registry.view<BoundingContourComponent>();
    for(auto e : view)
    {
      Entity entity = {e, this};
      auto boundingContour = entity.GetComponent<BoundingContourComponent>().cornerPoints;
      if(boundingContour.size() == 0)
        continue;

      auto translation = entity.GetComponent<TransformComponent>().translation;
      for(auto& c : boundingContour)
        c += translation;
      std::vector<cv::Point2f> contour;
      std::transform(boundingContour.begin(), boundingContour.end(), std::back_inserter(contour), [](glm::vec2 vec) {return cv::Point2f{ vec.x, vec.y }; });
      if(cv::pointPolygonTest(contour, cv::Point2f{relPos.x, relPos.y}, false)  >= 0)
      {
        APP_CORE_TRACE("Entity:{} is hovered", entity.GetComponent<IDComponent>().ID);
        return entity;
      }
    }
    return std::optional<Entity>();
  }

  std::vector<Entity> DrawingSheet::GetSelectedEntities()
  {
    std::vector<Entity> selectedEntities;
    auto view = m_registry.view<CommonAttributesComponent>();
    for(auto e : view)
    {
      Entity entity = {e, this};
      if(entity.GetComponent<CommonAttributesComponent>().selected)
        selectedEntities.push_back(entity);
    }
    return selectedEntities;
  }
  bool DrawingSheet::IsUnderSelectArea(Entity entity, glm::vec2 pos)
  {
    auto boundingContour = entity.GetComponent<BoundingContourComponent>().cornerPoints;
    auto translation = entity.GetComponent<TransformComponent>().translation;
    for(auto& c : boundingContour)
      c += translation;
    std::vector<cv::Point2f> entityContour;
    std::transform(boundingContour.begin(), boundingContour.end(), std::back_inserter(entityContour), [](glm::vec2 vec) {return cv::Point2f{ vec.x, vec.y }; });
      
    auto dx = m_secondPoint.x - m_firstPoint.x;
    auto dy = m_secondPoint.y - m_firstPoint.y;
    std::vector<cv::Point2f> selectContour{cv::Point2f{m_firstPoint.x, m_firstPoint.y}, cv::Point2f{m_firstPoint.x + dx, m_firstPoint.y},
      cv::Point2f{m_secondPoint.x, m_secondPoint.y}, cv::Point2f{m_secondPoint.x - dx, m_secondPoint.y}};
      
    std::vector<cv::Point2f> tmp;
    if (cv::intersectConvexConvex(selectContour, entityContour, tmp, true) > 0.0)
    {
      if(cv::intersectConvexConvex(selectContour, entityContour, tmp, false) == 0.0)
      {
        APP_CORE_INFO("Entity:{} selected with bb: tl:{}:{}, tr:{}:{}, br:{}:{}, bl:{}:{}", entity.GetComponent<IDComponent>().ID,
          boundingContour[0].x, boundingContour[0].y, boundingContour[1].x, boundingContour[1].y, boundingContour[2].x, boundingContour[2].y, boundingContour[3].x, boundingContour[3].y);
        return true;
      }
    }
    return false;
  }

  bool DrawingSheet::IsPickpointSelected(Entity entity, glm::vec2 pos)
  {
    auto pickPoints = entity.GetComponent<PickPointsComponent>().pickPoints;
    assert(pickPoints.size() != 0);
    auto translation = entity.GetComponent<TransformComponent>().translation;
    for(auto& p : pickPoints)
      p += translation;
    for(int i = 0; i < pickPoints.size(); i++)
    {
      auto point = pickPoints[i];
      std::vector<cv::Point2f> contour {{point.x - s_pickPointBoxSize / 2, point.y - s_pickPointBoxSize / 2}, {point.x + s_pickPointBoxSize / 2, point.y - s_pickPointBoxSize / 2},
        {point.x + s_pickPointBoxSize / 2, point.y + s_pickPointBoxSize / 2}, {point.x - s_pickPointBoxSize / 2, point.y + s_pickPointBoxSize / 2}};
      if(cv::pointPolygonTest(contour, cv::Point2f{pos.x, pos.y}, false)  >= 0)
      {
        entity.GetComponent<PickPointsComponent>().selectedPoint = i;
        return true;
      }
    }
    return false;
  }

  bool DrawingSheet::IsDragAreaSelected(Entity entity, glm::vec2 pos)
  {
    auto boundingContour = entity.GetComponent<BoundingContourComponent>().cornerPoints;
    assert(boundingContour.size() != 0);
    auto translation = entity.GetComponent<TransformComponent>().translation;
    for(auto& c : boundingContour)
      c += translation;
    std::vector<cv::Point2f> contour;
    std::transform(boundingContour.begin(), boundingContour.end(), std::back_inserter(contour), [](glm::vec2 vec) {return cv::Point2f{ vec.x, vec.y }; });
    if(cv::pointPolygonTest(contour, cv::Point2f{pos.x, pos.y}, false)  >= 0)
      return true;
    else
      return false;
  }
  
  Entity DrawingSheet::CreateSkinTemplate(glm::vec2 topLeft, glm::vec2 bottomRight, DrawObjectType objectType)
  {
    auto entity = CreateEntity(0, "SkinTemplate");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = entity.GetComponent<TransformComponent>();
    auto& skinTemplate = entity.GetComponent<SkinTemplateComponent>();

    static constexpr float minBoundingWidth = 0.1;
    static constexpr float minBoundingHeight = 0.1;
    static constexpr int verticalSliceCount = 3;
    static constexpr int horizontalSliceCount = 1;
    auto diff = bottomRight - topLeft;
    if(abs(diff.x) >= minBoundingWidth && abs(diff.y) >= minBoundingHeight)
    {
      auto boundingRectSize = glm::abs(diff);
      skinTemplate.verticalSliceSize = glm::vec2{ diff.x / 10.0, diff.y };
      skinTemplate.horizontalSliceSize = glm::vec2{ diff.x * (7.0 / 20.0), diff.y / 5.0 };
      skinTemplate.boundingRectSize = glm::abs(diff);
      auto center = topLeft + diff / glm::vec2(2.0);
      for (auto i = 0; i < verticalSliceCount; i++)
      {
        glm::vec2 vertTopLeft = glm::vec2{ - skinTemplate.verticalSliceSize.x * 1.5, -skinTemplate.verticalSliceSize.y / 2.0 } + center + glm::vec2{ i * skinTemplate.verticalSliceSize.x, 0.0 };
        glm::vec2 vertBottomRight = glm::vec2{ -skinTemplate.verticalSliceSize.x * 0.5, skinTemplate.verticalSliceSize.y / 2.0 } + center + glm::vec2{ i * skinTemplate.verticalSliceSize.x, 0.0 };
        auto rect = CreateEntity(0, "rectangle");
        RectangleComponentWrapper::CreateRectangle(rect, vertTopLeft, vertBottomRight, objectType);
      }
      for (auto i = 0; i < horizontalSliceCount; i++)
      {
        glm::vec2 horTopLeft = glm::vec2{ -skinTemplate.horizontalSliceSize.x * 1.5, -skinTemplate.horizontalSliceSize.y / 2.0 } + center + glm::vec2{ i * skinTemplate.horizontalSliceSize.x, 0.0 };
        glm::vec2 horBottomRight = glm::vec2{ -skinTemplate.horizontalSliceSize.x * 0.5, skinTemplate.horizontalSliceSize.y / 2.0 } + center + glm::vec2{ i * skinTemplate.horizontalSliceSize.x, 0.0 };
        auto rect = CreateEntity(0, "rectangle");
        RectangleComponentWrapper::CreateRectangle(rect, horTopLeft, horBottomRight, objectType);
      }
    }

    UpdateSkinTemplateShapeAttributes(entity);
    return entity;
  }

  void DrawingSheet::UpdateSkinTemplateShapeAttributes(Entity entity)
  {

  }

  void DrawingSheet::ClearSelectionShapes()
  {
    // clear the selection and selected pickpoint
    auto attributes = m_registry.view<CommonAttributesComponent>();
    for(auto e : attributes)
    {
      Entity entity = {e, this};
      if(entity.GetComponent<CommonAttributesComponent>().selected)
        entity.GetComponent<CommonAttributesComponent>().selected = false;
    }

    auto pickpoints = m_registry.view<PickPointsComponent>();
    for(auto e : pickpoints)
    {
      Entity entity = {e, this};
      entity.GetComponent<PickPointsComponent>().selectedPoint = -1;
    }
    
    m_draggedEntity.reset();
  }

  Entity DrawingSheet::CreateEntity(int id, const std::string &name)
  {
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<IDComponent>();
		entity.AddComponent<TransformComponent>();
    entity.AddComponent<CommonAttributesComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.tag = name.empty() ? "Entity" : name;

    return entity;
  }

  void DrawingSheet::DestroyEntity(Entity entity)
  {
    m_registry.destroy(entity);
  }

  void InitialObjectDrawState::OnMouseHovered(const glm::vec2 pos)
  {
    m_sheet->m_hoveredEntity = m_sheet->GetHoveredEntity(pos);
  }

  void InitialObjectDrawState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->ChangeDrawState(std::make_unique<FirstClickRecievedState>(m_sheet)); 
  }

  void FirstClickRecievedState::OnMouseButtonDown(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize; 
    m_sheet->ChangeDrawState(std::make_unique<DrawingTemporaryState>(m_sheet)); 
  }

  void FirstClickRecievedState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    // fallback to initial state, because it was just an accidental single click
    m_sheet->ChangeDrawState(std::make_unique<InitialObjectDrawState>(m_sheet));
  }


  void DrawingTemporaryState::OnMouseButtonDown(const glm::vec2 pos)
  { // TODO REFACTOR: implement here the visitor pattern and extract the creaton functionality into a class 
    m_sheet->m_secondPoint = pos / m_sheet->m_sheetSize;
    Entity entity;

    switch(m_sheet->m_currentDrawCommand)
    {
      case DrawCommand::DRAW_CIRCLE:
      {
        entity = m_sheet->CreateEntity(0, "circle");
        CircleComponentWrapper cw(CircleComponentWrapper::CreateCircle(entity, m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY));
        cw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_RECTANGLE:
      {
        entity = m_sheet->CreateEntity(0, "rectangle");
        RectangleComponentWrapper rw(RectangleComponentWrapper::CreateRectangle(entity, m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY));
        rw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_ARROW:
      {
        entity = m_sheet->CreateEntity(0, "arrow");
        ArrowComponentWrapper aw(ArrowComponentWrapper::CreateArrow(entity, m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY));
        aw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_SKIN_TEMPLATE:
      {
        entity = m_sheet->CreateSkinTemplate(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY);
        m_sheet->UpdateSkinTemplateShapeAttributes(entity);
        break;
      }
    }
  }

  void DrawingTemporaryState::OnMouseButtonReleased(const glm::vec2 pos)
  { // TODO REFACTOR: see above 
    m_sheet->m_secondPoint = pos / m_sheet->m_sheetSize;
    Entity entity;
    switch(m_sheet->m_currentDrawCommand)
    {
      case DrawCommand::DRAW_CIRCLE:
      {
        entity = m_sheet->CreateEntity(0, "circle");
        CircleComponentWrapper cw(CircleComponentWrapper::CreateCircle(entity, m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT));
        cw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_RECTANGLE:
      {
        entity = m_sheet->CreateEntity(0, "rectangle");
        RectangleComponentWrapper rw(RectangleComponentWrapper::CreateRectangle(entity, m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT));
        rw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_ARROW:
      {
        entity = m_sheet->CreateEntity(0, "arrow");
        ArrowComponentWrapper aw(ArrowComponentWrapper::CreateArrow(entity, m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT));
        aw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_SKIN_TEMPLATE:
      {
        entity = m_sheet->CreateSkinTemplate(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT);
        m_sheet->UpdateSkinTemplateShapeAttributes(entity);
        break;
      }
    }
    
    m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
  }

  void ObjectSelectInitialState::OnMouseHovered(const glm::vec2 pos)
  {
    m_sheet->m_hoveredEntity = m_sheet->GetHoveredEntity(pos);
  }

  void ObjectSelectInitialState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize; 
    auto entity = m_sheet->GetHoveredEntity(pos);
    if(entity.has_value())
      entity.value().GetComponent<CommonAttributesComponent>().selected = true; 
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectionState>(m_sheet));
  }

  void ObjectSelectionState::OnMouseButtonDown(const glm::vec2 pos)
  {
    m_sheet->m_secondPoint = pos / m_sheet->m_sheetSize;
    auto& firstPoint = m_sheet->m_firstPoint;
    auto& secondPoint = m_sheet->m_secondPoint;
    if ((firstPoint.x != secondPoint.x) && (firstPoint.y != secondPoint.y)) // TODO: may not need this
    { // baby blue select rectangle
      auto entity = m_sheet->CreateEntity(0, "rectangle");
      RectangleComponentWrapper rw(RectangleComponentWrapper::CreateRectangle(entity, m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY));
      entity.GetComponent<ColorComponent>().color = m_sheet->s_selectBoxColor;
      entity.GetComponent<RectangleComponent>().thickness = 2;
      entity.GetComponent<CommonAttributesComponent>().filled = true;
    }
  }

  void ObjectSelectionState::OnMouseButtonReleased(const glm::vec2 pos)
  { // If we have selected entities, then go back to initial select state, else go forward
    bool hasSelectedObject = false;
    auto view = m_sheet->m_registry.view<BoundingContourComponent>();
    for(auto e : view)
    {
      Entity entity = {e, m_sheet};
      if (m_sheet->IsUnderSelectArea(entity, pos))
      {
        entity.GetComponent<CommonAttributesComponent>().selected = true;
        hasSelectedObject = true;
      }
    }

    if(hasSelectedObject)
      m_sheet->ChangeDrawState(std::make_unique<ObjectSelectedState>(m_sheet)); 
    else
      m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet)); 
  }

  void ObjectSelectedState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize;
    auto view = m_sheet->m_registry.view<PickPointsComponent>();
    for(auto e : view)
    {
      Entity entity = {e, m_sheet};
      if(m_sheet->IsPickpointSelected(entity, m_sheet->m_firstPoint))
      {
        m_sheet->ChangeDrawState(std::make_unique<PickPointSelectedState>(m_sheet));
        return; 
      }
      else if(m_sheet->IsDragAreaSelected(entity, m_sheet->m_firstPoint))
      {
        m_sheet->m_draggedEntity = entity;
        m_sheet->ChangeDrawState(std::make_unique<ObjectDraggingState>(m_sheet)); 
        return;
      }
    }
   
    // clicking outside of the pickpoints and drage area, clear selection and go back to initial state
    m_sheet->ClearSelectionShapes();
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
  }

  void PickPointSelectedState::OnMouseButtonDown(const glm::vec2 pos)
  {
    auto currentPoint = pos / m_sheet->m_sheetSize; 
    auto diff = (currentPoint - m_sheet->m_firstPoint) * glm::vec2(1.0);
    m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize;
    auto view = m_sheet->m_registry.view<PickPointsComponent>();
    for(auto e : view)
    {
      Entity entity = {e, m_sheet};
      auto selectedPointIndex = entity.GetComponent<PickPointsComponent>().selectedPoint;
      if(selectedPointIndex != -1)
      {
        if (entity.HasComponent<RectangleComponent>())
        {
          RectangleComponentWrapper rw(entity);
          rw.OnPickPointDrag(diff, selectedPointIndex);
        }
        else if(entity.HasComponent<CircleComponent>())
        {
          CircleComponentWrapper cw(entity);
          cw.OnPickPointDrag(diff, selectedPointIndex);
        }
        else if(entity.HasComponent<ArrowComponent>())
        {
          ArrowComponentWrapper aw(entity);
          aw.OnPickPointDrag(diff, selectedPointIndex);
        }
        else
          APP_CORE_ERR("WTF this component");
      }
    }
  }

  void PickPointSelectedState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    // clear pickpoint selection
    auto view = m_sheet->m_registry.view<PickPointsComponent>();
    for(auto e : view)
    {
      Entity entity = {e, m_sheet};
      entity.GetComponent<PickPointsComponent>().selectedPoint = -1;
    }
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectedState>(m_sheet));
  }

  void ObjectDraggingState::OnMouseButtonDown(const glm::vec2 pos)
  {
    if(m_sheet->m_draggedEntity.has_value())
    {
      auto currentPoint = pos / m_sheet->m_sheetSize; 
      auto diff = (currentPoint - m_sheet->m_firstPoint) * glm::vec2(1.0);
      m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize;
      auto& transform = m_sheet->m_draggedEntity.value().GetComponent<TransformComponent>();
      transform.translation += diff;
    }
    else
      APP_CORE_ERR("Something went wrong with dragging");
  }

  void ObjectDraggingState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectedState>(m_sheet));
  }
} // namespace medicimage
