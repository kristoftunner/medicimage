#include <fstream>
#include <glm/gtx/rotate_vector.hpp>
#include <wx/log.h>

#include "image_editor.h"
#include "image_handling/image_editor.h"
#include "core/log.h"

namespace medicimage
{
std::unique_ptr<Image2D> ImageEditor::s_image;
std::unique_ptr<wxMemoryDC> ImageEditor::s_dc;

void ImageEditor::Begin(std::unique_ptr<Image2D> image)
{
  s_image = std::move(image);
  s_dc = std::make_unique<wxMemoryDC>(s_image->GetBitmap());
  wxImage image2 = s_image->GetBitmap().ConvertToImage();
  bool hasAlpha = image2.HasAlpha();
  s_dc->SetBrush(*wxTRANSPARENT_BRUSH);
}

std::unique_ptr<Image2D> ImageEditor::End()
{
  s_dc->SelectObject(wxNullBitmap);
  s_dc.reset();
  return std::move(s_image);
}


void ImageEditor::DrawCircle(glm::vec2 center, float radius, glm::vec4 color, float thickness, bool filled)
{
  glm::vec2 imageSize = {s_image->GetWidth(), s_image->GetHeight()};
  center *= imageSize;
  radius = radius * glm::length(imageSize);
  auto alpha = color.a;
  color *= 255.0;
  if(filled)
  { // TODO add transparency support with wxGraphicsContext
    s_dc->SetPen(*wxTRANSPARENT_PEN);
    s_dc->SetBrush(wxBrush(wxColor(0,0,0,0)));
    s_dc->DrawCircle(wxPoint{static_cast<int>(center.x), static_cast<int>(center.y)}, static_cast<int>(radius));

    s_dc->SetPen(wxPen(wxColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)), static_cast<int>(thickness)));
    s_dc->SetBrush(wxBrush(wxColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b), 128)));
    s_dc->DrawCircle(wxPoint{static_cast<int>(center.x), static_cast<int>(center.y)}, static_cast<int>(radius));
  }
  else
  {
    wxPen pen = wxPen(wxColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)), static_cast<int>(thickness));
    wxBrush bush = wxBrush(wxColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)), wxBRUSHSTYLE_TRANSPARENT);
    s_dc->SetPen(pen);
    s_dc->SetBrush(bush);
    s_dc->DrawCircle(wxPoint{static_cast<int>(center.x), static_cast<int>(center.y)}, static_cast<int>(radius));
  }

}

void ImageEditor::DrawRectangle(glm::vec2 topleft, glm::vec2 bottomright, glm::vec4 color, float thickness, bool filled)
{
  glm::vec2 imageSize = {s_image->GetWidth(), s_image->GetHeight()};
  topleft *= imageSize;
  bottomright *= imageSize; 
  
  if ((static_cast<int>(topleft.x) != static_cast<int>(bottomright.x)) && (static_cast<int>(topleft.y) != static_cast<int>(bottomright.y)))
  {
    float alpha = color.a;
    color *= 255.0;
    if(filled)
    {
      //s_image.copyTo(overlay);
      //cv::rectangle(overlay, cv::Point{ static_cast<int>(topleft.x), static_cast<int>(topleft.y) }, cv::Point{ static_cast<int>(bottomright.x), static_cast<int>(bottomright.y) },
      //  cv::Scalar(color.b, color.g, color.r), -1);
      //cv::addWeighted(overlay, alpha, s_image, 1 - alpha, 0, s_image);
    }
    else
    {
      wxPen pen = wxPen(wxColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)), static_cast<int>(thickness));
      wxBrush bush = wxBrush(wxColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)), wxBRUSHSTYLE_TRANSPARENT);
      s_dc->SetPen(pen);
      s_dc->SetBrush(bush);
      s_dc->DrawRectangle(wxRect(wxPoint{static_cast<int>(topleft.x), static_cast<int>(topleft.y)}, wxPoint{static_cast<int>(bottomright.x), static_cast<int>(bottomright.y)}));
    }
  }

  //TODO: add rotation 
}

void ImageEditor::DrawArrow(glm::vec2 begin, glm::vec2 end, glm::vec4 color, float thickness, double tipLength)
{
  glm::vec2 imageSize = {s_image->GetWidth(), s_image->GetHeight()};
  begin *= imageSize; 
  end *= imageSize; 
  auto scaledLength = 10 / glm::length(end - begin);
  color *= 255.0;
  glm::vec2 dir, rightDir, leftDir;
  dir = end - begin;
  dir = glm::normalize(dir);
  rightDir = glm::rotate(dir, glm::radians(-90.0f));
  leftDir = glm::rotate(dir, glm::radians(90.0f));
  glm::vec2 arrowPoint1 = end - dir * glm::vec2(10) + rightDir * glm::vec2(8);
  glm::vec2 arrowPoint2 = end - dir * glm::vec2(10) + leftDir * glm::vec2(8);
  
  // Draw the arrowhead
  wxPoint arrowPoints[3] = { {static_cast<int>(end.x), static_cast<int>(end.y)}, {static_cast<int>(arrowPoint1.x), static_cast<int>(arrowPoint1.y)}, {static_cast<int>(arrowPoint2.x), static_cast<int>(arrowPoint2.y)} };
  wxPen pen = wxPen(wxColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)), static_cast<int>(thickness));
  wxBrush bush = wxBrush(wxColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)), wxBRUSHSTYLE_TRANSPARENT);
  s_dc->SetPen(pen);
  s_dc->SetBrush(bush);
  s_dc->DrawLine(wxPoint{static_cast<int>(begin.x), static_cast<int>(begin.y)}, wxPoint{static_cast<int>(end.x), static_cast<int>(end.y)}); 
  s_dc->DrawPolygon(3, arrowPoints);
  //TODO: add rotation 
}
void ImageEditor::DrawLine(glm::vec2 begin, glm::vec2 end, glm::vec4 color, float thickness, double tipLengith)
{
  glm::vec2 imageSize = {s_image->GetWidth(), s_image->GetHeight()};
  begin *= imageSize; 
  end *= imageSize; 
  color *= 255.0;

  s_dc->DrawLine(wxPoint{static_cast<int>(begin.x), static_cast<int>(begin.y)}, wxPoint{static_cast<int>(end.x), static_cast<int>(end.y)}); // TODO: add arrow cuz it is only a line
}

void ImageEditor::DrawText(glm::vec2 bottomLeft, const std::string &text, int fontSize, float thickness)
{
  glm::vec2 imageSize = {s_image->GetWidth(), s_image->GetHeight()};
  bottomLeft *= imageSize;
  wxFont font(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
  s_dc->SetFont(font);
  s_dc->SetTextForeground(wxColour(0, 0, 0));
  s_dc->DrawText(text, wxPoint{static_cast<int>(bottomLeft.x), static_cast<int>(bottomLeft.y)});
}


void ImageEditor::DrawSpline(glm::vec2 begin, glm::vec2 middle, glm::vec2 end, int lineCount, glm::vec4 color, float thickness)
{
  glm::vec2 imageSize = {s_image->GetWidth(), s_image->GetHeight()};
  begin *= imageSize;
  middle *= imageSize;
  end *= imageSize;
  color = color * glm::vec4(255.0);

  wxPen pen = wxPen(wxColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)), static_cast<int>(thickness));
  wxBrush bush = wxBrush(wxColor(static_cast<int>(color.r), static_cast<int>(color.g), static_cast<int>(color.b)), wxBRUSHSTYLE_TRANSPARENT);
  s_dc->SetPen(pen);
  s_dc->SetBrush(bush);

  wxPoint splinePoints[3] = { {static_cast<int>(begin.x), static_cast<int>(begin.y)}, {static_cast<int>(middle.x), static_cast<int>(middle.y)}, {static_cast<int>(end.x), static_cast<int>(end.y)} };
  s_dc->DrawSpline(3, splinePoints);
}

glm::vec2 ImageEditor::GetTextBoundingBox(const std::string &text, int fontSize, float thickness)
{
  int baseline = 0;
  auto textSize = cv::getTextSize(text, s_defaultFont, fontSize, thickness, &baseline); 
  return glm::vec2{static_cast<float>(textSize.width) / static_cast<float>(s_image->GetWidth()), static_cast<float>(textSize.height) / static_cast<float>(s_image->GetHeight())};
}

Image2D ImageEditor::AddFooter(Image2D& image, const std::string& footerText)
{
  // add a sticker to the bottom with the image name, date and time
  // assuming the original texture has 1920x1080 resolution, expanding with 20-20 pixels left/right and 30 bottom, 20 top
  // TODO: do nothing for now

  //cv::UMat borderedImage;
  //cv::copyMakeBorder(image, borderedImage, s_topBorder, s_bottomBorder, s_sideBorder, s_sideBorder, cv::BORDER_CONSTANT , cv::Scalar{255,255,255} ); // adding white border
  //cv::putText(borderedImage, footerText, cv::Point{s_topBorder, borderedImage.rows - s_topBorder}, s_defaultFont, 1, cv::Scalar{0,0,0}, 3);
  //return borderedImage;
  return image;
}

std::unique_ptr<Image2D> ImageEditor::ReplaceImageFooter(const std::string& footerText, Image2D& image)
{
  //cv::UMat image;
  //cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  //image = image(cv::Range(s_topBorder, image.rows - s_bottomBorder), cv::Range(s_sideBorder, image.cols - s_sideBorder));
  //cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
  
  //cv::UMat borderedImage = AddFooter(image, footerText);
  
  //std::unique_ptr<Image2D> dstTexture = std::make_unique<Image2D>(texture->GetName(), borderedImage.cols, borderedImage.rows);
  //dstTexture->SetName(texture->GetName());
  //cv::cvtColor(borderedImage, borderedImage, cv::COLOR_BGR2RGBA);
  //cv::directx::convertToD3D11Texture2D(borderedImage, dstTexture->GetTexturePtr());
  //return std::move(dstTexture);
  return std::make_unique<Image2D>(image);
}

std::unique_ptr<Image2D> ImageEditor::RemoveFooter(Image2D& image)
{
  //cv::UMat image;
  //cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  //image = image(cv::Range(s_topBorder, image.rows - s_bottomBorder), cv::Range(s_sideBorder, image.cols - s_sideBorder));
  //cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
  //cv::cvtColor(image, image, cv::COLOR_BGR2RGBA);
  
  //std::unique_ptr<Image2D> dstTexture = std::make_unique<Image2D>(texture->GetName(), image.cols, image.rows);
  //dstTexture->SetName(texture->GetName());
  //cv::directx::convertToD3D11Texture2D(image, dstTexture->GetTexturePtr());
  //return std::move(dstTexture);
  return std::make_unique<Image2D>(image);
}

std::unique_ptr<Image2D> ImageEditor::AddImageFooter(const std::string& footerText, Image2D& image)
{
  //cv::UMat image;
  //cv::directx::convertFromD3D11Texture2D(texture->GetTexturePtr(), image);
  //cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
  
  //cv::UMat borderedImage = AddFooter(image, footerText);
  
  //std::unique_ptr<Image2D> dstTexture = std::make_unique<Image2D>(texture->GetName(), borderedImage.cols, borderedImage.rows);
  //dstTexture->SetName(texture->GetName());
  //cv::cvtColor(borderedImage, borderedImage, cv::COLOR_BGR2RGBA);
  //cv::directx::convertToD3D11Texture2D(borderedImage, dstTexture->GetTexturePtr());
  //return std::move(dstTexture);
  return std::make_unique<Image2D>(image);
}

} // namespace medicimage
