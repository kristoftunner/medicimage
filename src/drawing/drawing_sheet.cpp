#include "drawing/drawing_sheet.h"
#include "drawing/components.h"
#include "drawing/drawing_sheet.h"
#include "drawing/component_wrappers.h"
#include "core/log.h"
#include "image_handling/image_editor.h"
#include <algorithm>
#include <chrono>
#include <memory>

#include <opencv2/imgproc.hpp>
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
    m_drawing = std::make_unique<Image2D>(*m_originalDoc->image.get());
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
      case DrawCommand::DRAW_MULTILINE:
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

  const std::string DrawingSheet::GetDrawCommandName()
  {
    switch(m_currentDrawCommand)
    {
      case DrawCommand::DRAW_CIRCLE: return "DRAW_CIRCLE";
      case DrawCommand::DRAW_RECTANGLE: return "DRAW_RECTANGLE";
      case DrawCommand::DRAW_ELLIPSE: return "DRAW_ELLIPSE";
      case DrawCommand::DRAW_ARROW: return "DRAW_ARROW";
      case DrawCommand::DRAW_LINE: return "DRAW_LINE";
      case DrawCommand::DRAW_MULTILINE: return "DRAW_MULTILINE";
      case DrawCommand::DRAW_TEXT: return "DRAW_TEXT";
      case DrawCommand::DRAW_INCREMENTAL_LETTERS: return "DRAW_INCREMENTAL_LETTERS";
      case DrawCommand::DRAW_SKIN_TEMPLATE: return "DRAW_SKIN_TEMPLATE";
      case DrawCommand::DO_NOTHING: return "DO_NOTHING";
      default: return "UNKNOWN";
    }
  }

  std::unique_ptr<Image2D> DrawingSheet::Draw()
  {
    std::stringstream ss;
    ss << std::put_time(std::localtime(&(m_originalDoc->timestamp)), "%d-%b-%Y %X");
    std::string footerText = m_originalDoc->documentId + " - " + ss.str();
    m_drawing = ImageEditor::AddImageFooter(footerText, *m_originalDoc->image.get());

    ImageEditor::Begin(std::move(m_drawing));
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

    m_drawing = ImageEditor::End();

    std::for_each(circles.begin(), circles.end(), m_drawState->DeleteTemporaries());
    std::for_each(rectangles.begin(), rectangles.end(), m_drawState->DeleteTemporaries());
    std::for_each(arrows.begin(), arrows.end(), m_drawState->DeleteTemporaries());
    std::for_each(lines.begin(), lines.end(), m_drawState->DeleteTemporaries());
    std::for_each(texts.begin(), texts.end(), m_drawState->DeleteTemporaries());
    std::for_each(skinTemplates.begin(), skinTemplates.end(), m_drawState->DeleteTemporaries());
    std::for_each(splines.begin(), splines.end(), m_drawState->DeleteTemporaries());

    return std::make_unique<Image2D>(*m_drawing.get());
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
    {
      ImageEditor::Begin(std::move(m_drawing));
      m_drawState->OnMouseHovered(pos);
      m_drawing = ImageEditor::End();
    }
  }

  void DrawingSheet::OnMouseButtonPressed(const glm::vec2 pos)
  {
    if (m_currentDrawCommand != DrawCommand::DO_NOTHING)
    {
      ImageEditor::Begin(std::move(m_drawing));
      m_drawState->OnMouseButtonPressed(pos);
      m_drawing = ImageEditor::End();
    }
  }

  void DrawingSheet::OnMouseButtonDown(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
    {
      ImageEditor::Begin(std::move(m_drawing));
      m_drawState->OnMouseButtonDown(pos);
      m_drawing = ImageEditor::End();
    }
  }

  void DrawingSheet::OnMouseButtonReleased(const glm::vec2 pos)
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
    {
      ImageEditor::Begin(std::move(m_drawing));
      m_drawState->OnMouseButtonReleased(pos);
      m_drawing = ImageEditor::End();
    }
  }

  void DrawingSheet::OnTextInput(const std::string &inputText)
  {
    if (m_currentDrawCommand != DrawCommand::DO_NOTHING)
    {
      ImageEditor::Begin(std::move(m_drawing));
      m_drawState->OnTextInput(inputText);
      m_drawing = ImageEditor::End();
    }
  }

  void DrawingSheet::OnKeyPressed(KeyCode key)
  {
    if (m_currentDrawCommand != DrawCommand::DO_NOTHING)
    {
      ImageEditor::Begin(std::move(m_drawing));
      m_drawState->OnKeyPressed(key);
      m_drawing = ImageEditor::End();
    }
  }

  void DrawingSheet::OnUpdate()
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
    {
      ImageEditor::Begin(std::move(m_drawing));
      m_drawState->OnUpdate();
      m_drawing = ImageEditor::End();
    }
  }

  void DrawingSheet::OnCancel()
  {
    if(m_currentDrawCommand != DrawCommand::DO_NOTHING)
      m_drawState->OnCancel();
  }

  std::optional<Entity> DrawingSheet::GetHoveredEntity(const glm::vec2 pos)
  {
    // TODO: may want to move this into editor ui, so here only relative coordinates are handled
    const glm::vec2 relPos = GetNormalizedPos(pos);
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

  glm::vec2 DrawingSheet::GetNormalizedPos(const glm::vec2 pos)
  {
    glm::vec2 normalizedPos = pos / m_sheetSize;
    normalizedPos.x = std::clamp(normalizedPos.x, 0.0f, 1.0f);
    normalizedPos.y = std::clamp(normalizedPos.y, 0.0f, 1.0f);
    return normalizedPos;
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
    m_sheet->m_firstPoint = m_sheet->GetNormalizedPos(pos);
    m_sheet->ChangeDrawState(std::make_unique<DrawingTemporaryState>(m_sheet)); 
  }

  void FirstClickRecievedState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    // fallback to initial state, because it was just an accidental single click
    m_sheet->ChangeDrawState(std::make_unique<InitialObjectDrawState>(m_sheet));
  }

  static std::unique_ptr<BaseDrawComponentWrapper> CreateComponentWrapper(DrawCommand command, glm::vec2 firstPoint, glm::vec2 secondPoint, glm::vec2 sheetSize, DrawObjectType objectType)
  {
    switch(command)
    {
      case DrawCommand::DRAW_CIRCLE:
      {
        return std::make_unique<CircleComponentWrapper>(CircleComponentWrapper::CreateCircle(firstPoint, secondPoint, sheetSize.x / sheetSize.y, objectType));
      }
      case DrawCommand::DRAW_RECTANGLE:
      {
        return std::make_unique<RectangleComponentWrapper>(RectangleComponentWrapper::CreateRectangle(firstPoint, secondPoint, objectType));
      }
      case DrawCommand::DRAW_ARROW:
      {
        return std::make_unique<ArrowComponentWrapper>(ArrowComponentWrapper::CreateArrow(firstPoint, secondPoint, objectType));
      }
      case DrawCommand::DRAW_LINE:
      case DrawCommand::DRAW_MULTILINE:
      {
        return std::make_unique<LineComponentWrapper>(LineComponentWrapper::CreateLine(firstPoint, secondPoint, objectType));
      }
      case DrawCommand::DRAW_SKIN_TEMPLATE:
      {
        return std::make_unique<SkinTemplateComponentWrapper>(SkinTemplateComponentWrapper::CreateSkinTemplate(firstPoint, secondPoint, objectType));
      }
      default:
      {
        APP_CORE_ERR("Invalid draw command");
        return nullptr;
      }
    }
  }
  void DrawingTemporaryState::OnMouseButtonDown(const glm::vec2 pos)
  { 
    m_sheet->m_secondPoint = m_sheet->GetNormalizedPos(pos);
    Entity entity;

    auto wrapper = CreateComponentWrapper(m_sheet->m_currentDrawCommand, m_sheet->m_firstPoint, m_sheet->m_secondPoint, m_sheet->m_sheetSize, DrawObjectType::TEMPORARY);
    wrapper->UpdateShapeAttributes();
  }

  void DrawingTemporaryState::OnMouseButtonReleased(const glm::vec2 pos)
  { // TODO REFACTOR: see above 
    m_sheet->m_secondPoint = m_sheet->GetNormalizedPos(pos);
    Entity entity;
    auto wrapper = CreateComponentWrapper(m_sheet->m_currentDrawCommand, m_sheet->m_firstPoint, m_sheet->m_secondPoint, m_sheet->m_sheetSize, DrawObjectType::PERMANENT);
    wrapper->UpdateShapeAttributes();
    
    m_sheet->Annotated(); // needed for weird UI feature

    if(m_sheet->m_currentDrawCommand == DrawCommand::DRAW_LINE) // line is a special case, because we are drawing multiple NOT connected lines
      m_sheet->ChangeDrawState(std::make_unique<InitialObjectDrawState>(m_sheet));
    else if(m_sheet->m_currentDrawCommand == DrawCommand::DRAW_MULTILINE)
      m_sheet->m_firstPoint = m_sheet->m_secondPoint;
    else
    {
      m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
      m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));

    }

  }
  
  void DrawTextInitialState::OnMouseHovered(const glm::vec2 pos)
  { // change the mouse cursor to text editor cursor
    ;
  }
  
  void DrawTextInitialState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = m_sheet->GetNormalizedPos(pos);
    m_sheet->ChangeDrawState(std::make_unique<DrawTextState>(m_sheet)); 
  }

  DrawTextState::~DrawTextState()
  {
    if(m_text != "")
    {
      m_sheet->Annotated(); // needed for weird UI feature
    }
  }

  void DrawTextState::OnTextInput(const std::string &inputText)
  {
    if(m_text == " ")   // first delete the space character, which is there only for showing a blank space when using text input
      m_text = "";
    m_text += inputText;
    TextComponentWrapper tw(TextComponentWrapper::CreateText(m_sheet->m_firstPoint, m_text, s_defaultFontSize, DrawObjectType::TEMPORARY));
    tw.UpdateShapeAttributes();
  }
  
  void DrawTextState::OnKeyPressed(KeyCode key)
  { // exit the command if enter is pressed
    if(m_text != "")
    {
      if(key == Key::MDIK_RETURN)
      {
        TextComponentWrapper tw(TextComponentWrapper::CreateText(m_sheet->m_firstPoint, m_text, s_defaultFontSize, DrawObjectType::PERMANENT));
        tw.UpdateShapeAttributes();
        m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
        m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
      }
      else if(key == Key::MDIK_BACKSPACE)
      {
        m_text.pop_back();
        TextComponentWrapper tw(TextComponentWrapper::CreateText(m_sheet->m_firstPoint, m_text, s_defaultFontSize, DrawObjectType::TEMPORARY));
        tw.UpdateShapeAttributes();
      }
    }
  }
  
  void DrawTextState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    if(m_text != "")
    {
      TextComponentWrapper tw(TextComponentWrapper::CreateText(m_sheet->m_firstPoint, m_text, s_defaultFontSize, DrawObjectType::PERMANENT));
      tw.UpdateShapeAttributes();
    }
    m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
  }

  void DrawTextState::OnUpdate()
  {
    if(m_text != "")
    {
      TextComponentWrapper tw(TextComponentWrapper::CreateText(m_sheet->m_firstPoint, m_text, s_defaultFontSize, DrawObjectType::TEMPORARY));
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
    else if(key == Key::MDIK_UP)
    {
      IncrementLetter();
    }
    else if(key == Key::MDIK_DOWN)
    {
      DecrementLetter();
    }
  }
  void DrawIncrementalLetters::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = m_sheet->GetNormalizedPos(pos);
    TextComponentWrapper tw(TextComponentWrapper::CreateText(m_sheet->m_firstPoint, m_text, s_defaultFontSize, DrawObjectType::PERMANENT));
    tw.UpdateShapeAttributes();
    IncrementLetter();
    m_sheet->Annotated(); // needed for weird UI feature
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

  void DrawIncrementalLetters::DecrementLetter()
  {
    if(m_text == "AA")
    {
      m_sheet->SetDrawCommand(DrawCommand::OBJECT_SELECT); 
      m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
    }
    
    std::string::iterator currentLetter = m_text.end()-1;
    if(m_text.size() > 2)
      APP_CORE_ERR("Something went wrong with the letter increments");
    
    char& c = *currentLetter;
    if((c > 'A') && (c <= 'Z'))
      c--;
    else if(c == 'A')
    {
      if(m_text.size() == 1)
      {
        m_text += 'Z';
        c = 'Z';
      }
      else if(m_text.size() == 2)
      {
        m_text.at(0)--;
        c = 'Z';
      }
    }
  }

  void ObjectSelectInitialState::OnMouseHovered(const glm::vec2 pos)
  {
    m_sheet->m_hoveredEntity = m_sheet->GetHoveredEntity(pos);
  }

  void ObjectSelectInitialState::OnMouseButtonPressed(const glm::vec2 pos)
  {
    m_sheet->m_firstPoint = m_sheet->GetNormalizedPos(pos);
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
    m_sheet->m_secondPoint = m_sheet->GetNormalizedPos(pos);
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
    m_sheet->m_firstPoint = m_sheet->GetNormalizedPos(pos);
    auto view = Entity::View<BoundingContourComponent>();
    // first iterate trough all the selected objects to see if we are clicking on a pickpoint or drag area
    for(auto e : view)
    {
      Entity entity(e);
      if(entity.GetComponent<CommonAttributesComponent>().selected)
      {
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
    }
    // next iterate trough all the not selected objects to see if we are clicking on the bounding box
    m_sheet->ClearSelectionShapes();
    auto hoveredEntity = m_sheet->GetHoveredEntity(pos);
    if(hoveredEntity.has_value())
    {
      hoveredEntity.value().GetComponent<CommonAttributesComponent>().selected = true;
      return;
    }
    // clicking outside of the pickpoints and drage area, clear selection and go back to initial state
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectInitialState>(m_sheet));
  }

  static std::unique_ptr<BaseDrawComponentWrapper> CreateDrawComponentWrapper(Entity entity)
  {
    if (entity.HasComponent<RectangleComponent>())
    {
      return std::make_unique<RectangleComponentWrapper>(entity);
    }
    else if(entity.HasComponent<CircleComponent>())
    {
      return std::make_unique<CircleComponentWrapper>(entity);
    }
    else if(entity.HasComponent<ArrowComponent>())
    {
      return std::make_unique<ArrowComponentWrapper>(entity);
    }
    else if(entity.HasComponent<LineComponent>())
    {
      return std::make_unique<LineComponentWrapper>(entity);
    }
    else if(entity.HasComponent<SkinTemplateComponent>())
    {
      return std::make_unique<SkinTemplateComponentWrapper>(entity);
    }
    else if(entity.HasComponent<TextComponent>())
    { // Do nothing wiht tex component
      return std::make_unique<TextComponentWrapper>(entity);
    }
    else
      APP_CORE_ERR("WTF this component");

  }   

  void PickPointSelectedState::OnMouseButtonDown(const glm::vec2 pos)
  {
    auto currentPoint = m_sheet->GetNormalizedPos(pos);
    auto diff = (currentPoint - m_sheet->m_firstPoint) * glm::vec2(1.0);
    m_sheet->m_firstPoint = m_sheet->GetNormalizedPos(pos);
    auto view = Entity::View<PickPointsComponent>();
    for(auto e : view)
    {
      Entity entity(e);
      auto selectedPointIndex = entity.GetComponent<PickPointsComponent>().selectedPoint;
      if(selectedPointIndex != -1)
      {
        auto wrapper = CreateDrawComponentWrapper(entity);
        wrapper->OnPickPointDrag(diff, selectedPointIndex);
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
    { // TODO: implement a proper visitor pattern here

      auto currentPoint = m_sheet->GetNormalizedPos(pos);
      auto diff = (currentPoint - m_sheet->m_firstPoint) * glm::vec2(1.0);
      m_sheet->m_firstPoint = m_sheet->GetNormalizedPos(pos);
      
      auto wrapper = CreateDrawComponentWrapper(m_sheet->m_draggedEntity.value());
      wrapper->OnObjectDrag(diff);
    }
  }

  void ObjectDraggingState::OnMouseButtonReleased(const glm::vec2 pos)
  {
    m_sheet->ChangeDrawState(std::make_unique<ObjectSelectedState>(m_sheet));
  }

} // namespace medicimage
