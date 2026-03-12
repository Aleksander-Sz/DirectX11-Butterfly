#pragma once
#include <d3d11.h>

namespace mini
{
	struct SwapChainDescription : DXGI_SWAP_CHAIN_DESC
	{
		SwapChainDescription(HWND wndHwnd, SIZE wndSize);
	};

	struct Viewport : D3D11_VIEWPORT
	{
		explicit Viewport(SIZE size);
	};

	struct Texture2DDescription : D3D11_TEXTURE2D_DESC
	{
		Texture2DDescription(UINT width, UINT height);
		static Texture2DDescription DepthStencilDescription(UINT width, UINT height);
	};

	struct BufferDescription : D3D11_BUFFER_DESC
	{
		BufferDescription(UINT bindFlags, size_t byteWidth);
		static BufferDescription VertexBufferDescription(size_t byteWidth)
		{
			return { D3D11_BIND_VERTEX_BUFFER, byteWidth };
		}
		static BufferDescription IndexBufferDescription(size_t byteWidth)
		{
			return { D3D11_BIND_INDEX_BUFFER, byteWidth };
		}
		static BufferDescription ConstantBufferDescription(size_t byteWidth);
	};

	//******************* NEW *******************
	struct BlendDescription : D3D11_BLEND_DESC
	{
		BlendDescription();
	};

	struct DepthStencilDescription : D3D11_DEPTH_STENCIL_DESC
	{
		DepthStencilDescription();
	};

	struct RasterizerDescription : D3D11_RASTERIZER_DESC
	{
		RasterizerDescription();
	};
}