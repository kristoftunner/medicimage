#include "drawing/component_wrappers.h"
#include "image_handling/image_editor.h"
#include "core/log.h"
#include "component_wrappers.h"

namespace medicimage
{
  Entity RectangleComponentWrapper::CreateRectangle(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
  {
    auto entity = DrawingSheet::CreateEntity(0, "rectangle");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = entity.GetComponent<TransformComponent>();
    auto& color = entity.AddComponent<ColorComponent>();  
    auto& rectangle = entity.AddComponent<RectangleComponent>();

    auto tmpRect = cv::Rect2f(cv::Point2f{ firstPoint.x, firstPoint.y }, cv::Point2f{ secondPoint.x, secondPoint.y });
    rectangle.width = tmpRect.width;
    rectangle.height = tmpRect.height; 

    glm::vec2 topleft{tmpRect.tl().x, tmpRect.tl().y};
    transform.translation = topleft;
    return entity;
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

  Entity medicimage::CircleComponentWrapper::CreateCircle(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
  {
    auto entity = DrawingSheet::CreateEntity(0, "Circle");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = entity.GetComponent<TransformComponent>();
    transform.translation = firstPoint;

    auto& color = entity.AddComponent<ColorComponent>();  
    auto& circle = entity.AddComponent<CircleComponent>();
    circle.radius = glm::length(firstPoint - secondPoint);

    return entity;
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

  Entity ArrowComponentWrapper::CreateArrow(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
  {
    auto entity = DrawingSheet::CreateEntity(0, "Arrow");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = entity.GetComponent<TransformComponent>();
    transform.translation = firstPoint;

    auto& color = entity.AddComponent<ColorComponent>();  
    auto& arrow = entity.AddComponent<ArrowComponent>();
    arrow.end = secondPoint - firstPoint;

    return entity;
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

  Entity SkinTemplateComponentWrapper::CreateSkinTemplate(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
  {
    auto entity = DrawingSheet::CreateEntity(0, "Skin template");
    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = entity.GetComponent<TransformComponent>();
    auto& skinTemplate = entity.AddComponent<SkinTemplateComponent>();

    static constexpr float minBoundingWidth = 0.1;
    static constexpr float minBoundingHeight = 0.1;
    static constexpr int verticalSliceCount = 3;
    static constexpr int horizontalSliceCount = 1;
    auto diff = secondPoint - firstPoint;
    if(abs(diff.x) >= minBoundingWidth && abs(diff.y) >= minBoundingHeight)
    {
      auto boundingRectSize = glm::abs(diff);
      skinTemplate.verticalSliceSize = glm::vec2{ diff.x / 10.0, diff.y };
      skinTemplate.horizontalSliceSize = glm::vec2{ diff.x * (7.0 / 20.0), diff.y / 5.0 };
      skinTemplate.boundingRectSize = glm::abs(diff);
      auto center = firstPoint + diff / glm::vec2(2.0);
      for (auto i = 0; i < verticalSliceCount; i++)
      {
        glm::vec2 vertTopLeft = glm::vec2{ - skinTemplate.verticalSliceSize.x * 1.5, -skinTemplate.verticalSliceSize.y / 2.0 } + center + glm::vec2{ i * skinTemplate.verticalSliceSize.x, 0.0 };
        glm::vec2 vertBottomRight = glm::vec2{ -skinTemplate.verticalSliceSize.x * 0.5, skinTemplate.verticalSliceSize.y / 2.0 } + center + glm::vec2{ i * skinTemplate.verticalSliceSize.x, 0.0 };
        auto rect = RectangleComponentWrapper::CreateRectangle(vertTopLeft, vertBottomRight, objectType);
      }
      for (auto i = 0; i < horizontalSliceCount; i++)
      {
        glm::vec2 horTopLeft = glm::vec2{ -skinTemplate.horizontalSliceSize.x * 1.5, -skinTemplate.horizontalSliceSize.y / 2.0 } + center + glm::vec2{ i * skinTemplate.horizontalSliceSize.x, 0.0 };
        glm::vec2 horBottomRight = glm::vec2{ -skinTemplate.horizontalSliceSize.x * 0.5, skinTemplate.horizontalSliceSize.y / 2.0 } + center + glm::vec2{ i * skinTemplate.horizontalSliceSize.x, 0.0 };
        RectangleComponentWrapper::CreateRectangle(horTopLeft, horBottomRight, objectType);
      }
    }

    return entity;
  }

  void SkinTemplateComponentWrapper::UpdateShapeAttributes()
  {
    auto& skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    if(!m_entity.HasComponent<BoundingContourComponent>())
      m_entity.AddComponent<BoundingContourComponent>();
    if(!m_entity.HasComponent<PickPointsComponent>())
      m_entity.AddComponent<PickPointsComponent>();

    auto& boundingBox = m_entity.GetComponent<BoundingContourComponent>();
    auto& pickPoints = m_entity.GetComponent<PickPointsComponent>();
    
    auto boundingRectSize = skinTemplate.boundingRectSize;
    boundingBox.cornerPoints = {{0,0}, {boundingRectSize.x, 0}, {boundingRectSize.x, boundingRectSize.y}, {0, boundingRectSize.y}, {0,0}};
    pickPoints.pickPoints = {glm::vec2{boundingRectSize.x, boundingRectSize.y} + glm::vec2{0, -boundingRectSize.y / 2},
      glm::vec2{boundingRectSize.x, boundingRectSize.y} + glm::vec2{-boundingRectSize.x / 2, 0}, {0, boundingRectSize.y / 2}, glm::vec2{boundingRectSize.x / 2, 0}};
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