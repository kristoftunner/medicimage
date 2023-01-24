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
  if(m_editorState == EditorState::SCREENSHOT)
  {
    if(m_timer.Done())
    {
      APP_CORE_INFO("Screenshot added!");
      m_editorState = EditorState::SHOW_CAMERA;
    }
  }
  else if(m_editorState == EditorState::SHOW_CAMERA)
  {
    // load a frame from the camera
    auto frame = std::move(m_camera.CaptureFrame());
    if (frame)
      m_activeOriginalImage = std::move(frame.value());
    frame.reset();
  }
}

void EditorUI::OnAttach()
{
  m_imageSavers = std::move(std::make_unique<ImageSaverContainer>(std::filesystem::current_path()));

  // loading in the icons
  m_circleIcon  = std::move(std::make_unique<Texture2D>("circle","assets/icons/circle.png"));
  m_screenshotIcon  = std::move(std::make_unique<Texture2D>("screenshot","assets/icons/screenshot.png"));
  m_lineIcon  = std::move(std::make_unique<Texture2D>("line","assets/icons/line.png"));
  m_pencilIcon  = std::move(std::make_unique<Texture2D>("pencil","assets/icons/pencil.png"));
  m_saveIcon  = std::move(std::make_unique<Texture2D>("save","assets/icons/save.png"));
  m_deleteIcon  = std::move(std::make_unique<Texture2D>("delete","assets/icons/delete.png"));
  m_rectangleIcon  = std::move(std::make_unique<Texture2D>("rectangle","assets/icons/rectangle.png"));
  m_arrowIcon  = std::move(std::make_unique<Texture2D>("arrow","assets/icons/arrow.png"));
  m_addTextIcon  = std::move(std::make_unique<Texture2D>("add-text","assets/icons/add-text.png"));

  // initieliaze the frames 
  m_activeOriginalImage = std::make_unique<Texture2D>("checkerboard", "assets/textures/Checkerboard.png"); 
  m_activeEditedImage = std::make_unique<Texture2D>("initial checkerboard", "assets/textures/Checkerboard.png"); // initialize the edited frame with the current frame and later update only the current frame in OnUpdate

  m_imageEditor.Init(Renderer::GetInstance().GetDevice());
  m_imageEditor.SetTextureForEditing(std::move(std::make_unique<Texture2D>(m_activeOriginalImage->GetTexturePtr(), "currently edited texture"))); // initialize image editor as well

  m_camera.Open();

} 

void EditorUI::OnDetach(){} 

void EditorUI::Draw(PrimitiveAddingType addType, ImVec2 imageSize)
{
  constexpr float circleRadiusSpeed = 1.5;
  switch(m_activeCommand.commandType) // TODO: refactor this multiple control indirection on editType
  {
    case DrawCommandType::DRAW_CIRCLE:
    {
      ImVec2 centerPoint = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
      ImVec2 secondPoint = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
      ImVec2 diff = { abs(centerPoint.x - secondPoint.x), abs(centerPoint.y - secondPoint.y) };
      float radius = sqrt(diff.x * diff.x + diff.y * diff.y) / 2 * circleRadiusSpeed ;
      m_imageEditor.AddCircle(centerPoint, radius, addType, m_thickness, m_color);
      break;
    }
    case DrawCommandType::DRAW_RECTANGLE:
    {
      ImVec2 topLeft = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
      ImVec2 bottomRight = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
      m_imageEditor.AddRectangle(topLeft, bottomRight, addType, m_thickness, m_color);
      break;
    }
    case DrawCommandType::DRAW_LINE:
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
  ImGuiIO& io = ImGui::GetIO();
  ImGuiStyle& style = ImGui::GetStyle();
  static bool dockspaceOpen = true;
  static bool opt_fullscreen = true;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
  // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
  // because it would be confusing to have two docking targets within each others.
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGui::Begin("DockSpace Demo", nullptr, window_flags);
  ImGui::PopStyleVar(2);
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
  {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_AutoHideTabBar);
  }
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
  ImGui::End();

  // Main window containing the stream and uuid input
  ImGui::Begin("Currently captured frame window", nullptr, ImGuiWindowFlags_NoTitleBar| ImGuiWindowFlags_NoMove);
  
  // Camera stream
  ImVec2 uvMin = ImVec2(0.0f, 0.0f);                 // Top-left
  ImVec2 uvMax = ImVec2(1.0f, 1.0f);                 // Lower-right
  ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
  ImVec4 backgroundColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
  ImVec2 canvasSize = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
  canvasSize.y = canvasSize.y - 35; // TODO: not hardcode these values

  if(m_editorState == EditorState::EDITING)
  {
    m_activeEditedImage = m_imageEditor.Draw(); // TODO: maybe move this to OnUpdate()
    ImVec2 imageTopLeft = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
    ImGui::Image(m_activeEditedImage->GetShaderResourceView(), canvasSize, uvMin, uvMax, tintColor, backgroundColor);
    ImVec2 imageSize = ImGui::GetItemRectSize();
    const bool isImageHovered = ImGui::IsItemHovered(); // Hovered  

    // calculate the mouse position, TODO: in texture coordinates(0,1), and do something with scrolling 
    const ImVec2 mousePosOnImage(io.MousePos.x - imageTopLeft.x, io.MousePos.y - imageTopLeft.y);
    // Add first and second point
    if (isImageHovered && (m_activeCommand.commandState == DrawCommandState::INITIAL) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
      m_cursorEditPoints.push_back(mousePosOnImage);
      m_cursorEditPoints.push_back(mousePosOnImage);
      m_activeCommand.commandState = DrawCommandState::FIRST_CLICK;
    }
    else if(isImageHovered && (m_activeCommand.commandState == DrawCommandState::FIRST_CLICK))
    {
      if(ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
        m_cursorEditPoints.back() = mousePosOnImage;
        // draw temporary primitive
        m_cursorEditPoints.back() = mousePosOnImage;
        Draw(PrimitiveAddingType::TEMPORARY, imageSize);
      }
      else
        m_activeCommand.commandState = DrawCommandState::MOUSE_DOWN;
    }
    else if(isImageHovered && (m_activeCommand.commandState == DrawCommandState::MOUSE_DOWN))
    {
      m_cursorEditPoints.back() = mousePosOnImage;
      Draw(PrimitiveAddingType::PERMANENT, imageSize);

      // go back the initial state so that we can draw another shape of the chosen type
      m_activeCommand = {m_activeCommand.commandType, DrawCommandState::INITIAL};
      m_cursorEditPoints.clear();
    }
      
  }
  else if((m_editorState == EditorState::SHOW_CAMERA) || (m_editorState == EditorState::SCREENSHOT))
  {
    ImGui::Image(m_activeOriginalImage->GetShaderResourceView(), canvasSize, uvMin, uvMax, tintColor, backgroundColor);
  }
  
  // uuid input
  static char uuidInputBuffer[32] = "";
  ImGui::PushItemWidth(-120);
  static bool uuidTextInputTriggered = false;
  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f)); 
  if(ImGui::InputText("##label", uuidInputBuffer, IM_ARRAYSIZE(uuidInputBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
  {
    uuidTextInputTriggered = true;
  }
  ImGui::PopStyleVar();
  ImGui::PopItemWidth();
  ImGui::SameLine();
  if(ImGui::Button("Submit"))
  {
    uuidTextInputTriggered = true;
  }

  ImGui::SameLine();
  if(ImGui::Button("Clear"))
  {
    memset(uuidInputBuffer, 0, sizeof(uuidInputBuffer));
  }

  if(uuidTextInputTriggered)
  {
    std::string inputText = std::string(uuidInputBuffer);
    uuidTextInputTriggered = false;
    auto checkInput = [&](const std::string& inputString){
      for(auto c : inputString)
      {
        if(std::isalnum(c) == 0)
        {
          if (c == '.' || c == ',' || c == '_' || c == '-')
            ;
          else
            return false;
        }
      }
      return true;};
    if (uuidInputBuffer[0] != '\0' && checkInput(inputText))
    {
      size_t pos;
      try
      {
        m_imageSavers->SelectImageSaver(inputText);
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
    else
      APP_CORE_ERR("Add only letters numbers and \\.\\,\\-\\_ characters for uuid");
  }
  ImGui::End();

  // toolbox 
  bool openTools = true;
  ImGui::Begin("Tools", &openTools , ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar);
  ImVec2 size = ImVec2(80.0f, 80.0f);                         // Size of the image we want to make visible
  ImVec4 iconBg = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

  if (ImGui::ImageButton("screenshot", m_screenshotIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::SHOW_CAMERA)
    {
      if(m_activeOriginalImage.get() != nullptr)
      {
        if (m_imageSavers->IsEmpty()) //TODO: should select it with an optional<ImageSaver> return type
          APP_CORE_ERR("Please input valid UUID for saving the current image!");
        else
          m_imageSavers->GetSelectedSaver().SaveImage(std::make_shared<Texture2D>(m_activeOriginalImage->GetTexturePtr(), m_activeOriginalImage->GetName()), ImageSaver::ImageType::ORIGINAL);
      }
      m_editorState = EditorState::SCREENSHOT;
      m_timer.Start(500);
    }
  }

  if(m_editorState != EditorState::EDITING)
  {
    ImGui::BeginDisabled();
    iconBg = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
  }

  if (ImGui::ImageButton("save", m_saveIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
    {
      if(m_activeEditedImage.get() != nullptr)
      {
        if (m_imageSavers->IsEmpty()) //TODO: should select it with an optional<ImageSaver> return type
          APP_CORE_ERR("Please input valid UUID for saving the current image!");
        else
          m_imageSavers->GetSelectedSaver().SaveImage(std::make_shared<Texture2D>(m_activeEditedImage->GetTexturePtr(), m_activeEditedImage->GetName()), ImageSaver::ImageType::ANNOTATED);
      }
      // we can get out of edit mode only with saving the image
      m_editorState = EditorState::SHOW_CAMERA;
      ImGui::BeginDisabled();
    }
  }  
  if (ImGui::ImageButton("delete", m_deleteIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
    {
      ImGui::OpenPopup("delete");
      ImVec2 center = ImGui::GetMainViewport()->GetCenter();
      ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }
  }

  if(m_editorState == EditorState::EDITING)
  {
    if (ImGui::BeginPopupModal("delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
      std::string imageName = m_activeEditedImage->GetName(); 
      ImGui::Text("Are you sure you want to delete image{}?\n deletion cannot be undone!", imageName);
      ImGui::Separator();
      bool beginDisabling = false;
      if (ImGui::Button("OK", ImVec2(120, 0))) 
      { 
        ImGui::CloseCurrentPopup(); 
        m_imageSavers->GetSelectedSaver().DeleteImage(m_activeEditedImage->GetName());
        APP_CORE_INFO("Image with ID:{} has been deleted", imageName);
        m_editorState = EditorState::SHOW_CAMERA;
        beginDisabling = true;
      }
      ImGui::SetItemDefaultFocus();
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(120, 0))) 
      { 
        ImGui::CloseCurrentPopup(); 
        m_editorState = EditorState::SHOW_CAMERA;
        beginDisabling = true;
      }
      ImGui::EndPopup();
      if (beginDisabling)
        ImGui::BeginDisabled();
    }
  } 

  if (ImGui::ImageButton("addText", m_addTextIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
      m_activeCommand = {DrawCommandType::ADD_TEXT, DrawCommandState::INITIAL};
  }
  if (ImGui::ImageButton("pencil", m_pencilIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
  }
  if (ImGui::ImageButton("circle", m_circleIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
      m_activeCommand = {DrawCommandType::DRAW_CIRCLE, DrawCommandState::INITIAL};
  }
  if (ImGui::ImageButton("line", m_lineIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
      m_activeCommand = {DrawCommandType::DRAW_LINE, DrawCommandState::INITIAL};
  }
  if (ImGui::ImageButton("rectangle", m_rectangleIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
      m_activeCommand = {DrawCommandType::DRAW_RECTANGLE, DrawCommandState::INITIAL};
  }
  if (ImGui::ImageButton("arrow", m_arrowIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
      m_activeCommand = {DrawCommandType::DRAW_ARROW, DrawCommandState::INITIAL};
  }
  
  if(m_editorState != EditorState::EDITING)
  {
    ImGui::EndDisabled();
  }
  
  // Picture thumbnails
  bool openThumbnails = true;
  ImGui::Begin("Thumbnails", &openThumbnails, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar);
  if (!m_imageSavers->IsEmpty())
  {
    for (const auto& imagePair : m_imageSavers->GetSelectedSaver().GetSavedImagePairs())
    {
      ImGui::Text("%s", imagePair.name.c_str());
      ImVec2 pos = ImGui::GetCursorScreenPos();
      ImVec2 canvasSize = ImGui::GetContentRegionAvail();
      float aspectRatio = m_activeOriginalImage->GetWidth() / m_activeOriginalImage->GetHeight();
      auto buttonImage = imagePair.annotatedImage.has_value() ? imagePair.annotatedImage.value() : imagePair.originalImage.value(); 
      if(ImGui::ImageButton(imagePair.name.c_str(), buttonImage->GetShaderResourceView(), ImVec2{ canvasSize.x, canvasSize.x / aspectRatio }, uvMin, uvMax, backgroundColor, tintColor))
      {
        // we can go into edit mode if we select an image from the thumbnails
        m_editorState = EditorState::EDITING;
        m_imageEditor.SetTextureForEditing(std::make_unique<Texture2D>(buttonImage->GetTexturePtr(), buttonImage->GetName()));
      }

      // little tooltip showing a zoomed version of the thumbnail image
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
        ImGui::Image(buttonImage->GetShaderResourceView(), ImVec2(tooltipRegionSize* zoom, tooltipRegionSize* zoom), uv0, uv1, tintColor, backgroundColor);
        ImGui::EndTooltip();
      }
    }
  }
  ImGui::End();

  ImGui::End();

  // some profiling
  ImGui::Begin("Profiling");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::End();
} 

} // namespace medicimage

