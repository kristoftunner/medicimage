#pragma once

#include <string>
#include <filesystem>
#include <wx/bitmap.h>
#include <wx/image.h>

using path = std::filesystem::path;

namespace medicimage
{
class Image2D 
{
private:
  wxBitmap m_bitmap; 
public:
  // TODO: create factory functions for image
  Image2D() = default;
	Image2D(const std::string& filename); // TODO: make the filename std::filesystem::path
  Image2D(Image2D& image);
  Image2D(unsigned char* data, int width, int height, int depth = 3);
  Image2D& operator=(Image2D& image);
	~Image2D();

  wxBitmap& GetBitmap() { return m_bitmap; }

	inline unsigned int GetWidth() const { return m_bitmap.GetLogicalWidth(); }
	inline unsigned int GetHeight() const { return m_bitmap.GetLogicalHeight(); }
private:
	void Load();
};
  
} // namespace medicimage
