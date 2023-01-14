#pragma once

#include "texture.h"
#include <memory>
#include "imgui.h"
#include "image_editor.h"
#include <d3d11.h>

namespace medicimage
{
class ImageEditor
{
public:
  ImageEditor() = default;
  void Init(ID3D11Device* device);
  void SetTextureForEditing(Texture2D* texture);
  // topLeft, width and height are relative to the texture size, between 0-1
  void DrawRectangle(ImVec2 topLeft, ImVec2 bottomRight);
  void DrawCircle(ImVec2 orig, float radius); // orig is sized between 0-1
  void DrawLine(ImVec2 begin, ImVec2 end);
  void DrawArrow(ImVec2 begin, ImVec2 end);
private:
  Texture2D* m_texture;
};

} // namespace medicimage
