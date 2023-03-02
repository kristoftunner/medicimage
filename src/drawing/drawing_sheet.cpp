#include "drawing/drawing_sheet.h"
#include "drawing/copmonents.h"
#include "drawing_sheet.h"
#include "core/log.h"
#include "image_editor.h"
#include <algorithm>

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
      ImageEditor::DrawCircle(m_drawing.get(), entity);
    }

    auto rectangles = m_registry.view<RectangleComponent>();
    for(auto e : rectangles)
    {
      Entity entity = {e, this};
      ImageEditor::DrawRectangle(m_drawing.get(), entity);
    }
    
    auto arrows = m_registry.view<ArrowComponent>();
    for(auto e : arrows)
    {
      Entity entity = {e, this};
      ImageEditor::DrawArrow(m_drawing.get(), entity);
    }

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

  void DrawingSheet::AddToSelection(Entity entity)
  {
    m_selectedEntities.push_back(entity);
  }

  void DrawingSheet::ClearSelection()
  {
    m_selectedEntities.clear();
  }

  std::optional<Entity> DrawingSheet::GetHoveredEntity(const glm::vec2 pos)
  {
    // TODO: may want to move this into editor ui, so here only relative coordinates are handled
    const glm::vec2 relPos = pos / m_sheetSize;
    auto view = m_registry.view<BoundingContourComponent>();
    for(auto e : view)
    {
      Entity entity = {e, this};
      auto& boundingContour = entity.GetComponent<BoundingContourComponent>().cornerPoints;
      if(boundingContour.size() == 0)
        continue;

      std::vector<cv::Point2f> contour;
      std::transform(boundingContour.begin(), boundingContour.end(), std::back_inserter(contour), [](glm::vec2 vec) {return cv::Point2f{ vec.x, vec.y }; });
      if(cv::pointPolygonTest(contour, cv::Point2f{relPos.x, relPos.y}, false)  >= 0)
      {
        APP_CORE_INFO("Entity:{} is hovered", entity.GetComponent<IDComponent>().ID);
        return entity;
      }
    }
    return std::optional<Entity>();
  }

  void DrawingSheet::SetHoveredEntity(Entity entity)
  {
    m_hoveredEntity = entity;
  }

  std::vector<Entity> DrawingSheet::GetEntitiesUnderSelection()
  {
    return std::vector<Entity>(); // TODO: implement
  }

  Entity DrawingSheet::CreateRectangle(glm::vec2 topLeft, glm::vec2 bottomRight, DrawObjectType objectType)
  {
    auto entity = CreateEntity(0, "rectangle");
    auto& transform = entity.GetComponent<TransformComponent>();
    transform.translation = topLeft;

    auto& color = entity.AddComponent<ColorComponent>();  
    auto& rectangle = entity.AddComponent<RectangleComponent>();
    auto& boundingBox = entity.AddComponent<BoundingContourComponent>();

    rectangle.width = abs(bottomRight.x - topLeft.x);
    rectangle.height = abs(topLeft.y - bottomRight.y); 
    rectangle.temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    boundingBox.cornerPoints = {topLeft, topLeft + glm::vec2{rectangle.width, 0}, bottomRight, topLeft + glm::vec2{0, rectangle.height}};
    
    auto tl = topLeft * m_sheetSize;
    auto br = bottomRight * m_sheetSize;
    glm::vec2 vec = tl - br; 
    glm::vec2 perp = glm::normalize(glm::vec2{-vec.y, vec.x});
    auto& corners = boundingBox.cornerPoints;
    return entity;
  }

  Entity DrawingSheet::CreateCircle(glm::vec2 topLeft, glm::vec2 bottomRight, DrawObjectType objectType)
  {
    auto entity = CreateEntity(0, "circle");
    auto& transform = entity.GetComponent<TransformComponent>();
    transform.translation = topLeft;

    auto& color = entity.AddComponent<ColorComponent>();  
    auto& circle = entity.AddComponent<CircleComponent>();
    auto& boundingBox = entity.AddComponent<BoundingContourComponent>();
    
    circle.radius = glm::length(topLeft - bottomRight);
    circle.temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    auto radius = circle.radius;

    // bounding polyigon of the circle is a square actually
    boundingBox.cornerPoints = {topLeft + glm::vec2{-radius, -radius}, topLeft + glm::vec2{radius, -radius}, topLeft + glm::vec2{radius, radius}, topLeft + glm::vec2{-radius, radius}};
    auto& corners = boundingBox.cornerPoints;
    //APP_CORE_INFO("Circle added: center:{}:{} radius:{}", topLeft.x, topLeft.y, radius);
    //APP_CORE_INFO("With bounding box:tl:{}:{} tr:{}:{} br:{}:{} bl:{}:{}", corners[0].x, corners[0].y, corners[1].x, corners[1].y, corners[2].x, corners[2].y, corners[3].x, corners[3].y);
    
    return entity;
  }

  Entity DrawingSheet::CreateArrow(glm::vec2 topLeft, glm::vec2 bottomRight, DrawObjectType objectType)
  {
    auto entity = CreateEntity(0, "arrow");
    auto& transform = entity.GetComponent<TransformComponent>();
    transform.translation = topLeft;

    auto& color = entity.AddComponent<ColorComponent>();  
    auto& arrow = entity.AddComponent<ArrowComponent>();
    auto& boundingBox = entity.AddComponent<BoundingContourComponent>();
    
    arrow.end = bottomRight;
    arrow.temporary = objectType == DrawObjectType::TEMPORARY ? true : false;

    glm::vec2 vec = topLeft - bottomRight; 
    glm::vec2 perp = glm::normalize(glm::vec2{-vec.y, vec.x});
    glm::vec2 offset = perp * glm::length(vec) * glm::vec2(0.2);
    boundingBox.cornerPoints = {topLeft + offset, bottomRight + offset, bottomRight - offset, topLeft - offset};
    auto& corners = boundingBox.cornerPoints;
    //APP_CORE_INFO("Arrow added: begin:{}:{} end:{}:{}", topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
    //APP_CORE_INFO("With bounding box:tl:{}:{} tr:{}:{} br:{}:{} bl:{}:{}", corners[0].x, corners[0].y, corners[1].x, corners[1].y, corners[2].x, corners[2].y, corners[3].x, corners[3].y);
    return entity;
  }

  Entity DrawingSheet::CreateEntity(int id, const std::string &name)
  {
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<IDComponent>();
		entity.AddComponent<TransformComponent>();
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
        auto view = m_sheet->m_registry.view<CircleComponent>();
        std::for_each(view.begin(), view.end(), DeleteTemporaries<CircleComponent>());
        entity = m_sheet->CreateCircle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY);
        break;
      }
      case DrawCommand::DRAW_RECTANGLE:
      {
        auto view = m_sheet->m_registry.view<RectangleComponent>();
        std::for_each(view.begin(), view.end(), DeleteTemporaries<RectangleComponent>());
        entity = m_sheet->CreateRectangle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY);
        break;
      }
      case DrawCommand::DRAW_ARROW:
      {
        auto view = m_sheet->m_registry.view<ArrowComponent>();
        std::for_each(view.begin(), view.end(), DeleteTemporaries<ArrowComponent>());
        entity = m_sheet->CreateArrow(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY);
        break;
      }
    }
  }

  void DrawingTemporaryState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    // delete the temporary object and add the permanent
    m_sheet->m_secondPoint = pos / m_sheet->m_sheetSize;
    Entity entity;
    switch(m_sheet->m_currentDrawCommand)
    {
      case DrawCommand::DRAW_CIRCLE:
      {
        auto view = m_sheet->m_registry.view<CircleComponent>();
        std::for_each(view.begin(), view.end(), DeleteTemporaries<CircleComponent>());
        entity = m_sheet->CreateCircle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT);
        break;
      }
      case DrawCommand::DRAW_RECTANGLE:
      {
        auto view = m_sheet->m_registry.view<RectangleComponent>();
        std::for_each(view.begin(), view.end(), DeleteTemporaries<RectangleComponent>());
        entity = m_sheet->CreateRectangle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT);
        break;
      }
      case DrawCommand::DRAW_ARROW:
      {
        auto view = m_sheet->m_registry.view<ArrowComponent>();
        std::for_each(view.begin(), view.end(), DeleteTemporaries<ArrowComponent>());
        entity = m_sheet->CreateArrow(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT);
        break;
      }
    }
    
    m_sheet->m_selectedEntities.clear();  // just to have a cleared selection before moving into selection
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
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectionState>(m_sheet)); 
  }

  void ObjectSelectionState::OnMouseButtonDown(const glm::vec2 pos)
  {
    m_sheet->m_secondPoint = pos / m_sheet->m_sheetSize;
    auto entities = m_sheet->GetEntitiesUnderSelection();
    for(auto& entity : entities)
    {
      auto it = std::find_if(m_sheet->m_selectedEntities.begin(), m_sheet->m_selectedEntities.end(), 
      [&](Entity e)
      {
        return e.GetComponent<IDComponent>().ID == entity.GetComponent<IDComponent>().ID;
      });
      if(it != m_sheet->m_selectedEntities.end())
        m_sheet->m_selectedEntities.push_back(entity);
    }
  }

  void ObjectSelectionState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectionState>(m_sheet)); 

  }

  void ObjectsSelectedState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    // TODO: implement
  }

} // namespace medicimage
