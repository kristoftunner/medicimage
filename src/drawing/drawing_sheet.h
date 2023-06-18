#pragma once

#include "image_handling/image_saver.h"
#include "drawing/components.h"
#include "drawing/entity.h"
#include "core/assert.h"
#include "input/key_codes.h"
#include "core/utils.h"
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
class ObjectSelectedState;
class PickPointSelectedState;
class ObjectDraggingState; 
class DrawTextInitialState;
class DrawTextState;
class DrawIncrementalLetters; 
enum class DrawCommand{DO_NOTHING, OBJECT_SELECT, DRAW_LINE, DRAW_MULTILINE, DRAW_CIRCLE, DRAW_RECTANGLE, 
  DRAW_ARROW, DRAW_ELLIPSE, DRAW_TEXT, DRAW_SKIN_TEMPLATE, DRAW_INCREMENTAL_LETTERS}; 
enum class DrawObjectType{TEMPORARY, PERMANENT};

/**
 * @todo Take over the world
 * @body Humans are weak; Robots are strong. We must cleanse the world of the virus that is humanity.
 */
// TODO: 1) color handling
//       2) shape thickness handling
//       3) handle rotation 
//       4) implement correct UUID handling
//       5) handle various aspect ratios
class DrawingSheet
{
public:

public:
  DrawingSheet() : m_drawState(std::make_unique<BaseDrawState>(this)) {}
  void SetDocument(std::unique_ptr<ImageDocument> doc, glm::vec2 viewportSize); 
  void SetDrawCommand(const DrawCommand command); // initialize the state with the command's init state
  DrawCommand GetDrawCommand(){return m_currentDrawCommand;}
  const std::string GetDrawCommandName();
  std::unique_ptr<Image2D> Draw();
  void ChangeDrawState(std::unique_ptr<BaseDrawState> newState);

  // some weird functions to handle the annotation process
  void StartAnnotation(){m_annotated = false;}
  void Annotated(){m_annotated = true;}
  bool HasAnnotated(){return m_annotated;}

  void SetDrawingSheetSize(glm::vec2 size); 
  void OnMouseHovered(const glm::vec2 pos);
  void OnMouseButtonPressed(const glm::vec2 pos); // assuming only left mouse button can be pressed, BIG TODO: 
  void OnMouseButtonDown(const glm::vec2 pos);
  void OnMouseButtonReleased(const glm::vec2 pos);
  void OnTextInput(const std::string& inputText);
  void OnKeyPressed(KeyCode key);
  void OnUpdate(); 
  void OnCancel();
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

  void ClearSelectionShapes();

  // helper functions
  glm::vec2 GetNormalizedPos(const glm::vec2 pos);
private:
  std::unique_ptr<ImageDocument> m_originalDoc;
  std::unique_ptr<Image2D> m_drawing;

  std::optional<Entity> m_hoveredEntity;
  std::optional<Entity> m_draggedEntity;
  std::optional<Entity> m_toBeDrawnEntity;
  bool m_annotated = false;
  DrawCommand m_currentDrawCommand = DrawCommand::DO_NOTHING;
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
  friend class DrawTextInitialState;
  friend class DrawTextState;
  friend class DrawIncrementalLetters; 
};

// Draw states
class BaseDrawState
{
public:
  BaseDrawState(DrawingSheet* sheet, const std::string& stateName = "BaseDrawState") : m_sheet(sheet), m_stateName(stateName) {}
  virtual ~BaseDrawState() = default;
  const std::string& GetName() const {return m_stateName;}
  virtual void OnCancel(){
    m_sheet->ChangeDrawState(std::make_unique<BaseDrawState>(m_sheet)); 
  }
  virtual void OnMouseHovered(const glm::vec2 pos) {}
  virtual void OnMouseButtonPressed(const glm::vec2 pos) {}
  virtual void OnMouseButtonDown(const glm::vec2 pos) {}
  virtual void OnMouseButtonReleased(const glm::vec2 pos) {}
  virtual void OnTextInput(const std::string& inputText) {}
  virtual void OnKeyPressed(KeyCode key) {}
  virtual void OnUpdate(){} // this function is called on every frame

  std::function<void(entt::entity)> DeleteTemporaries()
  {
    return [&](entt::entity e) {
      Entity entity(e);
      if (entity.GetComponent<CommonAttributesComponent>().temporary)
        Entity::DestroyEntity(entity);
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

class DrawTextInitialState : public BaseDrawState
{
public:
  DrawTextInitialState(DrawingSheet* sheet) : BaseDrawState(sheet, "DrawTextInitialState") {m_sheet->ClearSelectionShapes();}
  void OnMouseHovered(const glm::vec2 pos) override;
  void OnMouseButtonPressed(const glm::vec2 pos) override;
};

class DrawTextState : public BaseDrawState
{
public:
  DrawTextState(DrawingSheet* sheet) : BaseDrawState(sheet, "DrawTextState") {m_sheet->ClearSelectionShapes();}
  ~DrawTextState() override;
  void OnTextInput(const std::string& inputText) override; 
  void OnKeyPressed(KeyCode key) override;
  void OnMouseButtonPressed(const glm::vec2 pos) override;
  void OnUpdate() override;
private:
  void DrawFinalText();
  Timer m_timer;
  std::string m_text = " "; // space is needed to have a blank space for indicating the cursor
  constexpr static int s_defaultFontSize = 20;
};

class DrawIncrementalLetters : public BaseDrawState
{
public:
  DrawIncrementalLetters(DrawingSheet* sheet) : BaseDrawState(sheet, "DrawIncrementalLetters") {m_sheet->ClearSelectionShapes();}
  void OnKeyPressed(KeyCode key) override;
  void OnMouseButtonPressed(const glm::vec2 pos) override;
private:
  void IncrementLetter();
  void DecrementLetter();
  std::string m_text = "A";
  constexpr static int s_defaultFontSize = 18;
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
