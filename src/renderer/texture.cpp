#include "renderer/texture.h"
#include "image_handling/image_loader.h"
#include "core/log.h"

#include <wx/image.h>
#include <iostream>
#include <assert.h>
#include <wx/log.h> 
#include "texture.h"

namespace medicimage
{
  Image2D::Image2D(const std::string& filename)
  {
    m_bitmap = wxBitmap(filename, wxBITMAP_TYPE_ANY);
    if (!m_bitmap.IsOk())
    {
      wxLogError("Could not load image from file %s", filename);
      assert(false);
    }
  }

  Image2D::Image2D(wxBitmap &bitmap)
  {
    m_bitmap = bitmap;
    assert(m_bitmap.IsOk());
  }

  Image2D::Image2D(Image2D& image)
  {
    m_bitmap = image.GetBitmap();
  }

  Image2D::Image2D(unsigned char *data, int width, int height, int depth)
  {
    auto image = wxImage(width, height, (unsigned char*)data, true);
    m_bitmap = wxBitmap(image);
    if (!m_bitmap.IsOk())
    {
      wxLogError("Could not create bitmap");
      assert(false);
    }
  }

  Image2D &Image2D::operator=(Image2D &image)
  {
    m_bitmap = wxBitmap(image.GetBitmap());
    return *this;
  }

  medicimage::Image2D::~Image2D()
  {
    // TODO do I need this?
  }
}

