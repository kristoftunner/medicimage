#pragma once

#include "drawing/drawing_sheet.h"

namespace medicimage
{

/// @brief these are non-owning wrappers, containing the functionalities needed to work
///         on the components linked to the draw component. It does not own the 
class BaseDrawComponentWrapper 
{
public:
  BaseDrawComponentWrapper(Entity entity) : m_entity(entity){}
  virtual void UpdateShapeAttributes() = 0;
  virtual void OnPickPointDrag(glm::vec2 diff, int selectedPoint) = 0;
  virtual void OnObjectDrag(glm::vec2 diff) = 0;
  virtual void Draw() = 0;
  Entity GetEntity(){return m_entity;}
protected:
  Entity m_entity;
  static constexpr glm::vec4 s_selectBoxColor{0.23, 0.55, 0.70, 0.5};
  static constexpr glm::vec4 s_pickPointColor{0.14, 0.50, 0.62, 0.5};
  static constexpr float s_pickPointBoxSize = 0.02;
};

class RectangleComponentWrapper : public BaseDrawComponentWrapper
{
public:
  RectangleComponentWrapper(Entity entity) : BaseDrawComponentWrapper(entity) {}
  /// @brief Factory function for creating an entity describing a rectangle 
  /// @param baseEntity base entity to which we add all the other components 
  /// @return Entity containing the components needed for describing a rectangle, TODO: should we return with RectangleComponentWrapper 
  static Entity CreateRectangle(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType);
  void UpdateShapeAttributes() override;
  void OnPickPointDrag(glm::vec2 diff, int selectedPoint) override;
  void OnObjectDrag(glm::vec2 diff) override;
  void Draw() override;
};

class CircleComponentWrapper : public BaseDrawComponentWrapper
{
public:
  CircleComponentWrapper(Entity entity) : BaseDrawComponentWrapper(entity) {}
  /// @brief Factory function for creating an entity describing a circle 
  /// @param baseEntity base entity to which we add all the other components 
  /// @return Entity containing the components needed for describing a circle, TODO: should we return with CircleComponentWrapper 
  static Entity CreateCircle(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType);
  void UpdateShapeAttributes() override;
  void OnPickPointDrag(glm::vec2 diff, int selectedPoint) override;
  void OnObjectDrag(glm::vec2 diff) override;
  void Draw() override;
};

class ArrowComponentWrapper : public BaseDrawComponentWrapper
{
public:
  ArrowComponentWrapper(Entity entity) : BaseDrawComponentWrapper(entity) {}
  /// @brief Factory function for creating an entity describing an arrow 
  /// @param baseEntity base entity to which we add all the other components 
  /// @return Entity containing the components needed for describing an arrow, TODO: should we return with ArrowComponentWrapper 
  static Entity CreateArrow(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType);
  void UpdateShapeAttributes() override;
  void OnPickPointDrag(glm::vec2 diff, int selectedPoint) override;
  void OnObjectDrag(glm::vec2 diff) override;
  void Draw() override;
};

class SkinTemplateComponentWrapper : public BaseDrawComponentWrapper
{
public:
  SkinTemplateComponentWrapper(Entity entity) : BaseDrawComponentWrapper(entity) {}
  /// @brief Factory function for creating an entity describing an skin template 
  /// @param baseEntity base entity to which we add all the other components 
  /// @return Entity containing the components needed for describing an skin temaplte, TODO: should we return with SkinTemplateComponentWrapper 
  static Entity CreateSkinTemplate(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType);
  void UpdateShapeAttributes() override;
  void OnPickPointDrag(glm::vec2 diff, int selectedPoint) override;
  void OnObjectDrag(glm::vec2 diff) override;
  void Draw() override;
  glm::vec2 GetVerticalSliceWidthBounds();
  glm::vec2 GetVerticalSliceHeightBounds();
  glm::vec2 GetHorizontalSliceWidthBounds();
  glm::vec2 GetHorizontalSliceHeightBounds();
};
} // namespace medicimage
