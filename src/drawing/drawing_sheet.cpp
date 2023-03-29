#include "drawing/drawing_sheet.h"
#include "drawing/components.h"
#include "drawing/drawing_sheet.h"
#include "drawing/component_wrappers.h"
#include "core/log.h"
#include "image_handling/image_editor.h"
#include <algorithm>
#include <chrono>

#include <glm/gtx/perpendicular.hpp>
#include "drawing_sheet.h"

namespace medicimage
{
  void DrawingSheet::SetDocument(std::unique_ptr<ImageDocument> doc, glm::vec2 viewportSize)
  {
    // clear the registry for clear drawing
    auto objects = Entity::View<TransformComponent>();
    for(auto e : objects)
    {
      Entity entity(e);
      Entity::DestroyEntity(entity);
    }

    m_sheetSize = viewportSize;
    m_originalDoc = std::move(doc);
    m_drawing = std::make_unique<Texture2D>(m_originalDoc->texture->GetTexturePtr(), "texture");
  }
  
  void DrawingSheet::SetDrawCommand(const DrawCommand command)
  {
    m_currentDrawCommand = command;

    // set the initial command state according to the draw command
    switch(command)
    {
      case DrawCommand::DRAW_CIRCLE:
      case DrawCommand::DRAW_RECTANGLE:
      case DrawCommand::DRAW_ELLIPSE:
      case DrawCommand::DRAW_ARROW:
      case DrawCommand::DRAW_LINE:
      case DrawCommand::DRAW_SKIN_TEMPLATE:
      {
        m_drawState = std::make_unique<InitialObjectDrawState>(this);
        break;
      }
      case DrawCommand::DRAW_TEXT:
      {
        m_drawState = std::make_unique<DrawTextInitialState>(this);
        break;
      }
      case DrawCommand::DRAW_INCREMENTAL_LETTERS:
      {
        m_drawState = std::make_unique<DrawIncrementalLetters>(this);
        break;
      }
      case DrawCommand::DO_NOTHING:
      {
        m_drawState = std::make_unique<BaseDrawState>(this);
        break;
      }
    }
  }

  std::unique_ptr<Texture2D> DrawingSheet::Draw()
  {
    std::stringstream ss;
    ss << std::put_time(std::localtime(&(m_originalDoc->timestamp)), "%d-%b-%Y %X");
    std::string footerText = m_originalDoc->documentId + " - " + ss.str();
    m_drawing = ImageEditor::ReplaceImageFooter(footerText, m_originalDoc->texture.get());

    ImageEditor::Begin(m_drawing.get());
    auto circles = Entity::View<CircleComponent>();
    for(auto e : circles)
    {
      Entity entity(e);
      CircleComponentWrapper cw(entity);
      if(!cw.IsComposed())
        cw.Draw();
    }

    auto rectangles = Entity::View<RectangleComponent>();
    for(auto e : rectangles)
    {
      Entity entity(e);
      RectangleComponentWrapper rw(entity);
      if(!rw.IsComposed())
        rw.Draw(); 
    }
    
    auto arrows = Entity::View<ArrowComponent>();
    for(auto e : arrows)
    {
      Entity entity(e);
      ArrowComponentWrapper aw(entity);
      if(!aw.IsComposed())
        aw.Draw();
    }
    
    auto lines = Entity::View<LineComponent>();
    for(auto e : lines)
    {
      Entity entity(e);
      LineComponentWrapper lw(entity);
      if(!lw.IsComposed())
        lw.Draw();
    }
    
    auto texts = Entity::View<TextComponent>();
    for(auto e : texts)
    {
      Entity entity(e);
      TextComponentWrapper tw(entity);
      if(!tw.IsComposed())
        tw.Draw();
    }

    auto skinTemplates = Entity::View<SkinTemplateComponent>();
    for(auto e : skinTemplates)
    {
      Entity entity(e);
      SkinTemplateComponentWrapper sw(entity);
      if(!sw.IsComposed())
        sw.Draw();
    }
    
    auto splines = Entity::View<SplineComponent>();
    for(auto e : splines)
    {
      Entity entity(e);
      SplineComponentWrapper sw(entity);
      if(!sw.IsComposed())
        sw.Draw();
    }

    ImageEditor::End(m_drawing.get());

    std::for_each(circles.begin(), circles.end(), m_drawState->DeleteTemporaries());
    std::for_each(rectangles.begin(), rectangles.end(), m_drawState->DeleteTemporaries());
    std::for_each(arrows.begin(), arrows.end(), m_drawState->DeleteTemporaries());
    std::for_each(lines.begin(), lines.end(), m_drawState->DeleteTemporaries());
    std::for_each(texts.begin(), texts.end(), m_drawState->DeleteTemporaries());
    std::for_each(skinTemplates.begin(), skinTemplates.end(), m_drawState->DeleteTemporaries());
    std::for_each(splines.begin(), splines.end(), m_drawState->DeleteTemporaries());

    return std::move(std::make_unique<Texture2D>(*m_drawing.get()));
  }

  void DrawingSheet::ChangeDrawState(std::unique_ptr<BaseDrawState> newState)
  {
    m_drawState = std::move(newState);
  }

  void DrawingSheet::SetDrawingSheetSize(glm::vec2 size)
  {
    m_sheetSize = size;
  }

  void DrawingSheet::OnMouseHovered(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnMouseHovered(pos);
  }

  void DrawingSheet::OnMouseButtonPressed(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnMouseButtonPressed(pos);
  }

  void DrawingSheet::OnMouseButtonDown(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnMouseButtonDown(pos);
  }

  void DrawingSheet::OnMouseButtonReleased(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnMouseButtonReleased(pos);
  }

  void DrawingSheet::OnTextInput(const std::string &inputText)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnTextInput(inputText);
  }

  void DrawingSheet::OnKeyPressed(KeyCode key)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnKeyPressed(key);
  }

  void DrawingSheet::OnUpdate()
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnUpdate();
  }

  std::optional<Entity> DrawingSheet::GetHoveredEntity(const glm::vec2 pos)
  {
    // TODO: may want to move this into editor ui, so here only relative coordinates are handled
    const glm::vec2 relPos = pos / m_sheetSize;
    auto view = Entity::View<BoundingContourComponent>();
    for(auto e : view)
    {
      Entity entity(e);
      auto boundingContour = entity.GetComponent<BoundingContourComponent>().cornerPoints;
      if(boundingContour.size() == 0)
        continue;

      auto translation = entity.GetComponent<TransformComponent>().translation;
      for(auto& c : boundingContour)
        c += translation;
      std::vector<cv::Point2f> contour;
      std::transform(boundingContour.begin(), boundingContour.end(), std::back_inserter(contour), [](glm::vec2 vec) {return cv::Point2f{ vec.x, vec.y }; });
      if(cv::pointPolygonTest(contour, cv::Point2f{relPos.x, relPos.y}, false)  >= 0)
      {
        APP_CORE_TRACE("Entity:{} is hovered", entity.GetComponent<IDComponent>().ID);
        return entity;
      }
    }
    return std::optional<Entity>();
  }

  std::vector<Entity> DrawingSheet::GetSelectedEntities()
  {
    std::vector<Entity> selectedEntities;
    auto view = Entity::View<CommonAttributesComponent>();
    for(auto e : view)
    {
      Entity entity(e);
      if(entity.GetComponent<CommonAttributesComponent>().selected)
        selectedEntities.push_back(entity);
    }
    return selectedEntities;
  }
  bool DrawingSheet::IsUnderSelectArea(Entity entity, glm::vec2 pos)
  {
    auto boundingContour = entity.GetComponent<BoundingContourComponent>().cornerPoints;
    auto translation = entity.GetComponent<TransformComponent>().translation;
    for(auto& c : boundingContour)
      c += translation;
    std::vector<cv::Point2f> entityContour;
    std::transform(boundingContour.begin(), boundingContour.end(), std::back_inserter(entityContour), [](glm::vec2 vec) {return cv::Point2f{ vec.x, vec.y }; });
      
    auto dx = m_secondPoint.x - m_firstPoint.x;
    auto dy = m_secondPoint.y - m_firstPoint.y;
    std::vector<cv::Point2f> selectContour{cv::Point2f{m_firstPoint.x, m_firstPoint.y}, cv::Point2f{m_firstPoint.x + dx, m_firstPoint.y},
      cv::Point2f{m_secondPoint.x, m_secondPoint.y}, cv::Point2f{m_secondPoint.x - dx, m_secondPoint.y}};
      
    std::vector<cv::Point2f> tmp;
    if (cv::intersectConvexConvex(selectContour, entityContour, tmp, true) > 0.0)
    {
      if(cv::intersectConvexConvex(selectContour, entityContour, tmp, false) == 0.0)
      {
        APP_CORE_INFO("Entity:{} selected with bb: tl:{}:{}, tr:{}:{}, br:{}:{}, bl:{}:{}", entity.GetComponent<IDComponent>().ID,
          boundingContour[0].x, boundingContour[0].y, boundingContour[1].x, boundingContour[1].y, boundingContour[2].x, boundingContour[2].y, boundingContour[3].x, boundingContour[3].y);
        return true;
      }
    }
    return false;
  }

  bool DrawingSheet::IsPickpointSelected(Entity entity, glm::vec2 pos)
  {
    auto pickPoints = entity.GetComponent<PickPointsComponent>().pickPoints;
    assert(pickPoints.size() != 0);
    auto translation = entity.GetComponent<TransformComponent>().translation;
    for(auto& p : pickPoints)
      p += translation;
    for(int i = 0; i < pickPoints.size(); i++)
    {
      auto point = pickPoints[i];
      std::vector<cv::Point2f> contour {{point.x - s_pickPointBoxSize / 2, point.y - s_pickPointBoxSize / 2}, {point.x + s_pickPointBoxSize / 2, point.y - s_pickPointBoxSize / 2},
        {point.x + s_pickPointBoxSize / 2, point.y + s_pickPointBoxSize / 2}, {point.x - s_pickPointBoxSize / 2, point.y + s_pickPointBoxSize / 2}};
      if(cv::pointPolygonTest(contour, cv::Point2f{pos.x, pos.y}, false)  >= 0)
      {
        entity.GetComponent<PickPointsComponent>().selectedPoint = i;
        return true;
      }
    }
    return false;
  }

  bool DrawingSheet::IsDragAreaSelected(Entity entity, glm::vec2 pos)
  {
    auto boundingContour = entity.GetComponent<BoundingContourComponent>().cornerPoints;
    assert(boundingContour.size() != 0);
    auto translation = entity.GetComponent<TransformComponent>().translation;
    for(auto& c : boundingContour)
      c += translation;
    std::vector<cv::Point2f> contour;
    std::transform(boundingContour.begin(), boundingContour.end(), std::back_inserter(contour), [](glm::vec2 vec) {return cv::Point2f{ vec.x, vec.y }; });
    if(cv::pointPolygonTest(contour, cv::Point2f{pos.x, pos.y}, false)  >= 0)
      return true;
    else
      return false;
  }
  
  void DrawingSheet::ClearSelectionShapes()
  {
    // clear the selection and selected pickpoint
    auto attributes = Entity::View<CommonAttributesComponent>();
    for(auto e : attributes)
    {
      Entity entity(e);
      if(entity.GetComponent<CommonAttributesComponent>().selected)
        entity.GetComponent<CommonAttributesComponent>().selected = false;
    }

    auto pickpoints = Entity::View<PickPointsComponent>();
    for(auto e : pickpoints)
    {
      Entity entity(e);
      entity.GetComponent<PickPointsComponent>().selectedPoint = -1;
    }
    
    m_draggedEntity.reset();
  }


  void InitialObjectDrawState::OnMouseHovered(const glm::vec2 pos)
  {
    m_sheet->m_hoveredEntity = m_sheet->GetHoveredEntity(pos);
  }

  void InitialObjectDrawState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->ChangeDrawState(std::make_unique<FirstClickRecievedState>(m_sheet)); 
  }

  void FirstClickRecievedState::OnMouseButtonDown(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize; 
    m_sheet->ChangeDrawState(std::make_unique<DrawingTemporaryState>(m_sheet)); 
  }

  void FirstClickRecievedState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    // fallback to initial state, because it was just an accidental single click
    m_sheet->ChangeDrawState(std::make_unique<InitialObjectDrawState>(m_sheet));
  }


  void DrawingTemporaryState::OnMouseButtonDown(const glm::vec2 pos)
  { // TODO REFACTOR: implement here the visitor pattern and extract the creaton functionality into a class 
    m_sheet->m_secondPoint = pos / m_sheet->m_sheetSize;
    Entity entity;

    switch(m_sheet->m_currentDrawCommand)
    {
      case DrawCommand::DRAW_CIRCLE:
      {
        CircleComponentWrapper cw(CircleComponentWrapper::CreateCircle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY));
        cw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_RECTANGLE:
      {
        RectangleComponentWrapper rw(RectangleComponentWrapper::CreateRectangle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY));
        rw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_ARROW:
      {
        ArrowComponentWrapper aw(ArrowComponentWrapper::CreateArrow(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY));
        aw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_LINE:
      {
        LineComponentWrapper lw(LineComponentWrapper::CreateLine(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY));
        lw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_SKIN_TEMPLATE:
      {
        SkinTemplateComponentWrapper sw(SkinTemplateComponentWrapper::CreateSkinTemplate(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY));
        sw.UpdateShapeAttributes();
        break;
      }
    }
  }

  void DrawingTemporaryState::OnMouseButtonReleased(const glm::vec2 pos)
  { // TODO REFACTOR: see above 
    m_sheet->m_secondPoint = pos / m_sheet->m_sheetSize;
    Entity entity;
    switch(m_sheet->m_currentDrawCommand)
    {
      case DrawCommand::DRAW_CIRCLE:
      {
        CircleComponentWrapper cw(CircleComponentWrapper::CreateCircle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT));
        cw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_RECTANGLE:
      {
        RectangleComponentWrapper rw(RectangleComponentWrapper::CreateRectangle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT));
        rw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_ARROW:
      {
        ArrowComponentWrapper aw(ArrowComponentWrapper::CreateArrow(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT));
        aw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_LINE:
      {
        LineComponentWrapper lw(LineComponentWrapper::CreateLine(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT));
        lw.UpdateShapeAttributes();
        break;
      }
      case DrawCommand::DRAW_SKIN_TEMPLATE:
      {
        SkinTemplateComponentWrapper sw(SkinTemplateComponentWrapper::CreateSkinTemplate(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::PERMANENT));
        sw.UpdateShapeAttributes();
        break;
      }
    }
    
    m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
  }
  
  void DrawTextInitialState::OnMouseHovered(const glm::vec2 pos)
  { // change the mouse cursor to text editor cursor
    ;
  }
  
  void DrawTextInitialState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize; 
    m_sheet->ChangeDrawState(std::make_unique<DrawTextState>(m_sheet)); 
  }

  void DrawTextState::OnTextInput(const std::string &inputText)
  {
    m_text += inputText;
  }
  
  void DrawTextState::OnKeyPressed(KeyCode key)
  { // exit the command if enter is pressed
    if(key == Key::MDIK_RETURN)
    {
      if(m_text != "")
      {
        TextComponentWrapper tw(TextComponentWrapper::CreateText(m_sheet->m_firstPoint, m_text, 4, DrawObjectType::PERMANENT));
        tw.UpdateShapeAttributes();
      }
      m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
      m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
    }
    else if(key == Key::MDIK_BACKSPACE)
    {
      if(m_text != "")
        m_text.pop_back();
    }
  }
  
  void DrawTextState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    if(m_text != "")
    {
        TextComponentWrapper tw(TextComponentWrapper::CreateText(m_sheet->m_firstPoint, m_text, 4, DrawObjectType::PERMANENT));
        tw.UpdateShapeAttributes();
    }
    m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
  }

  void DrawTextState::OnUpdate()
  {
    if(m_text != "")
    {
      TextComponentWrapper tw(TextComponentWrapper::CreateText(m_sheet->m_firstPoint, m_text, 4, DrawObjectType::TEMPORARY));
      tw.UpdateShapeAttributes();
    }
  }
  
  void DrawIncrementalLetters::OnKeyPressed(KeyCode key)
  {
    if(key == Key::MDIK_RETURN)
    {
      m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
      m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
      
    }
  }
  void DrawIncrementalLetters::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize; 
    TextComponentWrapper tw(TextComponentWrapper::CreateText(m_sheet->m_firstPoint, m_text, 2, DrawObjectType::PERMANENT));
    tw.UpdateShapeAttributes();
    IncrementLetter();
  }

  void DrawIncrementalLetters::IncrementLetter()
  {
    if(m_text == "ZZ")
    {
      m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
      m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
    }
    
    std::string::iterator currentLetter = m_text.end()-1;
    if(m_text.size() > 2)
      APP_CORE_ERR("Something went wrong with the letter increments");
    
    char& c = *currentLetter;
    if((c >= 'A') && (c < 'Z'))
      c++;
    else if(c == 'Z')
    {
      if(m_text.size() == 1)
      {
        m_text += 'A';
        c = 'A';
      }
      else if(m_text.size() == 2)
      {
        m_text.at(0)++;
        c = 'A';
      }
    }
  }

  void ObjectSelectInitialState::OnMouseHovered(const glm::vec2 pos)
  {
    m_sheet->m_hoveredEntity = m_sheet->GetHoveredEntity(pos);
  }

  void ObjectSelectInitialState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize; 
    auto entity = m_sheet->GetHoveredEntity(pos);
    if(entity.has_value())
    {
      entity.value().GetComponent<CommonAttributesComponent>().selected = true; 
      m_sheet->ChangeDrawState(std::make_unique<ObjectSelectedState>(m_sheet));
    }
    else
    {
      m_sheet->ChangeDrawState(std::make_unique<ObjectSelectionState>(m_sheet));
    }
  }

  void ObjectSelectionState::OnMouseButtonDown(const glm::vec2 pos)
  {
    m_sheet->m_secondPoint = pos / m_sheet->m_sheetSize;
    auto& firstPoint = m_sheet->m_firstPoint;
    auto& secondPoint = m_sheet->m_secondPoint;
    if ((firstPoint.x != secondPoint.x) && (firstPoint.y != secondPoint.y)) // TODO: may not need this
    { // baby blue select rectangle
      RectangleComponentWrapper rw(RectangleComponentWrapper::CreateRectangle(m_sheet->m_firstPoint, m_sheet->m_secondPoint, DrawObjectType::TEMPORARY));
      auto entity = rw.GetEntity();
      entity.GetComponent<ColorComponent>().color = m_sheet->s_selectBoxColor;
      entity.GetComponent<ThicknessComponent>().thickness = 2;
      entity.GetComponent<CommonAttributesComponent>().filled = true;
    }
  }

  void ObjectSelectionState::OnMouseButtonReleased(const glm::vec2 pos)
  { // If we have selected entities, then go back to initial select state, else go forward
    bool hasSelectedObject = false;
    auto view = Entity::View<BoundingContourComponent>();
    for(auto e : view)
    {
      Entity entity = {e};
      if (m_sheet->IsUnderSelectArea(entity, pos))
      {
        entity.GetComponent<CommonAttributesComponent>().selected = true;
        hasSelectedObject = true;
      }
    }

    if(hasSelectedObject)
      m_sheet->ChangeDrawState(std::make_unique<ObjectSelectedState>(m_sheet)); 
    else
      m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet)); 
  }

  void ObjectSelectedState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize;
    auto view = Entity::View<PickPointsComponent>();
    for(auto e : view)
    {
      Entity entity(e);
      if(m_sheet->IsPickpointSelected(entity, m_sheet->m_firstPoint))
      {
        m_sheet->ChangeDrawState(std::make_unique<PickPointSelectedState>(m_sheet));
        return; 
      }
      else if(m_sheet->IsDragAreaSelected(entity, m_sheet->m_firstPoint))
      {
        m_sheet->m_draggedEntity = entity;
        m_sheet->ChangeDrawState(std::make_unique<ObjectDraggingState>(m_sheet)); 
        return;
      }
    }
   
    // clicking outside of the pickpoints and drage area, clear selection and go back to initial state
    m_sheet->ClearSelectionShapes();
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
  }

  void PickPointSelectedState::OnMouseButtonDown(const glm::vec2 pos)
  {
    auto currentPoint = pos / m_sheet->m_sheetSize; 
    auto diff = (currentPoint - m_sheet->m_firstPoint) * glm::vec2(1.0);
    m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize;
    auto view = Entity::View<PickPointsComponent>();
    for(auto e : view)
    {
      Entity entity(e);
      auto selectedPointIndex = entity.GetComponent<PickPointsComponent>().selectedPoint;
      if(selectedPointIndex != -1)
      {
        if (entity.HasComponent<RectangleComponent>())
        {
          RectangleComponentWrapper rw(entity);
          rw.OnPickPointDrag(diff, selectedPointIndex);
        }
        else if(entity.HasComponent<CircleComponent>())
        {
          CircleComponentWrapper cw(entity);
          cw.OnPickPointDrag(diff, selectedPointIndex);
        }
        else if(entity.HasComponent<ArrowComponent>())
        {
          ArrowComponentWrapper aw(entity);
          aw.OnPickPointDrag(diff, selectedPointIndex);
        }
        else if(entity.HasComponent<LineComponent>())
        {
          LineComponentWrapper lw(entity);
          lw.OnPickPointDrag(diff, selectedPointIndex);
        }
        else if(entity.HasComponent<SkinTemplateComponent>())
        {
          SkinTemplateComponentWrapper sw(entity);
          sw.OnPickPointDrag(diff, selectedPointIndex);
        }
        else if(entity.HasComponent<TextComponent>())
        { // Do nothing wiht tex component
          ;
        }
        else
          APP_CORE_ERR("WTF this component");
      }
    }
  }

  void PickPointSelectedState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    // clear pickpoint selection
    auto view = Entity::View<PickPointsComponent>();
    for(auto e : view)
    {
      Entity entity(e);
      entity.GetComponent<PickPointsComponent>().selectedPoint = -1;
    }
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectedState>(m_sheet));
  }

  void ObjectDraggingState::OnMouseButtonDown(const glm::vec2 pos)
  {
    if(m_sheet->m_draggedEntity.has_value())
    { // TODO: call here the OnObjectDrag function
      auto currentPoint = pos / m_sheet->m_sheetSize; 
      auto diff = (currentPoint - m_sheet->m_firstPoint) * glm::vec2(1.0);
      m_sheet->m_firstPoint = pos / m_sheet->m_sheetSize;
      auto& transform = m_sheet->m_draggedEntity.value().GetComponent<TransformComponent>();
      transform.translation += diff;
    }
    else
      APP_CORE_ERR("Something went wrong with dragging");
  }

  void ObjectDraggingState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectedState>(m_sheet));
  }

} // namespace medicimage
