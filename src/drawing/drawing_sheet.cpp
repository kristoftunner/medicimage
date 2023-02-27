#include "drawing/drawing_sheet.h"
#include "drawing/copmonents.h"
#include "drawing_sheet.h"
#include "core/log.h"

namespace medicimage
{

  void DrawingSheet::SetDocument(std::unique_ptr<ImageDocument> doc, glm::vec2 viewportSize)
  {
    m_viewportSize = viewportSize;
    m_document = std::move(doc);
  }
  
  void DrawingSheet::SetDrawCommand(const DrawCommand command)
  {
    m_currentDrawCommand = command;
    switch(command)
    {
      case DrawCommand::DRAW_CIRCLE:
      {
        break;
      }
      case DrawCommand::DRAW_RECTANGLE:
      {
        break;
      }
    }
  }

  void DrawingSheet::Draw()
  {

  }

  void DrawingSheet::OnMouseHovered(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::INITIAL)
      m_drawState->OnMouseHovered(pos);
  }

  void DrawingSheet::OnMouseButtonPressed(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::INITIAL)
      m_drawState->OnMouseButtonPressed(pos);
  }

  void DrawingSheet::OnMouseButtonDown(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::INITIAL)
      m_drawState->OnMouseButtonDown(pos);
  }

  void DrawingSheet::OnTextInput(const std::string &inputText)
  {
    if(m_currentDrawCommand != DrawCommand::INITIAL)
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
    // TODO: calculate if there is an entity hovered or not
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

  Entity DrawingSheet::CreateEntity(int id, const std::string &name)
  {
		Entity entity = { m_registry.create(), this };
		entity.AddComponent<IDComponent>(id);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.tag = name.empty() ? "Entity" : name;

    return entity;
  }

  void InitialObjectDrawState::OnMouseHovered(const glm::vec2 pos)
  {
    auto hoveredEntity = m_sheet->GetHoveredEntity(pos);
    if(hoveredEntity)
      m_sheet->SetHoveredEntity(hoveredEntity.value()); 
  }

  void medicimage::InitialObjectDrawState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    std::unique_ptr<BaseDrawState> state = std::make_unique<FirstClickRecievedState>(m_sheet);
    m_sheet->ChangeDrawState(std::move(state)); 
  }

  void medicimage::FirstClickRecievedState::OnMouseButtonDown(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = pos / m_sheet->m_viewportSize; 
    std::unique_ptr<BaseDrawState> state = std::make_unique<DrawingTemporaryState>(m_sheet);
    m_sheet->ChangeDrawState(std::move(state)); 
  }

  void FirstClickRecievedState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    // fallback to initial state, because it was just an accidental single click
    std::unique_ptr<BaseDrawState> state = std::make_unique<InitialObjectDrawState>(m_sheet);
    m_sheet->ChangeDrawState(std::move(state));
  }

  void DrawingTemporaryState::OnMouseButtonDown(const glm::vec2 pos)
  {
    m_sheet->m_secondPoint = pos / m_sheet->m_viewportSize;
    auto entity = m_sheet->CreateRectangle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY);
    auto rectangle = entity.GetComponent<RectangleComponent>();
    auto transform = entity.GetComponent<TransformComponent>();
    APP_CORE_TRACE("Rectangle added: h:{} w:{} center:{}-{}", rectangle.height, rectangle.width, transform.translation.x, transform.translation.y);
  }

  void DrawingTemporaryState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    m_sheet->m_secondPoint = pos / m_sheet->m_viewportSize;
    auto entity = m_sheet->CreateRectangle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT);
    auto rectangle = entity.GetComponent<RectangleComponent>();
    auto transform = entity.GetComponent<TransformComponent>();
    APP_CORE_TRACE("Rectangle added: h:{} w:{} center:{}-{}", rectangle.height, rectangle.width, transform.translation.x, transform.translation.y);

    std::unique_ptr<BaseDrawState> state = std::make_unique<ObjectSelectInitialState>(m_sheet);
    m_sheet->m_selectedEntities.clear();  // just to have a cleared selection before moving into selection
    m_sheet->ChangeDrawState(std::move(state));
    m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
  }

  void ObjectSelectInitialState::OnMouseHovered(const glm::vec2 pos)
  {
    auto hoveredEntity = m_sheet->GetHoveredEntity(pos);
    if(hoveredEntity)
      m_sheet->SetHoveredEntity(hoveredEntity.value()); 
  }

  void ObjectSelectInitialState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = pos / m_sheet->m_viewportSize; 
    std::unique_ptr<BaseDrawState> state = std::make_unique<ObjectSelectionState>(m_sheet);
    m_sheet->ChangeDrawState(std::move(state)); 
  }

  void ObjectSelectionState::OnMouseButtonDown(const glm::vec2 pos)
  {
    m_sheet->m_secondPoint = pos / m_sheet->m_viewportSize;
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
    std::unique_ptr<BaseDrawState> state = std::make_unique<ObjectSelectionState>(m_sheet);
    m_sheet->ChangeDrawState(std::move(state)); 

  }

  void ObjectsSelectedState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    // TODO: implement
  }

} // namespace medicimage
