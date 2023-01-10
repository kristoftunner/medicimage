#include "texture.h"
#include "image_loader.h"
#include <iostream>

namespace medicimage
{

	Texture2D::Texture2D(unsigned int width, unsigned int height)
		: m_fileName("NULL")
	{
		m_width = width;
		m_height = height;
		Load();
	}

	Texture2D::Texture2D(const std::string& name, const std::string& filename)
		: m_name(name), m_fileName(filename)
	{
		Load();
    CreateShaderResourceView();
    CreateSamplerState();
	}

  Texture2D::Texture2D(ID3D11Texture2D* srcTexture)
  {
    if(srcTexture ==nullptr)
      std::cout << "Something went wrong!" << std::endl;
    m_texture->GetDesc(&m_desc); 
    CreateShaderResourceView();
    CreateSamplerState();
  }

	Texture2D::~Texture2D()
	{
    // TODO: release what can be released
  }

	void Texture2D::Load()
	{
    Image image(m_fileName);
		m_width = image.Width();
		m_height = image.Height();
		
    unsigned int stride = 4;
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = image.GetImage().data();
		initData.SysMemPitch = image.BytesPerRow();
		initData.SysMemSlicePitch = image.BytesPerRow() * image.Height();

		ZeroMemory(&m_desc, sizeof(D3D11_TEXTURE2D_DESC));
		m_desc.Width = m_width;
		m_desc.Height = m_height;
		m_desc.MipLevels = 1;
		m_desc.ArraySize = 1;
		m_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		m_desc.Usage = D3D11_USAGE_IMMUTABLE;
		m_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		m_desc.SampleDesc.Count = 1;
		m_desc.SampleDesc.Quality = 0;

		ThrowIfFailed(Renderer::GetInstance().GetDevice()->CreateTexture2D(&m_desc, &initData, &m_texture));
	}
  
  void Texture2D::CreateShaderResourceView()
  {
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = m_desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = m_desc.MipLevels;

		ThrowIfFailed(Renderer::GetInstance().GetDevice()->CreateShaderResourceView(m_texture, &srvDesc, &m_resourceView));

		m_desc.Usage = D3D11_USAGE_DEFAULT;
		m_desc.CPUAccessFlags = 0;
		m_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  }

  void Texture2D::CreateSamplerState()
  {
		ZeroMemory(&m_samplerDesc, sizeof(D3D11_SAMPLER_DESC));
		m_samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		m_samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		m_samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		m_samplerDesc.MinLOD = 0;
		m_samplerDesc.MaxLOD = 11;
		m_samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; 
		m_samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		m_samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		ThrowIfFailed(Renderer::GetInstance().GetDevice()->CreateSamplerState(&m_samplerDesc, &m_samplerState));
  }

	void Texture2D::Bind(unsigned int slot) const
	{
		Renderer::GetInstance().GetDeviceContext()->PSSetShaderResources(slot, 1, &m_resourceView);
		Renderer::GetInstance().GetDeviceContext()->PSSetSamplers(slot, 1, &m_samplerState);
	}

	void Texture2D::Unbind(unsigned int slot) const
	{
		ID3D11ShaderResourceView* rv = nullptr;
		Renderer::GetInstance().GetDeviceContext()->PSSetShaderResources(slot, 1, &rv);
	}

} // namespace medicimage
