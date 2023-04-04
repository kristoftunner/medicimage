#pragma once


#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace medicimage
{
	struct IDComponent
	{
		int ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};
	
  struct TagComponent
	{
		std::string tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: tag(tag) {}
	};
	
  struct TransformComponent
	{ 
    // all the values are relative to the drawing sheet resolution
		glm::vec2 translation = { 0.0f, 0.0f};
		glm::vec2 rotation = { 0.0f, 0.0f};
	  
    // this might be extracted into the specific components,
    // because a rectangle has 2 DoF, while the circle has only 1
    glm::vec2 scale = { 1.0f, 1.0f};  

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: translation(translation) {}
	};

  struct BoundingContourComponent
  {
    std::vector<glm::vec2> cornerPoints;  // storing the contour of the bounding contour
    BoundingContourComponent() = default;
    BoundingContourComponent(const BoundingContourComponent&) = default;
    BoundingContourComponent(const std::vector<glm::vec2>& points) : cornerPoints(points) {}
  };

  struct PickPointsComponent
  {
    std::vector<glm::vec2> pickPoints;
    int selectedPoint = -1;
    PickPointsComponent() = default;
    PickPointsComponent(const PickPointsComponent&) = default;
    PickPointsComponent(const std::vector<glm::vec2>& points) : pickPoints(points) {}
  };

  enum class RectanglePicPoints {RIGHT=0, BOTTOM=1, LEFT=2, TOP=3};
  enum class CirclePickPoints {RIGHT=0, BOTTOM=1, LEFT=2, TOP=3};
  enum class SkinTemplatePickPoints {RIGHT=0, BOTTOM=1, LEFT=2, TOP=3, 
    LEFT_SLICES_TOP, LEFT_SLICES_BOTTOM, MIDDLE_SLICES_LEFT, MIDDLE_SLICES_RIGHT, RIGHT_SLICES_TOP, RIGHT_SLICES_BOTTOM};
  enum class ArrowPickPoints {BEGIN=0, END=1};
  enum class LinePickPoints {BEGIN=0, END=1};

  struct ColorComponent
  {
    glm::vec4 color{1.0f, 0.0f, 0.0f, 0.0f};

    ColorComponent() = default;
    ColorComponent(glm::vec4 color) : color(color){}
  };

  struct ThicknessComponent
  {
    int thickness = 3;
    ThicknessComponent() = default;
  };

  struct CommonAttributesComponent
  {
    bool selected = false;
    bool temporary = true;
    bool filled = false;
    bool composed = false;
    CommonAttributesComponent() = default;
    CommonAttributesComponent(bool selected, bool temporary) : selected(selected), temporary(temporary){} 
  };

  struct CircleComponent
  {
    // center is described by the translation in the transform component
    float radius = 0.0f;  // normalized
  
    CircleComponent() = default;
    CircleComponent(float radius)
      : radius(radius){}
  };

  struct RectangleComponent
  {
    // width and height are also relative, should be scaled by the pixel size of the image 
    float height = 0.0f;
    float width = 0.0f;
  
    RectangleComponent() = default;
    RectangleComponent(float width, float height)
      : width(width), height(height) {}
  };

  struct ArrowComponent
  {
    glm::vec2 begin{0.0f, 0.0f};  // this is 0,0 in most cases, only changes when editing with pickpoints, because it seems logicaly correct to use instead of the transform
    glm::vec2 end {0.0f, 0.0f}; 
    
    ArrowComponent() = default;
    ArrowComponent(glm::vec2 end, glm::vec2 begin)
      : end(end), begin(begin) {}
  };
  
  struct LineComponent 
  {
    glm::vec2 begin{0.0f, 0.0f};  // this is 0,0 in most cases, only changes when editing with pickpoints, because it seems logicaly correct to use instead of the transform
    glm::vec2 end {0.0f, 0.0f}; 
    
    LineComponent() = default;
    LineComponent(glm::vec2 end, glm::vec2 begin)
      : end(end), begin(begin) {}
  };

  struct SplineComponent
  {
     glm::vec2 begin{0.0f, 0.0f};
     glm::vec2 middle{0.0f, 0.0f};
     glm::vec2 end{0.0f, 0.0f};
     int lineCount = 0;
  };

  struct SkinTemplateComponent
  {
    glm::vec2 boundingRectSize{0.0, 0.0};
    int leftHorSliceCount = 0;
    int rightHorSliceCount = 0;
    int vertSliceCount = 0;
    float vertSliceWidthSpan = 0.0;
    float leftHorSliceWidthSpan = 0.0;
    float leftHorSliceHeightSpan = 0.0;
    float rightHorSliceWidthSpan = 0.0;
    float rightHorSliceHeightSpan = 0.0;
    std::vector<entt::entity> leftHorizontalSlices; // TODO: here we should store the UUID of the rectangle
    std::vector<entt::entity> rightHorizontalSlices; // TODO: here we should store the UUID of the rectangle
    std::vector<entt::entity> verticalSlices;
    std::vector<entt::entity> splines;
    bool drawable = false;
    bool drawSpline = false;
    SkinTemplateComponent() = default;
  };

  struct TextComponent
  {
    std::string text = "";
    int fontSize = 1;
    glm::vec2 boxSize{0.0, 0.0};
    TextComponent() = default;
  };
} // namespace medicimage
