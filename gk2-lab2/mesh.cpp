#include "mesh.h"
#include <algorithm>
using namespace std;
using namespace mini;
using namespace DirectX;

Mesh::Mesh()
	: m_indexCount(0), m_primitiveType(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
{ }

Mesh::Mesh(dx_ptr_vector<ID3D11Buffer>&& vbuffers, vector<unsigned int>&& vstrides, vector<unsigned int>&& voffsets,
	dx_ptr<ID3D11Buffer>&& indices, unsigned int indexCount, D3D_PRIMITIVE_TOPOLOGY primitiveType)
{
	assert(vbuffers.size() == voffsets.size() && vbuffers.size() == vstrides.size());
	m_indexCount = indexCount;
	m_primitiveType = primitiveType;
	m_indexBuffer = move(indices);

	m_vertexBuffers = std::move(vbuffers);
	m_strides = move(vstrides);
	m_offsets = move(voffsets);
}

Mesh::Mesh(Mesh&& right) noexcept
	: m_indexBuffer(move(right.m_indexBuffer)), m_vertexBuffers(move(right.m_vertexBuffers)),
	m_strides(move(right.m_strides)), m_offsets(move(right.m_offsets)),
	m_indexCount(right.m_indexCount), m_primitiveType(right.m_primitiveType)
{
	right.Release();
}

void Mesh::Release()
{
	m_vertexBuffers.clear();
	m_strides.clear();
	m_offsets.clear();
	m_indexBuffer.reset();
	m_indexCount = 0;
	m_primitiveType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
}

Mesh& Mesh::operator=(Mesh&& right) noexcept
{
	Release();
	m_vertexBuffers = move(right.m_vertexBuffers);
	m_indexBuffer = move(right.m_indexBuffer);
	m_strides = move(right.m_strides);
	m_offsets = move(right.m_offsets);
	m_indexCount = right.m_indexCount;
	m_primitiveType = right.m_primitiveType;
	right.Release();
	return *this;
}

void Mesh::Render(const dx_ptr<ID3D11DeviceContext>& context) const
{
	if (!m_indexBuffer || m_vertexBuffers.empty())
		return;
	context->IASetPrimitiveTopology(m_primitiveType);
	context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetVertexBuffers(0, m_vertexBuffers.size(), m_vertexBuffers.data(), m_strides.data(), m_offsets.data());
	context->DrawIndexed(m_indexCount, 0, 0);
}

void Mesh::RenderTriangleStrip(const dx_ptr<ID3D11DeviceContext>& context) const
{
	if (!m_indexBuffer || m_vertexBuffers.empty())
		return;
	context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetVertexBuffers(0, m_vertexBuffers.size(), m_vertexBuffers.data(), m_strides.data(), m_offsets.data());
	context->DrawIndexed(m_indexCount, 0, 0);
}

Mesh::~Mesh()
{
	Release();
}

std::vector<VertexPositionColor> mini::Mesh::ColoredBoxVerts(float width, float height, float depth)
{
	return {
		//Front Face
		{ { -0.5f * width, -0.5f * height, -0.5f * depth }, { 1.0f, 0.0f, 0.0f } },
		{ { +0.5f * width, -0.5f * height, -0.5f * depth }, { 1.0f, 0.0f, 0.0f } },
		{ { +0.5f * width, +0.5f * height, -0.5f * depth }, { 1.0f, 0.0f, 0.0f } },
		{ { -0.5f * width, +0.5f * height, -0.5f * depth }, { 1.0f, 0.0f, 0.0f } },

		//Back Face
		{ { +0.5f * width, -0.5f * height, +0.5f * depth }, { 0.0f, 1.0f, 1.0f } },
		{ { -0.5f * width, -0.5f * height, +0.5f * depth }, { 0.0f, 1.0f, 1.0f } },
		{ { -0.5f * width, +0.5f * height, +0.5f * depth }, { 0.0f, 1.0f, 1.0f } },
		{ { +0.5f * width, +0.5f * height, +0.5f * depth }, { 0.0f, 1.0f, 1.0f } },

		//Left Face
		{ { -0.5f * width, -0.5f * height, +0.5f * depth }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f * width, -0.5f * height, -0.5f * depth }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f * width, +0.5f * height, -0.5f * depth }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f * width, +0.5f * height, +0.5f * depth }, { 0.0f, 1.0f, 0.0f } },

		//Right Face
		{ { +0.5f * width, -0.5f * height, -0.5f * depth }, { 1.0f, 0.0f, 1.0f } },
		{ { +0.5f * width, -0.5f * height, +0.5f * depth }, { 1.0f, 0.0f, 1.0f } },
		{ { +0.5f * width, +0.5f * height, +0.5f * depth }, { 1.0f, 0.0f, 1.0f } },
		{ { +0.5f * width, +0.5f * height, -0.5f * depth }, { 1.0f, 0.0f, 1.0f } },

		//Bottom Face
		{ { -0.5f * width, -0.5f * height, +0.5f * depth }, { 0.0f, 0.0f, 1.0f } },
		{ { +0.5f * width, -0.5f * height, +0.5f * depth }, { 0.0f, 0.0f, 1.0f } },
		{ { +0.5f * width, -0.5f * height, -0.5f * depth }, { 0.0f, 0.0f, 1.0f } },
		{ { -0.5f * width, -0.5f * height, -0.5f * depth }, { 0.0f, 0.0f, 1.0f } },

		//Top Face
		{ { -0.5f * width, +0.5f * height, -0.5f * depth }, { 1.0f, 1.0f, 0.0f } },
		{ { +0.5f * width, +0.5f * height, -0.5f * depth }, { 1.0f, 1.0f, 0.0f } },
		{ { +0.5f * width, +0.5f * height, +0.5f * depth }, { 1.0f, 1.0f, 0.0f } },
		{ { -0.5f * width, +0.5f * height, +0.5f * depth }, { 1.0f, 1.0f, 0.0f } }
	};
}

std::vector<VertexPositionNormal> mini::Mesh::ShadedBoxVerts(float width, float height, float depth)
//DONE : 0.1. Create vertex list for box with normal vectors
{
	return {
		//Front Face
		{ { -0.5f * width, -0.5f * height, -0.5f * depth }, {  0.0f,  0.0f, -1.0f } }, 
		{ { +0.5f * width, -0.5f * height, -0.5f * depth }, {  0.0f,  0.0f, -1.0f } },
		{ { +0.5f * width, +0.5f * height, -0.5f * depth }, {  0.0f,  0.0f, -1.0f } },
		{ { -0.5f * width, +0.5f * height, -0.5f * depth }, {  0.0f,  0.0f, -1.0f } },

		//Back Face
		{ { +0.5f * width, -0.5f * height, +0.5f * depth }, {  0.0f,  0.0f,  1.0f } },
		{ { -0.5f * width, -0.5f * height, +0.5f * depth }, {  0.0f,  0.0f,  1.0f } },
		{ { -0.5f * width, +0.5f * height, +0.5f * depth }, {  0.0f,  0.0f,  1.0f } },
		{ { +0.5f * width, +0.5f * height, +0.5f * depth }, {  0.0f,  0.0f,  1.0f } },

		//Left Face
		{ { -0.5f * width, -0.5f * height, +0.5f * depth }, { -1.0f,  0.0f,  0.0f } },
		{ { -0.5f * width, -0.5f * height, -0.5f * depth }, { -1.0f,  0.0f,  0.0f } },
		{ { -0.5f * width, +0.5f * height, -0.5f * depth }, { -1.0f,  0.0f,  0.0f } },
		{ { -0.5f * width, +0.5f * height, +0.5f * depth }, { -1.0f,  0.0f,  0.0f } },

		//Right Face
		{ { +0.5f * width, -0.5f * height, -0.5f * depth }, {  1.0f,  0.0f,  1.0f } },
		{ { +0.5f * width, -0.5f * height, +0.5f * depth }, {  1.0f,  0.0f,  1.0f } },
		{ { +0.5f * width, +0.5f * height, +0.5f * depth }, {  1.0f,  0.0f,  1.0f } },
		{ { +0.5f * width, +0.5f * height, -0.5f * depth }, {  1.0f,  0.0f,  1.0f } },

		//Bottom Face
		{ { -0.5f * width, -0.5f * height, +0.5f * depth }, {  0.0f, -1.0f,  0.0f } },
		{ { +0.5f * width, -0.5f * height, +0.5f * depth }, {  0.0f, -1.0f,  0.0f } },
		{ { +0.5f * width, -0.5f * height, -0.5f * depth }, {  0.0f, -1.0f,  0.0f } },
		{ { -0.5f * width, -0.5f * height, -0.5f * depth }, {  0.0f, -1.0f,  0.0f } },

		//Top Face
		{ { -0.5f * width, +0.5f * height, -0.5f * depth }, {  0.0f,  1.0f,  0.0f } },
		{ { +0.5f * width, +0.5f * height, -0.5f * depth }, {  0.0f,  1.0f,  0.0f } },
		{ { +0.5f * width, +0.5f * height, +0.5f * depth }, {  0.0f,  1.0f,  0.0f } },
		{ { -0.5f * width, +0.5f * height, +0.5f * depth }, {  0.0f,  1.0f,  0.0f } }
	};
}

std::vector<unsigned short> mini::Mesh::BoxIdxs()
{
	return {
		 0, 2, 1,  0, 3, 2,
		 4, 6, 5,  4, 7, 6,
		 8,10, 9,  8,11,10,
		12,14,13, 12,15,14,
		16,18,17, 16,19,18,
		20,22,21, 20,23,22
	};
}

std::vector<VertexPositionNormal> mini::Mesh::PentagonVerts(float radius)
{
	std::vector<VertexPositionNormal> vertices;
	vertices.reserve(5);
	float a = 0, da = XM_2PI / 5.0f;
	for (int i = 0; i < 5; ++i, a -= da)
	{
		float sina, cosa;
		XMScalarSinCos(&sina, &cosa, a);
		vertices.push_back({ { cosa * radius, sina * radius, 0.0f }, { 0.0f, 0.0f, -1.0f } });
	}
	return vertices;
}

std::vector<unsigned short> mini::Mesh::PentagonIdxs()
{
	return { 0, 1, 2, 0, 2, 3, 0, 3, 4 };
}

std::vector<VertexPositionNormal> mini::Mesh::TriangleVerts(float radius)
{
	std::vector<VertexPositionNormal> vertices;
	vertices.reserve(3);
	float a = 0, da = XM_2PI / 3.0f;
	for (int i = 0; i < 3; ++i, a -= da)
	{
		float sina, cosa;
		XMScalarSinCos(&sina, &cosa, a);
		vertices.push_back({ { cosa * radius, sina * radius, 0.0f }, { 0.0f, 0.0f, -1.0f } });
	}
	return vertices;
}

std::vector<unsigned short> mini::Mesh::TriangleIdxs()
{
	return { 0, 1, 2 };
}

std::vector<VertexPositionNormal> mini::Mesh::DoubleRectVerts(float width, float height)
//TODO : 1.09. Create vertex list for the butterfly wing
{
	return { };
}

std::vector<unsigned short> mini::Mesh::DoubleRectIdxs()
//
// 1.09. Create index list for the butterfly wing
{
	return {  };
}

std::vector<DirectX::XMFLOAT3> mini::Mesh::BillboardVerts(float width, float height)
//TODO : 1.29. Initialize vertex and index list for billboards
{
	return { };
}

std::vector<unsigned short> mini::Mesh::BillboardIdx()
//TODO : 1.29. Initialize vertex and index list for billboards
{
	return { };
}