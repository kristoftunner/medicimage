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
  bool IsComposed(){return m_entity.GetComponent<CommonAttributesComponent>().composed;}
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

class LineComponentWrapper : public BaseDrawComponentWrapper
{
public:
  LineComponentWrapper(Entity entity) : BaseDrawComponentWrapper(entity) {}
  /// @brief Factory function for creating an entity describing an arrow 
  /// @param baseEntity base entity to which we add all the other components 
  /// @return Entity containing the components needed for describing an arrow, TODO: should we return with LineComponentWrapper 
  static Entity CreateLine(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType);
  void UpdateShapeAttributes() override;
  void OnPickPointDrag(glm::vec2 diff, int selectedPoint) override;
  void OnObjectDrag(glm::vec2 diff) override;
  void Draw() override;
};

class TextComponentWrapper : public BaseDrawComponentWrapper
{
public:
  TextComponentWrapper(Entity entity) : BaseDrawComponentWrapper(entity) {}
  /// @brief Factory function for creating an entity describing an arrow 
  /// @param baseEntity base entity to which we add all the other components 
  /// @return Entity containing the components needed for describing an arrow, TODO: should we return with TextComponentWrapper 
  static Entity CreateText(glm::vec2 firstPoint, const std::string& inputText,int fontSize, DrawObjectType objectType);
  void UpdateShapeAttributes() override;
  void OnPickPointDrag(glm::vec2 diff, int selectedPoint) override {}
  void OnObjectDrag(glm::vec2 diff) override;
  void Draw() override;
};

/// @brief Component wrapper for Spline, it is special because it can be drawn from other components
///         It does not follow the first->second point drawing principles 
class SplineComponentWrapper : public BaseDrawComponentWrapper
{
public:
  SplineComponentWrapper(Entity entity) : BaseDrawComponentWrapper(entity) {}
  /// @brief Factory function for creating an entity describing an arrow 
  /// @param baseEntity base entity to which we add all the other components 
  /// @return Entity containing the components needed for describing an arrow, TODO: should we return with TextComponentWrapper 
  static Entity CreateSpline(glm::vec2 begin, glm::vec2 middle, glm::vec2 end, DrawObjectType objectType);
  void UpdateShapeAttributes() override {};
  void OnPickPointDrag(glm::vec2 diff, int selectedPoint) override {}
  void OnObjectDrag(glm::vec2 diff) override {};
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
  
  void SetLeftHorizontalWidthSpan(float span);
  void SetRightHorizontalWidthSpan(float span);
  void SetVertcialWidthSpan(float span);

  glm::ivec2 GetVerticalSliceCountBounds();
  glm::ivec2 GetLeftHorizontalSliceCountBounds();
  glm::ivec2 GetRightHorizontalSliceCountBounds();
private:
  static void GenerateSlices(Entity entity);

  static constexpr float s_minimumSliceSize = 0.015;

  static constexpr int s_defaultVerticalCount = 3;
  static constexpr float s_maxVerticalWidthSpan = 0.7;
  static constexpr float s_minVerticalWidthSpan = 0.3;
  static constexpr float s_defaultVerticalWidthSpan = 0.5;

  static constexpr int s_defaultHorizontalCount = 2;
  static constexpr float s_maxHorizontalHeightSpan = 0.8;
  static constexpr float s_minHorizontalHeightSpan = 0.2;
  static constexpr float s_defaultHorizontalHeightSpan = 0.5;
  static constexpr float s_maxHorizontalWidthSpan = 0.8;
  static constexpr float s_minHorizontalWidthSpan = 0.2;
};
} // namespace medicimage
