#include "drawing/component_wrappers.h"
#include "image_handling/image_editor.h"
#include "core/log.h"
#include "component_wrappers.h"

namespace medicimage
{
  Entity RectangleComponentWrapper::CreateRectangle(Entity baseEntity, glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
  {
    baseEntity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = baseEntity.GetComponent<TransformComponent>();
    auto& color = baseEntity.AddComponent<ColorComponent>();  
    auto& rectangle = baseEntity.AddComponent<RectangleComponent>();

    auto tmpRect = cv::Rect2f(cv::Point2f{ firstPoint.x, firstPoint.y }, cv::Point2f{ secondPoint.x, secondPoint.y });
    rectangle.width = tmpRect.width;
    rectangle.height = tmpRect.height; 

    glm::vec2 topleft{tmpRect.tl().x, tmpRect.tl().y};
    transform.translation = topleft;
    return baseEntity;
  }

  void RectangleComponentWrapper::UpdateShapeAttributes()
  {
    auto& rectangle = m_entity.GetComponent<RectangleComponent>();
    if(!m_entity.HasComponent<BoundingContourComponent>())
      m_entity.AddComponent<BoundingContourComponent>();
    if(!m_entity.HasComponent<PickPointsComponent>())
      m_entity.AddComponent<PickPointsComponent>();

    auto& boundingBox = m_entity.GetComponent<BoundingContourComponent>();
    auto& pickPoints = m_entity.GetComponent<PickPointsComponent>();
    
    boundingBox.cornerPoints = {{0,0}, {rectangle.width, 0}, {rectangle.width, rectangle.height}, {0, rectangle.height}, {0,0}};
    pickPoints.pickPoints = {glm::vec2{rectangle.width, rectangle.height} + glm::vec2{0, -rectangle.height / 2},
      glm::vec2{rectangle.width, rectangle.height} + glm::vec2{-rectangle.width / 2, 0}, {0, rectangle.height / 2}, glm::vec2{rectangle.width / 2, 0}};
  }

  void RectangleComponentWrapper::OnPickPointDrag(glm::vec2 diff, int selectedPoint)
  {
    switch(selectedPoint)  // [TODO REFACTOR]: extract these out into functions
    {
      case static_cast<int>(RectanglePicPoints::RIGHT):
      {
        m_entity.GetComponent<RectangleComponent>().width += diff.x;
        break;
      }
      case static_cast<int>(RectanglePicPoints::BOTTOM):
      {
        m_entity.GetComponent<RectangleComponent>().height += diff.y;
        break;
      }
      case static_cast<int>(RectanglePicPoints::LEFT):
      {
        m_entity.GetComponent<RectangleComponent>().width += -diff.x;
        m_entity.GetComponent<TransformComponent>().translation += glm::vec2{diff.x, 0};
        break;
      }
      case static_cast<int>(RectanglePicPoints::TOP):
      {
        m_entity.GetComponent<RectangleComponent>().height += -diff.y;
        m_entity.GetComponent<TransformComponent>().translation += glm::vec2{0, diff.y};
        break;
      }
      default:
        APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPoint, m_entity.GetComponent<IDComponent>().ID);
        break;
    } 
    UpdateShapeAttributes();
  }

  void RectangleComponentWrapper::OnObjectDrag(glm::vec2 diff)
  {
    // TODO: some checks wether we drag the component outside or not etc..
    auto& transform = m_entity.GetComponent<TransformComponent>();
    transform.translation += diff;
  }

  void RectangleComponentWrapper::Draw()
  {
    auto& transform  = m_entity.GetComponent<TransformComponent>();
    auto& commonAttributes = m_entity.GetComponent<CommonAttributesComponent>();
    auto& rectangle = m_entity.GetComponent<RectangleComponent>();
    auto& color = m_entity.GetComponent<ColorComponent>().color;
    auto& topleft = transform.translation;
    auto bottomright = topleft + glm::vec2{rectangle.width, rectangle.height}; 
    ImageEditor::DrawRectangle(topleft, bottomright, color, rectangle.thickness, commonAttributes.filled);
    
    if(commonAttributes.selected)
    {
      auto& pickPoints = m_entity.GetComponent<PickPointsComponent>().pickPoints;
      for(auto& point : pickPoints)
      {
        ImageEditor::DrawCircle(point + transform.translation, s_pickPointBoxSize / 2, s_pickPointColor, 2, true);
      }
    }
  }

  Entity medicimage::CircleComponentWrapper::CreateCircle(Entity baseEntity, glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
  {
    baseEntity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = baseEntity.GetComponent<TransformComponent>();
    transform.translation = firstPoint;

    auto& color = baseEntity.AddComponent<ColorComponent>();  
    auto& circle = baseEntity.AddComponent<CircleComponent>();
    circle.radius = glm::length(firstPoint - secondPoint);

    return baseEntity;
  }

  void CircleComponentWrapper::UpdateShapeAttributes()
  {
    auto& circle = m_entity.GetComponent<CircleComponent>();
    
    if(!m_entity.HasComponent<BoundingContourComponent>())
      m_entity.AddComponent<BoundingContourComponent>();
    if(!m_entity.HasComponent<PickPointsComponent>())
      m_entity.AddComponent<PickPointsComponent>();
    
    auto& boundingBox = m_entity.GetComponent<BoundingContourComponent>();
    auto& pickPoints = m_entity.GetComponent<PickPointsComponent>();
    auto radius = circle.radius;
    // bounding polyigon of the circle is a square actually
    boundingBox.cornerPoints = {{-radius, -radius}, {radius, -radius}, {radius, radius}, {-radius, radius}, {0,0}};
    pickPoints.pickPoints = {{radius, 0}, {0, circle.radius}, {-circle.radius, 0}, {0, -circle.radius}};
  }

  void CircleComponentWrapper::OnPickPointDrag(glm::vec2 diff, int selectedPoint)
  {
    switch(selectedPoint)  // [TODO REFACTOR]: extract these out into functions
    {
      case static_cast<int>(CirclePickPoints::RIGHT):
        m_entity.GetComponent<CircleComponent>().radius += diff.x;
        break;
      case static_cast<int>(CirclePickPoints::LEFT):
        m_entity.GetComponent<CircleComponent>().radius += -diff.x;
        break;
      case static_cast<int>(CirclePickPoints::BOTTOM):
        m_entity.GetComponent<CircleComponent>().radius += diff.y;
        break;
      case static_cast<int>(CirclePickPoints::TOP):
        m_entity.GetComponent<CircleComponent>().radius += -diff.y;
        break;
      default:
        APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPoint, m_entity.GetComponent<IDComponent>().ID);
        break;
    } 
    UpdateShapeAttributes(); 
  }

  void CircleComponentWrapper::OnObjectDrag(glm::vec2 diff)
  {
    // TODO: some checks wether we drag the component outside or not etc..
    auto& transform = m_entity.GetComponent<TransformComponent>();
    transform.translation += diff;
  }
 
  void CircleComponentWrapper::Draw()
  {
    auto& transform  = m_entity.GetComponent<TransformComponent>();
    auto& commonAttributes = m_entity.GetComponent<CommonAttributesComponent>();
    auto& circle = m_entity.GetComponent<CircleComponent>();
    auto& color = m_entity.GetComponent<ColorComponent>().color;
    auto& center = transform.translation;
    ImageEditor::DrawCircle(center, circle.radius, color, circle.thickness, commonAttributes.filled);

    if(commonAttributes.selected)
    {
      auto& pickPoints = m_entity.GetComponent<PickPointsComponent>().pickPoints;
      auto& translation = m_entity.GetComponent<TransformComponent>().translation;
      for(auto& point : pickPoints)
      {
        
        ImageEditor::DrawCircle(point + translation, s_pickPointBoxSize / 2, s_pickPointColor, 2, true);
      }
    } 
  }

  Entity ArrowComponentWrapper::CreateArrow(Entity baseEntity, glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
  {
    baseEntity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = baseEntity.GetComponent<TransformComponent>();
    transform.translation = firstPoint;

    auto& color = baseEntity.AddComponent<ColorComponent>();  
    auto& arrow = baseEntity.AddComponent<ArrowComponent>();
    arrow.end = secondPoint - firstPoint;

    return baseEntity;
  }

  void ArrowComponentWrapper::UpdateShapeAttributes()
  {
    auto& arrow = m_entity.GetComponent<ArrowComponent>();

    if(!m_entity.HasComponent<BoundingContourComponent>())
      m_entity.AddComponent<BoundingContourComponent>();
    if(!m_entity.HasComponent<PickPointsComponent>())
      m_entity.AddComponent<PickPointsComponent>();
    auto& boundingBox = m_entity.GetComponent<BoundingContourComponent>();
    auto& pickPoints = m_entity.GetComponent<PickPointsComponent>();

    glm::vec2 vec = arrow.begin - arrow.end; 
    glm::vec2 perp = glm::normalize(glm::vec2{-vec.y, vec.x});
    glm::vec2 offset = perp * glm::length(vec) * glm::vec2(0.2);
    boundingBox.cornerPoints = {offset, arrow.end + offset, arrow.end - offset, -offset, offset};
    pickPoints.pickPoints = {arrow.begin, arrow.end};
  }

  void ArrowComponentWrapper::OnPickPointDrag(glm::vec2 diff, int selectedPoint)
  {
    switch(selectedPoint)  // [TODO REFACTOR]: extract these out into functions
    {
      case static_cast<int>(ArrowPickPoints::BEGIN):
        m_entity.GetComponent<ArrowComponent>().begin += diff;
        break;
      case static_cast<int>(ArrowPickPoints::END):
        m_entity.GetComponent<ArrowComponent>().end += diff;
        break;
      default:
        APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPoint, m_entity.GetComponent<IDComponent>().ID);
        break;
    } 
    UpdateShapeAttributes();
  }
 
  void ArrowComponentWrapper::OnObjectDrag(glm::vec2 diff)
  {
    // TODO: some checks wether we drag the component outside or not etc..
    auto& transform = m_entity.GetComponent<TransformComponent>();
    transform.translation += diff;
  }
  
  void ArrowComponentWrapper::Draw()
  {
    auto& transform  = m_entity.GetComponent<TransformComponent>();
    auto& commonAttributes = m_entity.GetComponent<CommonAttributesComponent>();
    auto& arrow = m_entity.GetComponent<ArrowComponent>();
    auto& color = m_entity.GetComponent<ColorComponent>().color;
    auto begin = arrow.begin + transform.translation; 
    auto end = arrow.end + transform.translation; 
    ImageEditor::DrawArrow(begin, end, color, arrow.thickness, 0.1);
    
    if(commonAttributes.selected)
    {
      auto& pickPoints = m_entity.GetComponent<PickPointsComponent>().pickPoints;
      for(auto& point : pickPoints)
      {
        ImageEditor::DrawCircle(point + transform.translation, s_pickPointBoxSize / 2, s_pickPointColor, 2, true);
      }
    }
  }

  Entity SkinTemplateComponentWrapper::CreateSkinTemplate(Entity baseEntity, glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
  {
    return baseEntity;
  }

  void SkinTemplateComponentWrapper::UpdateShapeAttributes()
  {
  }

  void SkinTemplateComponentWrapper::OnPickPointDrag(glm::vec2 diff, int selectedPoint)
  {
  }

  void SkinTemplateComponentWrapper::OnObjectDrag(glm::vec2 diff)
  {
  }

  void SkinTemplateComponentWrapper::Draw()
  {
  }

  glm::vec2 SkinTemplateComponentWrapper::GetVerticalSliceWidthBounds()
  {
    return glm::vec2(1.0);
  }

  glm::vec2 SkinTemplateComponentWrapper::GetVerticalSliceHeightBounds()
  {
    return glm::vec2(1.0);
  }

  glm::vec2 SkinTemplateComponentWrapper::GetHorizontalSliceWidthBounds()
  {
    return glm::vec2(1.0);
  }

  glm::vec2 SkinTemplateComponentWrapper::GetHorizontalSliceHeightBounds()
  {
    return glm::vec2(1.0);
  }
} // namespace medicimage