#include "editor_ui.h"
#include "log.h"

namespace medicimage
{

  bool EditorUI::s_enterPressed = false;

EditorUI::EditorUI() 
  : Layer("EditorUI")
{}

EditorUI::~EditorUI()
{} 

void EditorUI::OnUpdate()
{
  // TODO: get the frame from the CameraAPI in the future, for now loading a simple texture every frame
  if(!m_inEditMode)
  {
    auto frame = std::move(m_camera.CaptureFrame());
    if (frame)
      m_currentFrame = std::move(frame.value());
    frame.reset();
  }
}

void EditorUI::OnAttach()
{
  m_imageSavers = std::move(std::make_unique<ImageSaverContainer>(std::filesystem::current_path()));

  // loading in the icons
  m_circleIcon  = std::move(std::make_unique<Texture2D>("circle","assets/icons/circle.png"));
  m_startEditingIcon  = std::move(std::make_unique<Texture2D>("start-editing","assets/icons/start-editing.png"));
  m_lineIcon  = std::move(std::make_unique<Texture2D>("line","assets/icons/line.png"));
  m_pencilIcon  = std::move(std::make_unique<Texture2D>("pencil","assets/icons/pencil.png"));
  m_saveIcon  = std::move(std::make_unique<Texture2D>("save","assets/icons/save.png"));
  m_rectangleIcon  = std::move(std::make_unique<Texture2D>("rectangle","assets/icons/rectangle.png"));
  m_arrowIcon  = std::move(std::make_unique<Texture2D>("arrow","assets/icons/arrow.png"));
  m_addTextIcon  = std::move(std::make_unique<Texture2D>("add-text","assets/icons/add-text.png"));

  // initieliaze the frames 
  m_currentFrame = std::make_unique<Texture2D>("checkerboard", "assets/textures/Checkerboard.png"); 
  m_currentEditedFrame = std::make_unique<Texture2D>("initial checkerboard", "assets/textures/Checkerboard.png"); // initialize the edited frame with the current frame and later update only the current frame in OnUpdate

  m_imageEditor.Init(Renderer::GetInstance().GetDevice());
  m_imageEditor.SetTextureForEditing(std::move(std::make_unique<Texture2D>(m_currentFrame->GetTexturePtr(), "currently edited texture"))); // initialize image editor as well

  m_camera.Open();

} 

void EditorUI::OnDetach(){} 

void EditorUI::Draw(PrimitiveAddingType addType, ImVec2 imageSize)
{
  constexpr float circleRadiusSpeed = 1.5;
  switch(m_currentCommand.editType) // TODO: refactor this multiple control indirection on editType
  {
    case EditCommandType::DRAW_CIRCLE:
    {
      ImVec2 centerPoint = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
      ImVec2 secondPoint = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
      ImVec2 diff = { abs(centerPoint.x - secondPoint.x), abs(centerPoint.y - secondPoint.y) };
      float radius = sqrt(diff.x * diff.x + diff.y * diff.y) / 2 * circleRadiusSpeed ;
      m_imageEditor.AddCircle(centerPoint, radius, addType, m_thickness, m_color);
      break;
    }
    case EditCommandType::DRAW_RECTANGLE:
    {
      ImVec2 topLeft = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
      ImVec2 bottomRight = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
      m_imageEditor.AddRectangle(topLeft, bottomRight, addType, m_thickness, m_color);
      break;
    }
    case EditCommandType::DRAW_LINE:
    {
      ImVec2 begin = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
      ImVec2 end = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
      m_imageEditor.AddLine(begin, end, addType, m_thickness, m_color);
      break;
    }
  }
}

void EditorUI::OnImguiRender()
{

  
  // DockSpace
  static bool dockspaceOpen = true;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace Demo", &dockspaceOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar);
  if (ImGui::BeginMenuBar())
  {
    if (ImGui::BeginMenu("Settings"))
    {
      if(ImGui::BeginMenu("Options"))
      {
        static char destFolderBuffer[32] = "workspace"; // todo: add to some std::string
        ImGui::InputText("Destination folder", destFolderBuffer, IM_ARRAYSIZE(destFolderBuffer));
        ImGui::EndMenu();
      }
      APP_CORE_TRACE("Menu bar opened");
      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }
  ImGui::PopStyleVar();

  ImGuiIO& io = ImGui::GetIO();
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
  {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_AutoHideTabBar);
  }
  ImGui::End();

  // uuid input
  ImGui::Begin("Currently captured frame window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);


  char uuidInputBuffer[32];
  memset(uuidInputBuffer, 0, sizeof(uuidInputBuffer));
  ImGui::PushItemWidth(-1);
  if(ImGui::InputText("asd", uuidInputBuffer, IM_ARRAYSIZE(uuidInputBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
  {
    if (uuidInputBuffer[0] != '\0')
    {
      size_t pos;
      try
      {
        int uuid = std::stoi(std::string(uuidInputBuffer), & pos);
        m_imageSavers->SelectImageSaver(uuid);
      }
      catch (std::invalid_argument const& ex)
      {
        APP_CORE_WARN("Please write only numbers for a viable uuid!"); 
      }
      catch (std::out_of_range const& ex)
      {
        APP_CORE_WARN("Please add a number smaller for uuid!"); 
      }
    }
  }
  ImGui::PopItemWidth();

  ImVec2 uvMin = ImVec2(0.0f, 0.0f);                 // Top-left
  ImVec2 uvMax = ImVec2(1.0f, 1.0f);                 // Lower-right
  ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
  ImVec4 borderColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
  ImVec2 canvasSize = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
  
  ImVec2 imageTopLeft = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
  if(m_inEditMode)
    ImGui::Image(m_currentEditedFrame->GetShaderResourceView(), canvasSize, uvMin, uvMax, tintColor, borderColor);
  else
    ImGui::Image(m_currentFrame->GetShaderResourceView(), canvasSize, uvMin, uvMax, tintColor, borderColor);

  ImVec2 imageSize = ImGui::GetItemRectSize();

  if(m_currentCommand.editType == EditCommandType::DO_NOTHING)
  {
    ; // do actually nothing
  }
  else if(m_currentCommand.editType == EditCommandType::SAVE_IMAGE)
  {
    if(m_currentEditedFrame.get() != nullptr)
    {
      if (m_imageSavers->IsEmpty())
        APP_CORE_ERR("UUID not added");
      else
      {
        m_imageSavers->GetSelectedSaver().SaveImage(std::make_shared<Texture2D>(m_currentEditedFrame->GetTexturePtr(), ""), std::make_shared<Texture2D>(m_currentFrame->GetTexturePtr(), ""));
      }
    }
    
    //m_imageEditor.SetTextureForEditing(std::move(std::make_unique<Texture2D>(m_currentFrame->GetTexturePtr(),"currently edited texture")));
    m_currentCommand = {EditCommandType::DO_NOTHING, EditState::DONE};
    
    // we can get out of edit mode only with saving the image
    m_inEditMode = false;
  }
  else if(m_currentCommand.editType == EditCommandType::ADD_TEXT)
  {

  }
  else  // only the drawing commands left
  {
    if(!m_inEditMode)
      m_imageEditor.SetTextureForEditing(std::move(std::make_unique<Texture2D>(m_currentFrame->GetTexturePtr(), "currently edited texture")));

    m_inEditMode = true;
    const bool isImageHovered = ImGui::IsItemHovered(); // Hovered  

    // calculate the mouse position, TODO: in texture coordinates(0,1), and do something with scrolling 
    const ImVec2 mousePosOnImage(io.MousePos.x - imageTopLeft.x, io.MousePos.y - imageTopLeft.y);
    // Add first and second point
    if (isImageHovered && (m_currentCommand.editState == EditState::START_EDIT) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
      m_cursorEditPoints.push_back(mousePosOnImage);
      m_cursorEditPoints.push_back(mousePosOnImage);
      m_currentCommand.editState = EditState::FIRST_CLICK;
    }
    else if(isImageHovered && (m_currentCommand.editState == EditState::FIRST_CLICK))
    {
      if(ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
        m_cursorEditPoints.back() = mousePosOnImage;
        // draw temporary primitive
        m_cursorEditPoints.back() = mousePosOnImage;
        Draw(PrimitiveAddingType::TEMPORARY, imageSize);
      }
      else
      {
        m_currentCommand.editState = EditState::MOUSE_DOWN;

      }
    }
    else if(isImageHovered && (m_currentCommand.editState == EditState::MOUSE_DOWN))
    {
      m_cursorEditPoints.back() = mousePosOnImage;
      Draw(PrimitiveAddingType::PERMANENT, imageSize);

      m_currentCommand = {EditCommandType::DO_NOTHING, EditState::DONE};
      m_cursorEditPoints.clear();
    }
  }
  
  m_currentEditedFrame = m_imageEditor.Draw();
  ImGui::End();


  // Picture thumbnails
  bool openThumbnails = true;
  ImGui::Begin("Thumbnails", &openThumbnails, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar);
  if (!m_imageSavers->IsEmpty())
  {
    for (const auto& image : m_imageSavers->GetSelectedSaver().GetSavedImages())
    {
      ImGui::Text("%s", image->GetName().c_str());
      ImVec2 pos = ImGui::GetCursorScreenPos();
      ImVec2 canvasSize = ImGui::GetContentRegionAvail();
      float aspectRatio = m_currentFrame->GetWidth() / m_currentFrame->GetHeight();
      if(ImGui::ImageButton(image->GetName().c_str(),image->GetShaderResourceView(), ImVec2{ canvasSize.x, canvasSize.x / aspectRatio }, uvMin, uvMax, borderColor, tintColor))
      {
        ;
      }
      ImVec2 buttonSize = ImGui::GetItemRectSize();
      if (ImGui::IsItemHovered())
      {
        ImGui::BeginTooltip();
        float tooltipRegionSize = 64.0f;
        ImVec2 region = {io.MousePos.x - pos.x - tooltipRegionSize * 0.5f, io.MousePos.y - pos.y - tooltipRegionSize * 0.5f};
        float zoom = 2.0f;
        if (region.x < 0.0f) { region.x = 0.0f; }
        else if (region.x > buttonSize.x - tooltipRegionSize) { region.x = buttonSize.x - tooltipRegionSize; }
        if (region.y < 0.0f) { region.y = 0.0f; }
        else if (region.y > buttonSize.y - tooltipRegionSize) { region.y = buttonSize.y - tooltipRegionSize; }
        ImGui::Text("Min: (%.2f, %.2f)", region.x, region.y);
        ImGui::Text("Max: (%.2f, %.2f)", region.x + tooltipRegionSize, region.y + tooltipRegionSize);
        ImVec2 uv0 = ImVec2((region.x) / buttonSize.x, (region.y) / buttonSize.y);
        ImVec2 uv1 = ImVec2((region.x + tooltipRegionSize) / buttonSize.x, (region.y + tooltipRegionSize) / buttonSize.y);
        ImGui::Image(image->GetShaderResourceView(), ImVec2(tooltipRegionSize* zoom, tooltipRegionSize* zoom), uv0, uv1, tintColor, borderColor);
        ImGui::EndTooltip();
      }
    }
  }
  ImGui::End();


  // toolbox 
  bool openTools = true;
  ImGui::Begin("Tools", &openTools , ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar);
  ImVec2 size = ImVec2(80.0f, 80.0f);                         // Size of the image we want to make visible
  ImVec4 iconBg = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);             // white background
  
  if (ImGui::ImageButton("save", m_saveIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_currentCommand = { EditCommandType::SAVE_IMAGE, EditState::START_EDIT};
  }  

  if (ImGui::ImageButton("start-editing", m_startEditingIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_currentCommand = { EditCommandType::START_EDITING_IMAGE, EditState::START_EDIT};
  }
  if (ImGui::ImageButton("addText", m_addTextIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_currentCommand = {EditCommandType::ADD_TEXT, EditState::START_EDIT};
  }
  if (ImGui::ImageButton("pencil", m_pencilIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
  }
  if (ImGui::ImageButton("circle", m_circleIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_currentCommand = {EditCommandType::DRAW_CIRCLE, EditState::START_EDIT};
  }
  if (ImGui::ImageButton("line", m_lineIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_currentCommand = {EditCommandType::DRAW_LINE, EditState::START_EDIT}; 
  }
  if (ImGui::ImageButton("rectangle", m_rectangleIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_currentCommand = {EditCommandType::DRAW_RECTANGLE, EditState::START_EDIT}; 
  }
  if (ImGui::ImageButton("arrow", m_arrowIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
    ;
  ImGui::End();

  // some profiling
  ImGui::Begin("Profiling");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::End();
} 

} // namespace medicimage

