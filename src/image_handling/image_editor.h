#pragma once

#include "renderer/texture.h"

#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>

#include <opencv2/imgproc.hpp>
#include <wx/dcmemory.h>
#include <wx/image.h>
namespace medicimage
{

struct ImageBorders
{
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
};
class ImageEditor
{
  public:
    ImageEditor() = default;
    // topLeft, width and height are relative to the texture size, between 0-1
    static std::unique_ptr<Image2D> AddImageFooter(const std::string &footerText, Image2D &image);
    static std::unique_ptr<Image2D> ReplaceImageFooter(const std::string &footerText, Image2D &image);
    static std::unique_ptr<Image2D> RemoveFooter(Image2D &texture);

    static void Begin(std::unique_ptr<Image2D> image);
    static std::unique_ptr<Image2D> End();
    static void DrawCircle(glm::vec2 center, float radius, glm::vec4 color, float thickness, bool filled);
    static void DrawRectangle(glm::vec2 topleft, glm::vec2 bottomright, glm::vec4 color, float thickness, bool filled);
    static void DrawArrow(glm::vec2 begin, glm::vec2 end, glm::vec4 color, float thickness);
    static void DrawLine(glm::vec2 begin, glm::vec2 end, glm::vec4 color, float thickness);
    static void DrawText(glm::vec2 bottomLeft, const std::string &text, int fontSize, float thickness);
    static void DrawSpline(glm::vec2 begin, glm::vec2 middle, glm::vec2 end, int lineCount, glm::vec4 color,
                           float thickness);
    static glm::vec2 GetTextBoundingBox(const std::string &text, int fontSize, float thickness);

    static glm::vec2 GetTopleftBorderSize()
    {
        return {s_sideBorder, s_topBorder};
    }
    static ImageBorders GetImageBorders();

  private:
    static Image2D AddFooter(Image2D &image, const std::string &footerText);
    static constexpr int s_sideBorder = 10;
    static constexpr int s_topBorder = 10;
    static constexpr int s_bottomBorder = 50;
    static constexpr auto s_defaultFont = cv::FONT_HERSHEY_SIMPLEX;
    // TODO: move this into a better place
    static std::unique_ptr<Image2D> s_image;
    static std::unique_ptr<wxMemoryDC> s_dc;
};

} // namespace medicimage
