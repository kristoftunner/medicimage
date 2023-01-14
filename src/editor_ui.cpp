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
  m_startEditingIcon  = std::move(std::make_unique<Texture2D>("start-editing","assets/icons/start-editing.png"));
  m_lineIcon  = std::move(std::make_unique<Texture2D>("line","assets/icons/line.png"));
  m_pencilIcon  = std::move(std::make_unique<Texture2D>("pencil","assets/icons/pencil.png"));
  m_saveIcon  = std::move(std::make_unique<Texture2D>("save","assets/icons/save.png"));
  m_rectangleIcon  = std::move(std::make_unique<Texture2D>("rectangle","assets/icons/rectangle.png"));
  m_arrowIcon  = std::move(std::make_unique<Texture2D>("arrow","assets/icons/arrow.png"));
  m_addTextIcon  = std::move(std::make_unique<Texture2D>("add-text","assets/icons/add-text.png"));

  m_currentFrame = std::make_unique<Texture2D>("checkerboard", "Checkerboard.png"); 
  m_currentEditedFrame = std::make_unique<Texture2D>("initial checkerboard", "Checkerboard.png"); // initialize the edited frame with the current frame and later update only the current frame in OnUpdate

  m_imageEditor.SetTextureForEditing(std::move(std::make_unique<Texture2D>(m_currentFrame->GetTexturePtr(), "currently edited texture"))); // initialize image editor as well
} 

void EditorUI::OnDetach(){} 

void EditorUI::DispatchDrawingCommand()
{

}

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
  ImGui::Image(m_currentEditedFrame->GetShaderResourceView(), canvasSize, uvMin, uvMax, tintColor, borderColor);
  ImVec2 imageSize = ImGui::GetItemRectSize();

  // image editing logic
  if(m_currentCommand.editType == EditCommandType::DO_NOTHING)
  {
    ; // do actually nothing
  }
  else if(m_currentCommand.editType == EditCommandType::SAVE_IMAGE)
  {
    if(m_currentEditedFrame.get() != nullptr)
      m_capturedImages.push_back(std::move(std::make_unique<Texture2D>(m_currentEditedFrame->GetTexturePtr(), GenerateImageName())));
    
    m_imageEditor.SetTextureForEditing(std::move(std::make_unique<Texture2D>(m_currentFrame->GetTexturePtr(),"currently edited texture")));
    m_currentCommand = {EditCommandType::DO_NOTHING, false};
  }
  else if(m_currentCommand.editType == EditCommandType::ADD_TEXT)
  {

  }
  else  // only the drawing commands left
  {
    m_inEditMode = true;
    const bool isImageHovered = ImGui::IsItemHovered(); // Hovered  

    // calculate the mouse position, TODO: in texture coordinates(0,1), and do something with scrolling 
    const ImVec2 mousePosOnImage(io.MousePos.x - imageTopLeft.x, io.MousePos.y - imageTopLeft.y);

    // Add first and second point
    if (isImageHovered && !m_currentCommand.editing && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
      m_cursorEditPoints.push_back(mousePosOnImage);
      m_cursorEditPoints.push_back(mousePosOnImage);
      m_currentCommand.editing = true;
      switch(m_currentCommand.editType) // TODO: refactor this multiple control indirection on editType
      {
        case EditCommandType::DRAW_CIRCLE:
        {
          ImVec2 centerPoint = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
          ImVec2 secondPoint = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
          ImVec2 diff = { abs(centerPoint.x - secondPoint.x), abs(centerPoint.y - secondPoint.y) };
          float radius = sqrt(diff.x * diff.x + diff.y * diff.y) / 2;
          m_imageEditor.AddCircle(centerPoint, radius);
          break;
        }
        case EditCommandType::DRAW_RECTANGLE:
        {
          ImVec2 topLeft = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
          ImVec2 bottomRight = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
          m_imageEditor.AddRectangle(topLeft, bottomRight);
          break;
        }
        case EditCommandType::DRAW_LINE:
        {
          ImVec2 begin = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
          ImVec2 end = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
          m_imageEditor.AddLine(begin, end);
          break;
        }
      }
    }
    if (m_currentCommand.editing)
    {
      m_cursorEditPoints.back() = mousePosOnImage;
      if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
        switch(m_currentCommand.editType) // TODO: refactor this multiple control indirection on editType
        {
          case EditCommandType::DRAW_CIRCLE:
          {
            ImVec2 centerPoint = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
            ImVec2 secondPoint = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
            ImVec2 diff = { abs(centerPoint.x - secondPoint.x), abs(centerPoint.y - secondPoint.y) };
            float radius = sqrt(diff.x * diff.x + diff.y * diff.y) / 2;
            m_imageEditor.AddCircle(centerPoint, radius);
            break;
          }
          case EditCommandType::DRAW_RECTANGLE:
          {
            ImVec2 topLeft = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
            ImVec2 bottomRight = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
            m_imageEditor.AddRectangle(topLeft, bottomRight);
            break;
          }
          case EditCommandType::DRAW_LINE:
          {
            ImVec2 begin = {m_cursorEditPoints[0].x / imageSize.x, m_cursorEditPoints[0].y / imageSize.y};
            ImVec2 end = {m_cursorEditPoints[1].x / imageSize.x, m_cursorEditPoints[1].y / imageSize.y};
            m_imageEditor.AddLine(begin, end);
            break;
          }
        }
        m_currentEditedFrame = m_imageEditor.Draw();

        m_currentCommand = {EditCommandType::DO_NOTHING, false};
        m_cursorEditPoints.clear();
      }
    }
  }

  //else if(m_addTextRequested)
  //{
  //  ImGui::BeginTooltip();
  //  static char buf1[32];
  //  ImGui::InputText("UTF-8 input", buf1, IM_ARRAYSIZE(buf1));
  //  ImGui::SetItemDefaultFocus();
  //  ImGui::EndTooltip();
  //}
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
  
  if (ImGui::ImageButton("save", m_saveIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_currentCommand = { EditCommandType::SAVE_IMAGE, false };
    APP_CORE_INFO("Current frame captured");
  }  

  if (ImGui::ImageButton("start-editing", m_startEditingIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_currentCommand = { EditCommandType::START_EDITING_IMAGE, false };
    APP_CORE_INFO("{} button pressed", "edit-image");
  }
  if (ImGui::ImageButton("addText", m_addTextIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    APP_CORE_INFO("{} button pressed", "addText");
    m_currentCommand = {EditCommandType::ADD_TEXT, false};
  }
  if (ImGui::ImageButton("pencil", m_pencilIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    APP_CORE_INFO("{} button pressed", "pencil");
  }
  if (ImGui::ImageButton("circle", m_circleIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_currentCommand = {EditCommandType::DRAW_CIRCLE, false};
    APP_CORE_INFO("{} button pressed", "circle");
  }
  if (ImGui::ImageButton("line", m_lineIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    APP_CORE_INFO("{} button pressed", "line");
    m_currentCommand = {EditCommandType::DRAW_LINE, false}; 
  }
  if (ImGui::ImageButton("rectangle", m_rectangleIcon->GetShaderResourceView(), size, uvMin, uvMax, iconBg, tintColor))
  {
    m_currentCommand = {EditCommandType::DRAW_RECTANGLE, false}; 
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

