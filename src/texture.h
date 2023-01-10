#pragma once

#include "renderer.h"
#include <string>

namespace medicimage
{
class Texture2D 
{
private:
	std::string m_name;
	std::string m_fileName;
	unsigned int m_width, m_height;

	D3D11_TEXTURE2D_DESC m_desc;
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_resourceView;
	ID3D11SamplerState* m_samplerState;
	D3D11_SAMPLER_DESC m_samplerDesc;
public:
	Texture2D(unsigned int width, unsigned int height);
	Texture2D(const std::string& name, const std::string& filename);
  Texture2D(ID3D11Texture2D* srcTexture);

	~Texture2D();

	void Bind(unsigned int slot = 0) const;
	void Unbind(unsigned int slot = 0) const;

	inline unsigned int GetWidth() const { return m_width; }
	inline unsigned int GetHeight() const { return m_height; }

	inline const std::string& GetName() const { return m_name; }
	inline const std::string& GetFilepath() const { return m_fileName; }
public:
  void CreateShaderResourceView();
  void CreateSamplerState();
private:
	void Load();
};
  
} // namespace medicimage
