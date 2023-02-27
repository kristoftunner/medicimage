#pragma once

#include "drawing/entity.h"
#include "image_handling/image_saver.h"
#include <glm/glm.hpp>
#include <string>
#include <optional>
namespace medicimage
{


// forward declaration of the states
class BaseDrawState;
class InitialObjectDrawState;
class FirstClickRecievedState; 
class DrawingTemporaryState;
class ObjectSelectInitialState;
class ObjectSelectionState; 

enum class DrawCommand{INITIAL, OBJECT_SELECT, DRAW_CIRCLE, DRAW_RECTANGLE, DRAW_ARROW, DRAW_ELLIPSE, DRAW_TEXT}; // Possible commands: DRAG(MOVE), 
enum class DrawObjectType{TEMPORARY, PERMANENT};

class DrawingSheet
{
public:
public:
  void SetDocument(std::unique_ptr<ImageDocument> doc, glm::vec2 viewportSize); 
  void SetDrawCommand(const DrawCommand command); // initialize the state with the command's init state
  void Draw();
  void ChangeDrawState(std::unique_ptr<BaseDrawState> newState);
  
  void OnMouseHovered(const glm::vec2 pos);
  void OnMouseButtonPressed(const glm::vec2 pos); // assuming only left mouse button can be pressed, BIG TODO: 
  void OnMouseButtonDown(const glm::vec2 pos);
  void OnMouseButtonReleased(const glm::vec2 pos);
  void OnTextInput(const std::string& inputText);

  void AddToSelection(Entity entity);
  void ClearSelection();

  // Hovering could be done in OnMouseHovered function, instead this functionality is covered by the draw states
  // because there can be a case when hovering is disabled by the draw command and the specific draw state
  std::optional<Entity> GetHoveredEntity(const glm::vec2 pos);
  void SetHoveredEntity(Entity entity);
  std::vector<Entity> GetEntitiesUnderSelection();

  static Entity CreateRectangle(glm::vec2 topLeft, glm::vec2 bottomRight, DrawObjectType objectType);
  Entity CreateEntity(int id, const std::string& name);
private:
  entt::registry m_registry;
  std::unique_ptr<ImageDocument> m_document;

  std::vector<Entity> m_selectedEntities;
  Entity m_hoveredEntity;

  DrawCommand m_currentDrawCommand;
  std::unique_ptr<BaseDrawState> m_drawState; 

  glm::vec2 m_firstPoint{1.0f, 1.0f};
  glm::vec2 m_secondPoint{1.0f, 1.0f}; 
  glm::vec2 m_viewportSize{1.0f, 1.0f}; 
  friend class Entity;

  // state classes can be friend`s, because they are altering frequently the DrawingSheet`s variables
  friend class InitialObjectDrawState;
  friend class FirstClickRecievedState; 
  friend class DrawingTemporaryState;
  friend class ObjectSelectInitialState;
  friend class ObjectSelectionState; 
};

class BaseDrawState
{
public:
  BaseDrawState(DrawingSheet* sheet) : m_sheet(sheet) {}
  virtual void OnMouseHovered(const glm::vec2 pos) {}
  virtual void OnMouseButtonPressed(const glm::vec2 pos) {}
  virtual void OnMouseButtonDown(const glm::vec2 pos) {}
  virtual void OnMouseButtonReleased(const glm::vec2 pos) {}
  virtual void OnTextInput(const std::string& inputText) {}
protected:
  DrawingSheet* m_sheet;
};

// Draw states
class InitialObjectDrawState : public BaseDrawState
{
public:
  InitialObjectDrawState(DrawingSheet* sheet) : BaseDrawState(sheet) {}
  void OnMouseHovered(const glm::vec2 pos) override;
  void OnMouseButtonPressed(const glm::vec2 pos) override;
};

class FirstClickRecievedState : public BaseDrawState
{
public:
  FirstClickRecievedState(DrawingSheet* sheet) : BaseDrawState(sheet) {}
  void OnMouseButtonDown(const glm::vec2 pos) override;       
  void OnMouseButtonReleased(const glm::vec2 pos) override;   
};

class DrawingTemporaryState : public BaseDrawState
{
public:
  DrawingTemporaryState(DrawingSheet* sheet) : BaseDrawState(sheet) {}
  void OnMouseButtonDown(const glm::vec2 pos) override;       
  void OnMouseButtonReleased(const glm::vec2 pos) override;   
};

// select states
class ObjectSelectInitialState : public BaseDrawState
{
public:
  ObjectSelectInitialState(DrawingSheet* sheet) : BaseDrawState(sheet) {}
  void OnMouseHovered(const glm::vec2 pos) override;   
  void OnMouseButtonPressed(const glm::vec2 pos) override;
};

class ObjectSelectionState : public BaseDrawState
{
public:
  ObjectSelectionState(DrawingSheet* sheet) : BaseDrawState(sheet) {}
  void OnMouseButtonDown(const glm::vec2 pos) override; 
  void OnMouseButtonReleased(const glm::vec2 pos) override;
};

class ObjectsSelectedState : public BaseDrawState
{
public:
  ObjectsSelectedState(DrawingSheet* sheet) : BaseDrawState(sheet) {}
  void OnMouseButtonPressed(const glm::vec2 pos);
};

} // namespace medicimage
