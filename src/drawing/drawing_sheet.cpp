#include "drawing/drawing_sheet.h"
#include "drawing/components.h"
#include "drawing_sheet.h"
#include "core/log.h"
#include "image_editor.h"
#include <algorithm>
#include <chrono>

#include <glm/gtx/perpendicular.hpp>

namespace medicimage
{
  DrawingSheet::Entity::Entity(entt::entity handle, DrawingSheet* sheet)
  	: m_entityHandle(handle), m_sheet(sheet)
  {
  }

  void DrawingSheet::SetDocument(std::unique_ptr<ImageDocument> doc, glm::vec2 viewportSize)
  {
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
    m_drawing = std::make_unique<Texture2D>(*(m_originalDoc->texture.get())); // start to draw to a clean document

    auto circles = m_registry.view<CircleComponent>();
    for(auto e : circles)
    {
      Entity entity = {e, this};
      auto& transform  = entity.GetComponent<TransformComponent>();
      auto& commonAttributes = entity.GetComponent<CommonAttributesComponent>();
      auto& circle = entity.GetComponent<CircleComponent>();
      auto& color = entity.GetComponent<ColorComponent>().color;
      auto& center = transform.translation;
      ImageEditor::DrawCircle(m_drawing.get(), center, circle.radius, color, circle.thickness, commonAttributes.filled);

      if(commonAttributes.selected)
      {
        auto& pickPoints = entity.GetComponent<PickPointsComponent>().pickPoints;
        auto& translation = entity.GetComponent<TransformComponent>().translation;
        for(auto& point : pickPoints)
        {
          
          ImageEditor::DrawCircle(m_drawing.get(), point + translation, s_pickPointBoxSize / 2, s_pickPointColor, 2, true);
        }
      } 
    }

    auto rectangles = m_registry.view<RectangleComponent>();
    for(auto e : rectangles)
    {
      Entity entity = {e, this};
      auto& transform  = entity.GetComponent<TransformComponent>();
      auto& commonAttributes = entity.GetComponent<CommonAttributesComponent>();
      auto& rectangle = entity.GetComponent<RectangleComponent>();
      auto& color = entity.GetComponent<ColorComponent>().color;
      auto& topleft = transform.translation;
      auto bottomright = topleft + glm::vec2{rectangle.width, rectangle.height}; 
      ImageEditor::DrawRectangle(m_drawing.get(), topleft, bottomright, color, rectangle.thickness, commonAttributes.filled);
    
      if(commonAttributes.selected)
      {
        auto& pickPoints = entity.GetComponent<PickPointsComponent>().pickPoints;
        for(auto& point : pickPoints)
        {
          ImageEditor::DrawCircle(m_drawing.get(), point + transform.translation, s_pickPointBoxSize / 2, s_pickPointColor, 2, true);
        }
      }
    }
    
    auto arrows = m_registry.view<ArrowComponent>();
    for(auto e : arrows)
    {
      Entity entity = {e, this};
      auto& transform  = entity.GetComponent<TransformComponent>();
      auto& commonAttributes = entity.GetComponent<CommonAttributesComponent>();
      auto& arrow = entity.GetComponent<ArrowComponent>();
      auto& color = entity.GetComponent<ColorComponent>().color;
      auto begin = arrow.begin + transform.translation; 
      auto end = arrow.end + transform.translation; 
      ImageEditor::DrawArrow(m_drawing.get(), begin, end, color, arrow.thickness, 0.1);
    
      if(commonAttributes.selected)
      {
        auto& pickPoints = entity.GetComponent<PickPointsComponent>().pickPoints;
        for(auto& point : pickPoints)
        {
          ImageEditor::DrawCircle(m_drawing.get(), point + transform.translation, s_pickPointBoxSize / 2, s_pickPointColor, 2, true);
        }
      }
    }
    
    std::for_each(circles.begin(), circles.end(), m_drawState->DeleteTemporaries());
    std::for_each(rectangles.begin(), rectangles.end(), m_drawState->DeleteTemporaries());
    std::for_each(arrows.begin(), arrows.end(), m_drawState->DeleteTemporaries());

    std::stringstream ss;
    ss << std::put_time(std::localtime(&(m_originalDoc->timestamp)), "%d-%b-%Y %X");
    std::string footerText = m_originalDoc->documentId + " - " + ss.str();
    return ImageEditor::AddImageFooter(footerText, m_drawing.get());
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

  void DrawingSheet::SetHoveredEntity(Entity entity)
  {
    m_hoveredEntity = entity;
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

  Entity DrawingSheet::CreateRectangle(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
  {
    auto entity = CreateEntity(0, "rectangle");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = entity.GetComponent<TransformComponent>();

    auto& color = entity.AddComponent<ColorComponent>();  
    auto& rectangle = entity.AddComponent<RectangleComponent>();
    auto& boundingBox = entity.AddComponent<BoundingContourComponent>();
    auto& pickPoints = entity.AddComponent<PickPointsComponent>();

    auto tmpRect = cv::Rect2f(cv::Point2f{ m_firstPoint.x, m_firstPoint.y }, cv::Point2f{ m_secondPoint.x, m_secondPoint.y });
    rectangle.width = tmpRect.width;
    rectangle.height = tmpRect.height; 

    glm::vec2 topleft{tmpRect.tl().x, tmpRect.tl().y};
    transform.translation = topleft;

    UpdateRectangleShapeAttributes(entity);
    return entity;
  }
  
  void DrawingSheet::UpdateRectangleShapeAttributes(Entity entity)
  {
    auto& rectangle = entity.GetComponent<RectangleComponent>();
    auto& boundingBox = entity.GetComponent<BoundingContourComponent>();
    auto& pickPoints = entity.GetComponent<PickPointsComponent>();
    
    boundingBox.cornerPoints = {{0,0}, {rectangle.width, 0}, {rectangle.width, rectangle.height}, {0, rectangle.height}, {0,0}};
    pickPoints.pickPoints = {glm::vec2{rectangle.width, rectangle.height} + glm::vec2{0, -rectangle.height / 2},
      glm::vec2{rectangle.width, rectangle.height} + glm::vec2{-rectangle.width / 2, 0}, {0, rectangle.height / 2}, glm::vec2{rectangle.width / 2, 0}};
  }

  Entity DrawingSheet::CreateCircle(glm::vec2 topLeft, glm::vec2 bottomRight, DrawObjectType objectType)
  {
    auto entity = CreateEntity(0, "circle");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = entity.GetComponent<TransformComponent>();
    auto& boundingBox = entity.AddComponent<BoundingContourComponent>();
    auto& pickPoints = entity.AddComponent<PickPointsComponent>();
    transform.translation = topLeft;

    auto& color = entity.AddComponent<ColorComponent>();  
    auto& circle = entity.AddComponent<CircleComponent>();
    circle.radius = glm::length(topLeft - bottomRight);

    UpdateCircleShapeAttributes(entity);
    return entity;
  }

  void DrawingSheet::UpdateCircleShapeAttributes(Entity entity)
  {
    auto& circle = entity.GetComponent<CircleComponent>();
    auto& boundingBox = entity.GetComponent<BoundingContourComponent>();
    auto& pickPoints = entity.GetComponent<PickPointsComponent>();
    auto radius = circle.radius;
    // bounding polyigon of the circle is a square actually
    boundingBox.cornerPoints = {{-radius, -radius}, {radius, -radius}, {radius, radius}, {-radius, radius}, {0,0}};
    pickPoints.pickPoints = {{radius, 0}, {0, circle.radius}, {-circle.radius, 0}, {0, -circle.radius}};
  }

  Entity DrawingSheet::CreateArrow(glm::vec2 topLeft, glm::vec2 bottomRight, DrawObjectType objectType)
  {
    auto entity = CreateEntity(0, "arrow");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = entity.GetComponent<TransformComponent>();
    auto& boundingBox = entity.AddComponent<BoundingContourComponent>();
    auto& pickPoints = entity.AddComponent<PickPointsComponent>();
    transform.translation = topLeft;

    auto& color = entity.AddComponent<ColorComponent>();  
    auto& arrow = entity.AddComponent<ArrowComponent>();
    arrow.end = bottomRight - topLeft;

    UpdateArrowShapeAttributes(entity);
    return entity;
  }

  void DrawingSheet::UpdateArrowShapeAttributes(Entity entity)
  {
    auto& arrow = entity.GetComponent<ArrowComponent>();
    auto& boundingBox = entity.GetComponent<BoundingContourComponent>();
    auto& pickPoints = entity.GetComponent<PickPointsComponent>();

    glm::vec2 vec = arrow.begin - arrow.end; 
    glm::vec2 perp = glm::normalize(glm::vec2{-vec.y, vec.x});
    glm::vec2 offset = perp * glm::length(vec) * glm::vec2(0.2);
    boundingBox.cornerPoints = {offset, arrow.end + offset, arrow.end - offset, -offset, offset};
    pickPoints.pickPoints = {arrow.begin, arrow.end};
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
    auto hoveredEntity = m_sheet->GetHoveredEntity(pos);
    if(hoveredEntity)
      m_sheet->SetHoveredEntity(hoveredEntity.value()); 
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
  {
    m_sheet->m_secondPoint = pos / m_sheet->m_sheetSize;
    Entity entity;

    switch(m_sheet->m_currentDrawCommand)
    {
      case DrawCommand::DRAW_CIRCLE:
      {
        entity = m_sheet->CreateCircle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY);
        break;
      }
      case DrawCommand::DRAW_RECTANGLE:
      {
        entity = m_sheet->CreateRectangle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY);
        break;
      }
      case DrawCommand::DRAW_ARROW:
      {
        entity = m_sheet->CreateArrow(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY);
        break;
      }
    }
  }

  void DrawingTemporaryState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    m_sheet->m_secondPoint = pos / m_sheet->m_sheetSize;
    Entity entity;
    switch(m_sheet->m_currentDrawCommand)
    {
      case DrawCommand::DRAW_CIRCLE:
      {
        entity = m_sheet->CreateCircle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT);
        break;
      }
      case DrawCommand::DRAW_RECTANGLE:
      {
        entity = m_sheet->CreateRectangle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT);
        break;
      }
      case DrawCommand::DRAW_ARROW:
      {
        entity = m_sheet->CreateArrow(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT);
        break;
      }
    }
    
    m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
  }

  void ObjectSelectInitialState::OnMouseHovered(const glm::vec2 pos)
  {
    auto hoveredEntity = m_sheet->GetHoveredEntity(pos);
    if(hoveredEntity)
      m_sheet->SetHoveredEntity(hoveredEntity.value()); 
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
      auto entity = m_sheet->CreateRectangle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY); // TODO: add opacity
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
    auto attributes = m_sheet->m_registry.view<CommonAttributesComponent>();
    for(auto e : attributes)
    {
      Entity entity = {e, m_sheet};
      if(entity.GetComponent<CommonAttributesComponent>().selected)
        entity.GetComponent<CommonAttributesComponent>().selected = false;
    }
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
          switch(selectedPointIndex)  // [TODO REFACTOR]: extract these out into functions
          {
            case static_cast<int>(RectanglePicPoints::RIGHT):
            {
              entity.GetComponent<RectangleComponent>().width += diff.x;
              break;
            }
            case static_cast<int>(RectanglePicPoints::BOTTOM):
            {
              entity.GetComponent<RectangleComponent>().height += diff.y;
              break;
            }
            case static_cast<int>(RectanglePicPoints::LEFT):
            {
              entity.GetComponent<RectangleComponent>().width += -diff.x;
              entity.GetComponent<TransformComponent>().translation += glm::vec2{diff.x, 0};
              break;
            }
            case static_cast<int>(RectanglePicPoints::TOP):
            {
              entity.GetComponent<RectangleComponent>().height += -diff.y;
              entity.GetComponent<TransformComponent>().translation += glm::vec2{0, diff.y};
              break;
            }
            default:
              APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPointIndex, entity.GetComponent<IDComponent>().ID);
              break;
          } 
          m_sheet->UpdateRectangleShapeAttributes(entity);
        }
        else if(entity.HasComponent<CircleComponent>())
        {
          switch(selectedPointIndex)  // [TODO REFACTOR]: extract these out into functions
          {
            case static_cast<int>(CirclePickPoints::RIGHT):
              entity.GetComponent<CircleComponent>().radius += diff.x;
              break;
            case static_cast<int>(CirclePickPoints::LEFT):
              entity.GetComponent<CircleComponent>().radius += -diff.x;
              break;
            case static_cast<int>(CirclePickPoints::BOTTOM):
              entity.GetComponent<CircleComponent>().radius += diff.y;
              break;
            case static_cast<int>(CirclePickPoints::TOP):
              entity.GetComponent<CircleComponent>().radius += -diff.y;
              break;
            default:
              APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPointIndex, entity.GetComponent<IDComponent>().ID);
              break;
          } 
          m_sheet->UpdateCircleShapeAttributes(entity);
        }
        else if(entity.HasComponent<ArrowComponent>())
        {
          switch(selectedPointIndex)  // [TODO REFACTOR]: extract these out into functions
          {
            case static_cast<int>(ArrowPickPoints::BEGIN):
              entity.GetComponent<ArrowComponent>().begin += diff;
              break;
            case static_cast<int>(ArrowPickPoints::END):
              entity.GetComponent<ArrowComponent>().end += diff;
              break;
            default:
              APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPointIndex, entity.GetComponent<IDComponent>().ID);
              break;
          } 
          m_sheet->UpdateArrowShapeAttributes(entity);
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
    auto currentPoint = pos / m_sheet->m_sheetSize; 
    auto diff = (currentPoint - m_sheet->m_firstPoint) * glm::vec2(1.0);
    m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize;
    auto& transform = m_sheet->m_draggedEntity.GetComponent<TransformComponent>();
    transform.translation += diff;
  }

  void ObjectDraggingState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectedState>(m_sheet));
  }
} // namespace medicimage
