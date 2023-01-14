#include "editor_ui.h"
#include "log.h"

namespace medicimage
{

EditorUI::EditorUI() 
  : Layer("EditorUI")
{}

EditorUI::~EditorUI()
{} 

void EditorUI::OnUpdate()
{
  // TODO: get the frame from the CameraAPI in the future, for now loading a simple texture every frame
  //m_currentFrame = std::make_shared<Texture2D>("checkerboard","assets/textures/Checkerboard.png");
}

void EditorUI::OnAttach()
{
  m_imageEditor.Init(Renderer::GetInstance().GetDevice());

  m_circleIcon  = std::move(std::make_unique<Texture2D>("circle","assets/icons/circle.png"));
  m_editImageIcon  = std::move(std::make_unique<Texture2D>("edit-image","assets/icons/edit-image.png"));
  m_lineIcon  = std::move(std::make_unique<Texture2D>("line","assets/icons/line.png"));
  m_pencilIcon  = std::move(std::make_unique<Texture2D>("pencil","assets/icons/pencil.png"));
  m_screenshotIcon  = std::move(std::make_unique<Texture2D>("screenshot","assets/icons/screenshot.png"));
  m_rectangleIcon  = std::move(std::make_unique<Texture2D>("rectangle","assets/icons/rectangle.png"));
  m_arrowIcon  = std::move(std::make_unique<Texture2D>("arrow","assets/icons/arrow.png"));
  m_addTextIcon  = std::move(std::make_unique<Texture2D>("add-text","assets/icons/add-text.png"));

  m_currentFrame = std::make_unique<Texture2D>("checkerboard", "Checkerboard.png"); // TODO: add it to the OnUpdate
} 

void EditorUI::OnDetach(){} 

void EditorUI::OnImguiRender()
{
  // DockSpace
  static bool dockspaceOpen = true;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace Demo", &dockspaceOpen, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking);
  ImGui::PopStyleVar();
  ImGuiIO& io = ImGui::GetIO();
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
  {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
  }
  ImGui::End();

  // Currently captured frame editing
  ImGui::Begin("Currently captured frame window", nullptr, ImGuiWindowFlags_NoMove);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
  static char buf2[32];
  ImGui::InputText("Example text input", buf2, IM_ARRAYSIZE(buf2));
  ImVec2 uvMin = ImVec2(0.0f, 0.0f);                 // Top-left
  ImVec2 uvMax = ImVec2(1.0f, 1.0f);                 // Lower-right
  ImVec4 tintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
  ImVec4 borderColor = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
  ImVec2 canvasSize = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
  
  ImVec2 imageTopLeft = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
  ImGui::Image(m_currentFrame->GetShaderResourceView(), canvasSize, uvMin, uvMax, tintColor, borderColor);
  ImVec2 imageSize = ImGui::GetItemRectSize();
  //APP_CORE_INFO("Canvas size: {} {}, Image size:{} {}",canvasSize.x, canvasSize.y, imageSize.x, imageSize.y);

  if(m_drawLineRequested || m_drawCircleRequested || m_drawRectangleRequested)
  {
    const bool isImageHovered = ImGui::IsItemHovered(); // Hovered  

    // calculate the mouse position, TODO: in texture coordinates(0,1), and do something with scrolling 
    const ImVec2 mousePosInImage(io.MousePos.x - imageTopLeft.x, io.MousePos.y - imageTopLeft.y);

    // Add first and second point
    static bool adding_line = false;
    if (isImageHovered && !adding_line && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
      m_cursorEditPoints.push_back(mousePosInImage);
      m_cursorEditPoints.push_back(mousePosInImage);
      adding_line = true;
    }
    if (adding_line)
    {
      m_cursorEditPoints.back() = mousePosInImage;
      if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
        m_imageEditor.SetTextureForEditing(m_currentFrame.get());
        ImVec2 topLeft = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
        ImVec2 bottomRight = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
        m_imageEditor.DrawRectangle(topLeft, bottomRight);
        adding_line = false;
      }
    }
  }
  else if(m_addTextRequested)
  {
    ImGui::BeginTooltip();
    static char buf1[32];
    ImGui::InputText("UTF-8 input", buf1, IM_ARRAYSIZE(buf1));
    ImGui::SetItemDefaultFocus();
    ImGui::EndTooltip();
  }
  ImGui::End();


  // Picture thumbnails
  bool openThumbnails = true;
  ImGui::Begin("Thumbnails", &openThumbnails, ImGuiWindowFlags_HorizontalScrollbar);
  for (auto& image : m_capturedImages)
  {
    ImGui::Text("%s",image->GetName().c_str());
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    float aspectRatio = m_currentFrame->GetWidth() / m_currentFrame->GetHeight();
    ImGui::Image(image->GetShaderResourceView(), ImVec2{canvasSize.x, canvasSize.x / aspectRatio}, uvMin, uvMax, tintColor, borderColor);
  }
  ImGui::End();
   
  bool openTools = true;
  ImGui::Begin("Tools", &openTools , ImGuiWindowFlags_HorizontalScrollbar);
  ImVec2 size = ImVec2(64.0f, 64.0f);                         // Size of the image we want to make visible
  ImVec4 iconBg = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);             // white background
  
  if (ImGui::ImageButton("screenshot", m_screenshotIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    APP_CORE_INFO("Current frame captured");
    if(m_currentFrame.get() != nullptr)
      m_capturedImages.push_back(std::make_shared<Texture2D>(m_currentFrame->GetTexturePtr(), GenerateImageName()));
  }  

  if (ImGui::ImageButton("edit-image", m_editImageIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
    APP_CORE_INFO("{} button pressed", "edit-image");
  if (ImGui::ImageButton("addText", m_addTextIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    APP_CORE_INFO("{} button pressed", "addText");
    m_addTextRequested = true;
  }
  if (ImGui::ImageButton("pencil", m_pencilIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    APP_CORE_INFO("{} button pressed", "pencil");
  }
  if (ImGui::ImageButton("circle", m_circleIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_drawCircleRequested = true;
    APP_CORE_INFO("{} button pressed", "circle");
  }
  if (ImGui::ImageButton("line", m_lineIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    APP_CORE_INFO("{} button pressed", "line");
    m_drawLineRequested = true; 
  }
  if (ImGui::ImageButton("rectangle", m_rectangleIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_drawRectangleRequested = true;
    APP_CORE_INFO("{} button pressed", "rectangle");
  }
  if (ImGui::ImageButton("arrow", m_arrowIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
    APP_CORE_INFO("{} button pressed", "arrow");
  ImGui::End();
} 

std::string EditorUI::GenerateImageName()
{
  std::string dst = m_uuid + "_" + std::to_string(m_capturedImageIndex++);
  return dst;
}

} // namespace medicimage

