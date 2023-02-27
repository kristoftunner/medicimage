#pragma once


#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

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

  struct ColorComponent
  {
    glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

    ColorComponent() = default;
    ColorComponent(glm::vec4 color) : color(color){}
  };

  struct CircleComponent
  {
    // center is described by the translation in the transform component
    float radius = 0.0f;
    float thickness = 1.0f; // might extract this attribute also out into a component
    bool temporary = true;

    CircleComponent(float radius = 0.0f, float thickness = 1.0f, bool temporary)
      : radius(radius), thickness(thickness), temporary(temporary) {}
  };

  struct RectangleComponent
  {
    // width and height are also relative, should be scaled by the pixel size of the image 
    float height = 0.0f;
    float width = 0.0f;
    float thickness = 1.0f;
    bool temporary = true;

    RectangleComponent(float width = 0.0f, float height = 0.0f, float thickness = 1.0f, bool temporary)
      : width(width), height(height), thickness(thickness), temporary(temporary) {}
  };

  struct ArrowComponent
  {
    glm::vec2 begin = {0.0f, 0.0f};
    glm::vec2 end {0.0f, 0.0f};
    float thickness = 1.0f;
    bool temporary = true;

    ArrowComponent(glm::vec2 begin, glm::vec2 end, float thickness, bool temporary)
      : begin(begin), end(end), thickness(thickness), temporary(temporary) {}
  };
} // namespace medicimage
