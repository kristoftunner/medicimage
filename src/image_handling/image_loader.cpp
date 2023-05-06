#include <iostream>

#include "image_handling/image_loader.h"
#include "stb_image.h"
#include "stb_image_resize.h"

namespace medicimage 
{

Image::Image(const std::string& path)
{
  LoadImage(path);
}

void Image::LoadImage(const std::string& path)
{
  constexpr int outputChannels = 4;
  int width, heigth, channels;
  stbi_set_flip_vertically_on_load(0);
  uint8_t* data = stbi_load(path.c_str(), &width, &heigth, &channels, outputChannels); 
  
  // some sanity checks
  if(data == nullptr)
  {
    std::cout << "Unable to load image" << std::endl; // TODO: use exception or useful arror handling
    m_imageLoaded = false;
  }
  else
  {
    m_desc.width = width;
    m_desc.height = heigth;
    m_desc.channels = outputChannels; 
    m_image.assign(data, data + (width * heigth * 4));
    m_imageLoaded = true;
  } 
  stbi_image_free(data);
}

void Image::FillAplha()
{
  int imageSize = m_desc.width * m_desc.height * 4;
  std::vector<uint8_t> rgbaImage(imageSize, 0);

  for(size_t pixel = 0; pixel < m_image.size() / m_desc.channels; pixel++)
  {
    rgbaImage[pixel * 4]     = m_image[pixel * m_desc.channels]; 
    rgbaImage[pixel * 4 + 1] = m_image[pixel * m_desc.channels + 1]; 
    rgbaImage[pixel * 4 + 2] = m_image[pixel * m_desc.channels + 2]; 
    rgbaImage[pixel * 4 + 3] = 0xff; 
  }

  m_image = std::move(rgbaImage);
  m_desc.channels = 4;
}

void Image::Resize(int width, int height)
{
  uint8_t* data = static_cast<uint8_t*>(std::malloc(width * height * m_desc.channels));
  if(stbir_resize_uint8(m_image.data(),m_desc.width, m_desc.height,0,data, width, height, 0, m_desc.channels) != 1)
    std::cout << "Error during image resize" << std::endl; //TODO: useful error handling
  else
  {
    m_desc = {width, height, m_desc.channels};
    m_image.assign(data, data + (m_desc.width * m_desc.height * m_desc.channels));
  }
  std::free(data);
}
  
} // namespace medicimage 