#include "editor_ui.h"
#include "core/log.h"
#include "widgets/ImFileDialog.h"

#include <assert.h>

namespace medicimage
{

  bool EditorUI::s_enterPressed = false;

EditorUI::EditorUI() 
  : Layer("EditorUI")
{
  m_inputText.fill(0);
}

EditorUI::~EditorUI()
{} 

void EditorUI::OnUpdate()
{
  if(m_editorState == EditorState::SCREENSHOT)
  {
    if(m_timer.Done())
    {
      m_editorState = EditorState::SHOW_CAMERA;
    }
  }
  else if(m_editorState == EditorState::SHOW_CAMERA)
  {
    // load a frame from the camera
    auto frame = std::move(m_camera.CaptureFrame());
    if (frame)
      m_frame = std::move(frame.value());
    frame.reset();
  }
}

void EditorUI::OnAttach()
{
  // first initialize OpenCL in OpenCV for the initial texture loading
  m_imageEditor.Init(Renderer::GetInstance().GetDevice());
  
  // init image saver container 
  m_imageSavers = std::move(std::make_unique<ImageSaverContainer>(m_appConfig.GetAppFolder()));
  for(const auto& patientFolder : m_appConfig.GetSavedPatientFolders())
    m_imageSavers->AddSaver(patientFolder.stem().string());
  
  if(m_imageSavers->HasSelectedSaver())
  {
    auto uuid = m_imageSavers->GetSelectedSaver().GetUuid();
    m_inputText.fill(0);
    assert(m_inputText.size() >= uuid.size());
    std::copy(uuid.begin(), uuid.end(), m_inputText.begin());
  }

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
  m_undoIcon  = std::move(std::make_unique<Texture2D>("add-text","assets/icons/left-arrow.png"));

  // initieliaze the frames 
  m_frame = std::make_unique<Texture2D>("initial checkerboard", "assets/textures/Checkerboard.png"); // initialize the edited frame with the current frame and later update only the current frame in OnUpdate
  m_camera.Open();
  
  // init file dialog
  ifd::FileDialog::Instance().CreateTexture = [&](uint8_t* data, int w, int h, char fmt) -> void*
  {
    // Here, there is a memory leak, because on DirectX there is no similar API for storing textures, like OpenGL 
    // but these textures are just the thumbnails on file dialog.. so for now it is okay, in the future this has 
    // to be fixed with either another filedialog plugin or fixing this one: TODO
    auto* texture = new medicimage::Texture2D("icon", w, h);
    Renderer::GetInstance().GetDeviceContext()->UpdateSubresource(texture->GetTexturePtr(), 0, 0, data, w*4, w*h*4);
    return reinterpret_cast<void*>(texture->GetShaderResourceView());
  }; 
	
  ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
    APP_CORE_INFO("FileDialog.DeleteTexture called");
  };

  // load the bigger font and the smaller font for restoring
  ImGuiIO& io = ImGui::GetIO(); 
  m_smallFont = io.Fonts->AddFontFromFileTTF("assets/fonts/banschrift.ttf", 18.0);
  m_largeFont = io.Fonts->AddFontFromFileTTF("assets/fonts/banschrift.ttf", 48.0);
  
  ImGuiStyle& style = ImGui::GetStyle();
  m_defaultFrameBgColor = style.Colors[ImGuiCol_Button];
} 

void EditorUI::OnDetach(){} 

void EditorUI::OnEvent(Event* event)
{
  EventDispatcher dispatcher(event);
  dispatcher.Dispatch<KeyTextInputEvent>(BIND_EVENT_FN(EditorUI::OnKeyTextInputEvent));
  dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(EditorUI::OnKeyPressedEvent));
}

bool EditorUI::OnKeyTextInputEvent(KeyTextInputEvent* e)
{
  if(m_editorState == EditorState::EDITING) // text input is handled by either ImGui or the drawing sheet
  {
    m_drawingSheet.OnTextInput(e->GetInputTextText());
  }
  return true;
}

bool EditorUI::OnKeyPressedEvent(KeyPressedEvent* e)
{
  // TODO: implement this
  //if ((m_activeCommand.commandState == DrawCommandState::FIRST_CLICK) && m_activeCommand.commandType == DrawCommand::ADD_TEXT)
  //{
  //  if(e->GetKeyCode() == Key::MDIK_RETURN)
  //  {
  //    m_activeCommand.commandState = DrawCommandState::FINISH;
  //    return true;
  //  }
  //  else
  //    return false;
  //}
  //else
  //  return false;

  return true;
}

void EditorUI::ShowImageWindow()
{
  // Main window containing the stream and uuid input
  ImGui::Begin("Currently captured frame window", nullptr, ImGuiWindowFlags_NoTitleBar| ImGuiWindowFlags_NoMove);
  
  // Camera stream
  ImGuiIO& io = ImGui::GetIO();
  ImGuiStyle& style = ImGui::GetStyle();
  constexpr ImVec2 uvMin = ImVec2(0.0f, 0.0f);                 // Top-left
  constexpr ImVec2 uvMax = ImVec2(1.0f, 1.0f);                 // Lower-right
  constexpr ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
  constexpr ImVec4 borderColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f); // 50% opaque white
  ImVec2 canvasSize = ImGui::GetContentRegionAvail();   // Resize canvas to what's available

  if(m_editorState == EditorState::EDITING)
  {
    m_frame = m_drawingSheet.Draw(); 
  
    ImGui::Image(m_frame->GetShaderResourceView(), canvasSize, uvMin, uvMax, tintColor, borderColor);
    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    auto viewportOffset = ImGui::GetWindowPos();
    glm::vec2 drawingSheetSize = {viewportMaxRegion.x - viewportMinRegion.x, viewportMaxRegion.y - viewportMinRegion.y};
    m_drawingSheet.SetDrawingSheetSize(drawingSheetSize);
    
    // Mouse button actions are prioritized over the hovering 
    if(ImGui::IsItemHovered())
    {
      auto mousePos = ImGui::GetMousePos();
      const ImVec2 mousePosOnImage(mousePos.x - viewportOffset.x - viewportMinRegion.x, mousePos.y - viewportOffset.y - viewportMinRegion.y);
     
      if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        m_drawingSheet.OnMouseButtonPressed({mousePosOnImage.x, mousePosOnImage.y});
      else if(ImGui::IsMouseDown(ImGuiMouseButton_Left))
        m_drawingSheet.OnMouseButtonDown({mousePosOnImage.x, mousePosOnImage.y});
      else if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        m_drawingSheet.OnMouseButtonReleased({mousePosOnImage.x, mousePosOnImage.y});
      else
        m_drawingSheet.OnMouseHovered({mousePosOnImage.x, mousePosOnImage.y});
    }
  }
  else
  { // just show the frame from the camera
    ImGui::Image(m_frame->GetShaderResourceView(), canvasSize, uvMin, uvMax, tintColor, borderColor);
  }
  ImGui::End();
  
  // uuid input
  bool openUuidInput = true;
  ImGui::Begin("UuidInput", &openUuidInput , ImGuiWindowFlags_NoTitleBar);
  static bool uuidTextInputTriggered = false;
  ImGui::PushFont(m_largeFont); 
  ImGui::PushItemWidth(-260); // TODO: do not hardcode it
  float sz = ImGui::GetTextLineHeight();
  if(ImGui::InputText("##label", m_inputText.data(), m_inputText.size(), ImGuiInputTextFlags_EnterReturnsTrue))
  {
    uuidTextInputTriggered = true;
  }

  ImGui::PopItemWidth();
  ImGui::SameLine();
  if(ImGui::Button("Submit"))
  {
    uuidTextInputTriggered = true;
  }

  ImGui::SameLine();
  if(ImGui::Button("Clear"))
  {
    m_inputText.fill(0);  // no clearing it because we dont use this as an iterated array, but C-style array in ImGui
  }
  ImGui::PopFont();

  if(uuidTextInputTriggered)
  {
    std::string inputText = std::string(m_inputText.data());
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
    if (m_inputText[0] != '\0' && checkInput(inputText))
    {
      size_t pos;
      try
      {
        m_imageSavers->SelectImageSaver(inputText);
        m_appConfig.PushPatientFolder(m_imageSavers->GetSelectedSaver().GetPatientFolder());
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

}

void EditorUI::ShowToolbox()
{
  bool openTools = true;
  ImGui::Begin("Tools", &openTools , ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar);
  constexpr ImVec2 bigIconSize = ImVec2(100.0f, 100.0f);                         
  constexpr ImVec2 smallIconSize = ImVec2(40.0f, 40.0f);                         
  ImVec4 iconBg = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

  constexpr ImVec2 uvMin = ImVec2(0.0f, 0.0f);                 // Top-left
  constexpr ImVec2 uvMax = ImVec2(1.0f, 1.0f);                 // Lower-right
  ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
  if (ImGui::ImageButton("screenshot", m_screenshotIcon->GetShaderResourceView(), bigIconSize, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::SHOW_CAMERA)
    {
      if(m_frame.get() != nullptr)
      {
        if (m_imageSavers->HasSelectedSaver()) //TODO: should select it with an optional<ImageSaver> return type
        { // create the ImageDocument here, because the screenshot is made here
          m_activeDocument = ImageDocument(std::make_unique<Texture2D>(m_frame->GetTexturePtr(), m_frame->GetName()));
          m_imageSavers->GetSelectedSaver().SaveImage(m_activeDocument, false);
        }
        else
        {
          APP_CORE_ERR("Please input valid UUID for saving the current image!");
        }
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

  if (ImGui::ImageButton("save", m_saveIcon->GetShaderResourceView(), smallIconSize, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
    {
      if(m_activeDocument.texture.get() != nullptr)
      {
        if (m_imageSavers->HasSelectedSaver()) //TODO: should select it with an optional<ImageSaver> return type
        { // update the texture in the document with the frame, because from this point it is not needed to store the original image
          m_activeDocument.texture = std::make_unique<Texture2D>(*m_frame.get());
          m_imageSavers->GetSelectedSaver().SaveImage(m_activeDocument, true);

        }
        else
          APP_CORE_ERR("Please input valid UUID for saving the current image!");
      }
      // we can get out of edit mode only with saving the image
      m_editorState = EditorState::SHOW_CAMERA;
      m_drawingSheet.SetDrawCommand(DrawCommand::DO_NOTHING);
      ImGui::BeginDisabled();
    }
  } 
  ImGui::SameLine();

  if (ImGui::ImageButton("delete", m_deleteIcon->GetShaderResourceView(), smallIconSize, uvMin, uvMax, iconBg, tintColor))
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
      std::string imageName = m_activeDocument.documentId; 
      ImGui::Text("Are you sure you want to delete image %s?\n deletion cannot be undone!", imageName.c_str());
      ImGui::Separator();
      bool beginDisabling = false;
      if (ImGui::Button("OK", ImVec2(120, 0))) 
      { 
        ImGui::CloseCurrentPopup();
        if(m_imageSavers->HasSelectedSaver())
          m_imageSavers->GetSelectedSaver().DeleteImage(imageName);
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
      m_drawingSheet.SetDrawCommand(DrawCommand::DO_NOTHING);
    }
  } 

  if (ImGui::ImageButton("undo", m_undoIcon->GetShaderResourceView(), smallIconSize, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
    {
      ImGui::OpenPopup("undo");
      ImVec2 center = ImGui::GetMainViewport()->GetCenter();
      ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    }
  }
  ImGui::SameLine();
  
  if(m_editorState == EditorState::EDITING)
  {
    if (ImGui::BeginPopupModal("undo", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
      std::string imageName = m_activeDocument.documentId; 
      ImGui::Text("Are you sure you want to clear the annotations?");
      ImGui::Separator();
      bool beginDisabling = false;
      if (ImGui::Button("OK", ImVec2(120, 0))) 
      { 
        ImGui::CloseCurrentPopup();
        m_editorState = EditorState::SHOW_CAMERA;
        beginDisabling = true;
      }
      ImGui::SetItemDefaultFocus();
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(120, 0))) 
      { 
        ImGui::CloseCurrentPopup(); 
      }
      ImGui::EndPopup();
      if (beginDisabling)
        ImGui::BeginDisabled();
      m_drawingSheet.SetDrawCommand(DrawCommand::DO_NOTHING);
    }
  } 
  
  // setting green border for the selected button
  ImGuiStyle& style = ImGui::GetStyle();
  
  //style.Colors[ImGuiCol_Button] = m_activeCommand.commandType == DrawCommand::ADD_TEXT ? m_toolUsedBgColor : m_defaultFrameBgColor; 
  style.Colors[ImGuiCol_Button] =  m_defaultFrameBgColor; 
  if (ImGui::ImageButton("addText", m_addTextIcon->GetShaderResourceView(), smallIconSize, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
      m_drawingSheet.SetDrawCommand(DrawCommand::DRAW_TEXT);
  }
  
  //style.Colors[ImGuiCol_Button] = m_activeCommand.commandType == DrawCommand::DRAW_CIRCLE ? m_toolUsedBgColor : m_defaultFrameBgColor; 
  style.Colors[ImGuiCol_Button] = m_defaultFrameBgColor; 
  if (ImGui::ImageButton("circle", m_circleIcon->GetShaderResourceView(), smallIconSize, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
      m_drawingSheet.SetDrawCommand(DrawCommand::DRAW_CIRCLE);
  }
  ImGui::SameLine();
  
  //style.Colors[ImGuiCol_Button] = m_activeCommand.commandType == DrawCommand::DRAW_LINE ? m_toolUsedBgColor : m_defaultFrameBgColor; 
  style.Colors[ImGuiCol_Button] = m_defaultFrameBgColor; 
  if (ImGui::ImageButton("line", m_lineIcon->GetShaderResourceView(), smallIconSize, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
      m_drawingSheet.SetDrawCommand(DrawCommand::DRAW_LINE);
  }
  
  //style.Colors[ImGuiCol_Button] = m_activeCommand.commandType == DrawCommand::DRAW_RECTANGLE ? m_toolUsedBgColor : m_defaultFrameBgColor; 
  style.Colors[ImGuiCol_Button] = m_defaultFrameBgColor; 
  if (ImGui::ImageButton("rectangle", m_rectangleIcon->GetShaderResourceView(), smallIconSize, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
      m_drawingSheet.SetDrawCommand(DrawCommand::DRAW_RECTANGLE);
  }
  ImGui::SameLine();

  //style.Colors[ImGuiCol_Button] = m_activeCommand.commandType == DrawCommand::DRAW_ARROW ? m_toolUsedBgColor : m_defaultFrameBgColor; 
  style.Colors[ImGuiCol_Button] = m_defaultFrameBgColor; 
  if (ImGui::ImageButton("arrow", m_arrowIcon->GetShaderResourceView(), smallIconSize, uvMin, uvMax, iconBg, tintColor))
  {
    if(m_editorState == EditorState::EDITING)
      m_drawingSheet.SetDrawCommand(DrawCommand::DRAW_ARROW);
  }

  if(m_editorState != EditorState::EDITING)
  {
    ImGui::EndDisabled();
  }
  // restore the default color
  style.Colors[ImGuiCol_Button] = m_defaultFrameBgColor; 
  
  //listbox for selecting saved uuids
  if (ImGui::BeginListBox("##listbox", ImVec2{ -FLT_MIN, 120 }))
  {
    for(const auto& saverMap : m_imageSavers->GetImageSavers())
    {
      auto& saver = saverMap.second;
      bool selected = saver.GetUuid() == m_imageSavers->GetSelectedUuid();
      auto uuid = saver.GetUuid();
      if(ImGui::Selectable(uuid.c_str(), &selected))
      {
        assert(m_inputText.size() >= uuid.size());
        m_inputText.fill(0);
        std::copy(uuid.begin(), uuid.end(), m_inputText.begin());
        m_imageSavers->SelectImageSaver(uuid);
        m_imageSavers->GetSelectedSaver().LoadPatientsFolder();
      }
    }
    ImGui::EndListBox();
  }
  ImGui::End();

}

void EditorUI::ShowThumbnails()
{
  // Picture thumbnails
  ImGuiIO& io = ImGui::GetIO();
  ImGuiStyle& style = ImGui::GetStyle();
  static int numOfPrevThumbs = 0;
  bool openThumbnails = true;
  ImGui::Begin("Thumbnails", &openThumbnails, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar);
  if (m_imageSavers->HasSelectedSaver())
  {
    int numOfCurrentThumbs = 0;
    ImVec4 backgroundColor = ImVec4(1.0f, 1.0f, 1.0f, 0.0f); // 50% opaque white
    for (const auto& image : m_imageSavers->GetSelectedSaver().GetSavedImages())
    {
      numOfCurrentThumbs++;
      constexpr ImVec2 uvMin = ImVec2(0.0f, 0.0f);                 // Top-left
      constexpr ImVec2 uvMax = ImVec2(1.0f, 1.0f);                 // Lower-right
      constexpr ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
      ImGui::Text("%s", image.texture->GetName().c_str());
      ImVec2 pos = ImGui::GetCursorScreenPos();
      ImVec2 canvasSize = ImGui::GetContentRegionAvail();
      float aspectRatio = static_cast<float>(m_frame->GetWidth()) / static_cast<float>(m_frame->GetHeight());
      if(ImGui::ImageButton(image.texture->GetName().c_str(), image.texture->GetShaderResourceView(), ImVec2{ canvasSize.x, canvasSize.x / aspectRatio }, uvMin, uvMax, backgroundColor, tintColor))
      {
        // we can go into edit mode if we select an image from the thumbnails
        m_editorState = EditorState::EDITING;
        m_drawingSheet.SetDocument(std::move(std::make_unique<ImageDocument>(image)), {image.texture->GetWidth(), image.texture->GetHeight()});  // BIG TODO: update store the image size somewhere 
        m_drawingSheet.ChangeDrawState(std::make_unique<ObjectSelectInitialState>(&m_drawingSheet));
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
        ImGui::Image(image.texture->GetShaderResourceView(), ImVec2(tooltipRegionSize* zoom, tooltipRegionSize* zoom), uv0, uv1, tintColor, backgroundColor);
        ImGui::EndTooltip();
      }
    }
    if(numOfPrevThumbs < numOfCurrentThumbs)
    {
      numOfPrevThumbs = numOfCurrentThumbs;
      ImGui::SetScrollHereY(1.0f);
    }
  }
  ImGui::End();
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
        if(ImGui::Button("Image folder"))
        {
          ifd::FileDialog::Instance().Open("DirectoryOpenDialog", "Open a directory", "");
        }
        ImGui::EndMenu();
      }
      APP_CORE_TRACE("Menu bar opened");
      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }
  ImGui::End();

	if (ifd::FileDialog::Instance().IsDone("DirectoryOpenDialog")) {
		if (ifd::FileDialog::Instance().HasResult()) {
			auto& result = ifd::FileDialog::Instance().GetResult();

      // when setting a new application folder, the thumbnails and the image savers should be reset(the data will remain in the data folder)
		  m_appConfig.UpdateAppFolder(result);
      m_imageSavers = std::move(std::make_unique<ImageSaverContainer>(m_appConfig.GetAppFolder()));
      APP_CORE_INFO("Directory:{} selected", result.string());
		}
		ifd::FileDialog::Instance().Close();
	}

  ShowImageWindow();
  ShowToolbox();
  ShowThumbnails(); 

  // some profiling
  ImGui::Begin("Profiling");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
  ImGui::End();
} 

} // namespace medicimage

