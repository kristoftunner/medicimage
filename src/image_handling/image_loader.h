#pragma once

#include <vector>

namespace medicimage 
{

struct ImageDescriptor
{
  ImageDescriptor() = default;
  ImageDescriptor(int width, int height, int channels) 
  : width(width), height(height), channels(channels){}
  int width=0, height=0, channels=0;
};

class Image 
{
public:
  Image() = default;
  Image(const std::string& path);
  void LoadImage(const std::string& path);
  void Resize(int width, int height);
  const std::vector<uint8_t>& GetImage(){return m_image;}
  const ImageDescriptor& GetImageDescriptor(){return m_desc;}
  bool ImageLoaded(){return m_imageLoaded;}
  int BytesPerRow(){return m_desc.channels * m_desc.width;}
  int Width(){return m_desc.width;}
  int Height(){return m_desc.height;}
private:
  void FillAplha();
private:
  // for now it loads only 8bit pixel images
  std::vector<uint8_t> m_image;
  ImageDescriptor m_desc;
  bool m_imageLoaded = false;
};

} // namespace medicimage 

