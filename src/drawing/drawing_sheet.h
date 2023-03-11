#pragma once

#include "image_handling/image_saver.h"
#include "drawing/components.h"
#include "core/assert.h"

#include <glm/glm.hpp>
#include <string>
#include <optional>
#include <entt/entt.hpp>

namespace medicimage
{

// forward declaration of the states
class BaseDrawState;
class InitialObjectDrawState;
class FirstClickRecievedState; 
class DrawingTemporaryState;
class ObjectSelectInitialState;
class ObjectSelectionState; 
class ObjectSelectedState;
class PickPointSelectedState;
class ObjectDraggingState; 

enum class DrawCommand{DO_NOTHING, OBJECT_SELECT, DRAW_LINE, DRAW_CIRCLE, DRAW_RECTANGLE, DRAW_ARROW, DRAW_ELLIPSE, DRAW_TEXT}; // Possible commands: DRAG(MOVE), 
enum class DrawObjectType{TEMPORARY, PERMANENT};

// TODO: 1) color handling
//       2) shape thickness handling
//       3) handle rotation 
//       4) implement correct UUID handling
//       5) handle various aspect ratios
class DrawingSheet
{
public:
  class Entity
  {
  public:
  	Entity() = default;
  	Entity(entt::entity handle, DrawingSheet* sheet);
  	Entity(const Entity& other) = default;

  	template<typename T, typename... Args>
  	T& AddComponent(Args&&... args)
  	{
  		MI_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");  
  		T& component = m_sheet->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
  		//m_sheet->OnComponentAdded<T>(*this, component);
  		return component;
  	}

  	template<typename T, typename... Args>
  	T& AddOrReplaceComponent(Args&&... args)
  	{
  		T& component = m_sheet->m_registry.emplace_or_replace<T>(m_entityHandle, std::forward<Args>(args)...);
  		//m_sheet->OnComponentAdded<T>(*this, component);
  		return component;
  	}

  	template<typename T>
  	T& GetComponent()
  	{
  		MI_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
  		return m_sheet->m_registry.get<T>(m_entityHandle);
  	}

  	template<typename T>
  	bool HasComponent()
  	{
      return m_sheet->m_registry.any_of<T>(m_entityHandle);
  	}

  	template<typename T>
  	void RemoveComponent()
  	{
  		MI_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
  		m_sheet->m_registry.remove<T>(m_entityHandle);
  	}

  	operator bool() const { return m_entityHandle != entt::null; }
  	operator entt::entity() const { return m_entityHandle; }
  	operator uint32_t() const { return (uint32_t)m_entityHandle; }

  	int GetUUID() { return GetComponent<IDComponent>().ID; }
  	const std::string& GetName() { return GetComponent<TagComponent>().tag; }

  	bool operator==(const Entity& other) const
  	{
  		return m_entityHandle == other.m_entityHandle && m_sheet == other.m_sheet;
  	}

  	bool operator!=(const Entity& other) const
  	{
  		return !(*this == other);
  	}
  private:
  	entt::entity m_entityHandle{ entt::null };
  	DrawingSheet* m_sheet = nullptr;
  };

public:
  DrawingSheet() : m_drawState(std::make_unique<BaseDrawState>(this)) {}
  void SetDocument(std::unique_ptr<ImageDocument> doc, glm::vec2 viewportSize); 
  void SetDrawCommand(const DrawCommand command); // initialize the state with the command's init state
  std::unique_ptr<Texture2D> Draw();
  void ChangeDrawState(std::unique_ptr<BaseDrawState> newState);
  void SetDrawingSheetSize(glm::vec2 size); 
  void OnMouseHovered(const glm::vec2 pos);
  void OnMouseButtonPressed(const glm::vec2 pos); // assuming only left mouse button can be pressed, BIG TODO: 
  void OnMouseButtonDown(const glm::vec2 pos);
  void OnMouseButtonReleased(const glm::vec2 pos);
  void OnTextInput(const std::string& inputText);
  
  // These are only for debug purpose
  BaseDrawState* GetDrawState() { return m_drawState.get(); }
  std::vector<glm::vec2> GetDrawingPoints(){ return std::vector<glm::vec2>{m_firstPoint, m_secondPoint};}

  // Hovering could be done in OnMouseHovered function, instead this functionality is covered by the draw states
  // because there can be a case when hovering is disabled by the draw command and the specific draw state
  std::optional<Entity> GetHoveredEntity(const glm::vec2 pos);
  std::vector<Entity> GetSelectedEntities();
  bool IsUnderSelectArea(Entity entity, glm::vec2 pos);
  bool IsPickpointSelected(Entity entity, glm::vec2 pos);
  bool IsDragAreaSelected(Entity entity, glm::vec2 pos);
  Entity CreateEntity(int id, const std::string& name);
  void DestroyEntity(Entity entity);
  Entity CreateRectangle(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType);
  void   UpdateRectangleShapeAttributes(Entity entity);
  Entity CreateCircle(glm::vec2 topLeft, glm::vec2 bottomRight, DrawObjectType objectType);
  void   UpdateCircleShapeAttributes(Entity entity);
  Entity CreateArrow(glm::vec2 topLeft, glm::vec2 bottomRight, DrawObjectType objectType);
  void   UpdateArrowShapeAttributes(Entity entity);
  void ClearSelectionShapes();

private:
  entt::registry m_registry;
  std::unique_ptr<ImageDocument> m_originalDoc;
  std::unique_ptr<Texture2D> m_drawing;

  std::optional<Entity> m_hoveredEntity;
  std::optional<Entity> m_draggedEntity;
  std::optional<Entity> m_toBeDrawnEntity;

  DrawCommand m_currentDrawCommand;
  std::unique_ptr<BaseDrawState> m_drawState; 

  glm::vec2 m_firstPoint{1.0f, 1.0f};
  glm::vec2 m_secondPoint{1.0f, 1.0f}; 
  glm::vec2 m_sheetSize{1.0f, 1.0f}; 
  
  // configs 
  static constexpr glm::vec4 s_selectBoxColor{0.23, 0.55, 0.70, 0.5};
  static constexpr glm::vec4 s_pickPointColor{0.14, 0.50, 0.62, 0.5};
  static constexpr float s_pickPointBoxSize = 0.02;
  friend class Entity;
  // state classes can be friend`s, because they are altering frequently the DrawingSheet`s variables
  friend class InitialObjectDrawState;
  friend class FirstClickRecievedState; 
  friend class DrawingTemporaryState;
  friend class ObjectSelectInitialState;
  friend class ObjectSelectionState;
  friend class ObjectSelectedState;
  friend class PickPointSelectedState;
  friend class ObjectDraggingState; 
};

using Entity = DrawingSheet::Entity;

// Draw states
class BaseDrawState
{
public:
  BaseDrawState(DrawingSheet* sheet, const std::string& stateName = "BaseDrawState") : m_sheet(sheet), m_stateName(stateName) {}
  const std::string& GetName() const {return m_stateName;}
  virtual void OnMouseHovered(const glm::vec2 pos) {}
  virtual void OnMouseButtonPressed(const glm::vec2 pos) {}
  virtual void OnMouseButtonDown(const glm::vec2 pos) {}
  virtual void OnMouseButtonReleased(const glm::vec2 pos) {}
  virtual void OnTextInput(const std::string& inputText) {}

  std::function<void(entt::entity)> DeleteTemporaries()
  {
    return [&](entt::entity e) {
      Entity entity = {e, m_sheet};
      if (entity.GetComponent<CommonAttributesComponent>().temporary)
        m_sheet->DestroyEntity(entity);
    };
  }
protected:
  DrawingSheet* m_sheet;
  std::string m_stateName;
};

class InitialObjectDrawState : public BaseDrawState
{
public:
  InitialObjectDrawState(DrawingSheet* sheet) : BaseDrawState(sheet, "InitialObjectDrawState") {m_sheet->ClearSelectionShapes();}
  void OnMouseHovered(const glm::vec2 pos) override;
  void OnMouseButtonPressed(const glm::vec2 pos) override;
};

class FirstClickRecievedState : public BaseDrawState
{
public:
  FirstClickRecievedState(DrawingSheet* sheet) : BaseDrawState(sheet, "FirstClickRecievedState") {}
  void OnMouseButtonDown(const glm::vec2 pos) override;       
  void OnMouseButtonReleased(const glm::vec2 pos) override;   
};

class DrawingTemporaryState : public BaseDrawState
{
public:
  DrawingTemporaryState(DrawingSheet* sheet) : BaseDrawState(sheet, "DrawingTemporaryState") {}
  void OnMouseButtonDown(const glm::vec2 pos) override;       
  void OnMouseButtonReleased(const glm::vec2 pos) override;   
};

// select states
class ObjectSelectInitialState : public BaseDrawState
{
public:
  ObjectSelectInitialState(DrawingSheet* sheet) : BaseDrawState(sheet, "ObjectSelectInitialState") {}
  void OnMouseHovered(const glm::vec2 pos) override;   
  void OnMouseButtonPressed(const glm::vec2 pos) override;
};

class ObjectSelectionState : public BaseDrawState
{
public:
  ObjectSelectionState(DrawingSheet* sheet) : BaseDrawState(sheet, "ObjectSelectionState") {}
  void OnMouseButtonDown(const glm::vec2 pos) override; 
  void OnMouseButtonReleased(const glm::vec2 pos) override;
};

class ObjectSelectedState : public BaseDrawState
{
public:
  ObjectSelectedState(DrawingSheet* sheet) : BaseDrawState(sheet, "ObjectSelectedState") {}
  void OnMouseButtonPressed(const glm::vec2 pos) override;
};

class PickPointSelectedState : public BaseDrawState
{
public:
  PickPointSelectedState(DrawingSheet* sheet) : BaseDrawState(sheet, "PickPointSelectedState") {}
  void OnMouseButtonDown(const glm::vec2 pos) override;
  void OnMouseButtonReleased(const glm::vec2 pos) override;
};

class ObjectDraggingState : public BaseDrawState
{
public:
  ObjectDraggingState(DrawingSheet* sheet) : BaseDrawState(sheet, "ObjectDraggingState") {}
  void OnMouseButtonDown(const glm::vec2 pos) override;
  void OnMouseButtonReleased(const glm::vec2 pos) override;
};
} // namespace medicimage
