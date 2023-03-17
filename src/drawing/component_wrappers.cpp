#include "drawing/component_wrappers.h"
#include "image_handling/image_editor.h"
#include "core/log.h"
#include "component_wrappers.h"

namespace medicimage
{
  Entity RectangleComponentWrapper::CreateRectangle(glm::vec2 firstPoint, glm::vec2 secondPoint, DrawObjectType objectType)
  {
    auto entity = Entity::CreateEntity(0, "rectangle");
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
    auto entity = Entity::CreateEntity(0, "Circle");
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
    auto entity = Entity::CreateEntity(0, "Arrow");
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
    auto entity = Entity::CreateEntity(0, "Skin template");

    entity.GetComponent<CommonAttributesComponent>().temporary = objectType == DrawObjectType::TEMPORARY ? true : false;
    
    auto& transform = entity.GetComponent<TransformComponent>();
    transform.translation = firstPoint;
    auto& skinTemplate = entity.AddComponent<SkinTemplateComponent>();
    auto& color = entity.AddComponent<ColorComponent>();  

    static constexpr float minBoundingWidth = 0.1;
    static constexpr float minBoundingHeight = 0.1;
    auto diff = secondPoint - firstPoint;
    if(abs(diff.x) >= minBoundingWidth && abs(diff.y) >= minBoundingHeight)
    {
      auto boundingRectSize = glm::abs(diff);
      skinTemplate.boundingRectSize = glm::abs(diff);
      skinTemplate.horizontalSliceCount = s_defaultHorizontalCount;
      skinTemplate.verticalSliceCount = s_defaultVerticalCount;
      skinTemplate.verticalSliceWidthSpan = s_defaultVerticalSpan;
      skinTemplate.horizontalSliceHeightSpan = s_defaultHorizontalSpan;

      // check the vertical slice width and horizontal height
      auto verticalSliceWidth = skinTemplate.verticalSliceWidthSpan  * skinTemplate.boundingRectSize.x / s_defaultVerticalCount;
      auto horizontalSliceHeight =  skinTemplate.horizontalSliceHeightSpan * skinTemplate.boundingRectSize.y / s_defaultHorizontalCount; 
      auto horizontalSliceWidth = (1 - skinTemplate.verticalSliceWidthSpan) / 2 * skinTemplate.boundingRectSize.x;
      assert(verticalSliceWidth > s_minimumSliceSize);
      assert(horizontalSliceHeight > s_minimumSliceSize);
      //skinTemplate.verticalSliceSize = glm::vec2{verticalSliceWidth, skinTemplate.boundingRectSize.y};
      //skinTemplate.horizontalSliceSize = glm::vec2{horizontalSliceWidth, horizontalSliceHeight};
      GenerateSlices(entity);
    }
    return entity;
  }

  void SkinTemplateComponentWrapper::GenerateSlices(Entity entity)
  {
    auto& transform = entity.GetComponent<TransformComponent>();
    auto& skinTemplate = entity.GetComponent<SkinTemplateComponent>();
    auto center = transform.translation + skinTemplate.boundingRectSize / glm::vec2(2.0);
    auto leftMidPoint = transform.translation + glm::vec2{0.0, skinTemplate.boundingRectSize.y / 2.0};

    // clear the current rectangles, because we will generate the new ones according to the number of slices and the vertical/horizontal sizes
    for(auto e : skinTemplate.horizontalSlices)
    {
      Entity::DestroyEntity(Entity(e));
    }
    skinTemplate.horizontalSlices.clear();
    for(auto e : skinTemplate.verticalSlices)
    {
      Entity::DestroyEntity(Entity(e));
    }
    skinTemplate.verticalSlices.clear();

    // check the sizes
    auto& color = entity.GetComponent<ColorComponent>();
    auto objectType = entity.GetComponent<CommonAttributesComponent>().temporary ? DrawObjectType::TEMPORARY : DrawObjectType::PERMANENT;
    for (auto i = 0; i < skinTemplate.verticalSliceCount; i++)
    {
      glm::vec2 sliceSize{skinTemplate.verticalSliceWidthSpan / skinTemplate.verticalSliceCount * skinTemplate.boundingRectSize.x, skinTemplate.boundingRectSize.y};
      
      auto leftShift = glm::vec2{ -(skinTemplate.verticalSliceCount * 0.5) * sliceSize.x, 0.0 };
      glm::vec2 vertTopLeft =  center + glm::vec2{ i * sliceSize.x, -sliceSize.y / 2.0} + leftShift;
      glm::vec2 vertBottomRight = center + glm::vec2{ (i + 1) * sliceSize.x, sliceSize.y / 2.0 } + leftShift;
      auto rect = RectangleComponentWrapper::CreateRectangle(vertTopLeft, vertBottomRight, objectType);
      rect.GetComponent<ColorComponent>() = color; 
      rect.GetComponent<CommonAttributesComponent>().composed = true;
      skinTemplate.verticalSlices.push_back(rect.GetHandle());
    }
    for (auto i = 0; i < skinTemplate.horizontalSliceCount; i++)
    {
      glm::vec2 sliceSize{(1 - skinTemplate.verticalSliceWidthSpan) / 2 * skinTemplate.boundingRectSize.x, 
        skinTemplate.horizontalSliceHeightSpan / skinTemplate.horizontalSliceCount * skinTemplate.boundingRectSize.y};
      
      auto upShift = glm::vec2{0.0, -(skinTemplate.horizontalSliceCount * 0.5) * sliceSize.y};
      glm::vec2 horTopLeft = leftMidPoint + glm::vec2{0.0, i * sliceSize.y} + upShift;
      glm::vec2 horBottomRight = leftMidPoint + glm::vec2{sliceSize.x, (i + 1) * sliceSize.y} + upShift;
      auto rect = RectangleComponentWrapper::CreateRectangle(horTopLeft, horBottomRight, objectType);
      rect.GetComponent<ColorComponent>() = color; 
      rect.GetComponent<CommonAttributesComponent>().composed = true;
      skinTemplate.horizontalSlices.push_back(rect.GetHandle());
    }
  }
  
  glm::vec2 SkinTemplateComponentWrapper::GetVerticalSliceWidthSpanBounds()
  {
    return glm::vec2{s_minVerticalSpan, s_maxVerticalSpan};
  }

  glm::vec2 SkinTemplateComponentWrapper::GetHorizontalSliceHeightSpanBounds()
  {
    return glm::vec2{s_minHorizontalSpan, s_maxHorizontalSpan};
  }

  glm::ivec2 SkinTemplateComponentWrapper::GetVerticalSliceCountBounds()
  {
    auto& skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    auto maxBound = (skinTemplate.verticalSliceWidthSpan * skinTemplate.boundingRectSize.x) / s_minimumSliceSize;
    return glm::ivec2{1, static_cast<int>(maxBound)};
  }

  glm::ivec2 SkinTemplateComponentWrapper::GetHorizontalSliceCountBounds()
  {
    auto& skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    auto maxBound = (skinTemplate.horizontalSliceHeightSpan * skinTemplate.boundingRectSize.y) / s_minimumSliceSize; 
    return glm::ivec2{1, static_cast<int>(maxBound)};
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

    GenerateSlices(m_entity);
  }

  void SkinTemplateComponentWrapper::OnPickPointDrag(glm::vec2 diff, int selectedPoint)
  {
    switch(selectedPoint)  // [TODO REFACTOR]: extract these out into functions
    {
      case static_cast<int>(SkinTemplatePickPoints::RIGHT):
      {
        auto& skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        float xScale = diff.x / skinTemplate.boundingRectSize.x;
        skinTemplate.boundingRectSize.x += diff.x;
        //skinTemplate.horizontalSliceSize.x *= (1 + xScale);
        //skinTemplate.verticalSliceSize.x *= (1 + xScale);
        GenerateSlices(m_entity);
        break;
      }
      case static_cast<int>(SkinTemplatePickPoints::BOTTOM):
      {
        auto& skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        float yScale = diff.y / skinTemplate.boundingRectSize.y;
        skinTemplate.boundingRectSize.y += diff.y;
        //skinTemplate.horizontalSliceSize.y *= (1 + yScale);
        //skinTemplate.verticalSliceSize.y *= (1 + yScale);
        GenerateSlices(m_entity);
        break;
      }
      case static_cast<int>(SkinTemplatePickPoints::LEFT):
      {
        auto& skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        float xScale = (-diff.x) / skinTemplate.boundingRectSize.x;
        skinTemplate.boundingRectSize.x += -diff.x;
        //skinTemplate.horizontalSliceSize.x *= (1 + xScale);
        //skinTemplate.verticalSliceSize.x *= (1 + xScale);
        m_entity.GetComponent<TransformComponent>().translation += glm::vec2{diff.x, 0};
        GenerateSlices(m_entity);
        break;
      }
      case static_cast<int>(SkinTemplatePickPoints::TOP):
      {
        auto& skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
        float yScale = (-diff.y) / skinTemplate.boundingRectSize.y;
        skinTemplate.boundingRectSize.y += -diff.y;
        //skinTemplate.horizontalSliceSize.y *= (1 + yScale);
        //skinTemplate.verticalSliceSize.y *= (1 + yScale);
        m_entity.GetComponent<TransformComponent>().translation += glm::vec2{0, diff.y};
        GenerateSlices(m_entity);
        break;
      }
      default:
        APP_CORE_ERR("Wrong pickpoint index({}) at component:{}", selectedPoint, m_entity.GetComponent<IDComponent>().ID);
        break;
    } 
    UpdateShapeAttributes(); 
  }

  void SkinTemplateComponentWrapper::OnObjectDrag(glm::vec2 diff)
  {
    // TODO: some checks wether we drag the component outside or not etc..
    auto& transform = m_entity.GetComponent<TransformComponent>();
    transform.translation += diff;
    auto& skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    for(auto e : skinTemplate.horizontalSlices)
    {
      Entity rect(e);
      rect.GetComponent<TransformComponent>().translation += diff;
    }
    for(auto e : skinTemplate.verticalSlices)
    {
      Entity rect(e);
      rect.GetComponent<TransformComponent>().translation += diff;
    }
  }

  void SkinTemplateComponentWrapper::Draw()
  {
    auto& skinTemplate = m_entity.GetComponent<SkinTemplateComponent>();
    for(auto e : skinTemplate.verticalSlices)
    {
      Entity rect(e);
      RectangleComponentWrapper rw(rect);
      rw.Draw();
    }
    for(auto e : skinTemplate.horizontalSlices)
    {
      Entity rect(e);
      RectangleComponentWrapper rw(rect);
      rw.Draw();
    }

    auto commonAttributes = m_entity.GetComponent<CommonAttributesComponent>();
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


} // namespace medicimage