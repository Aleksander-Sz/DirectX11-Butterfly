#include "butterflyDemo.h"

using namespace mini;
using namespace gk2;
using namespace DirectX;
using namespace std;

#pragma region Constants
const float ButterflyDemo::DODECAHEDRON_R = sqrtf(0.375f + 0.125f * sqrtf(5.0f));
const float ButterflyDemo::DODECAHEDRON_H = 1.0f + 2.0f * DODECAHEDRON_R;
const float ButterflyDemo::DODECAHEDRON_A = XMScalarACos(-0.2f * sqrtf(5.0f));

const float ButterflyDemo::MOEBIUS_R = 1.0f;
const float ButterflyDemo::MOEBIUS_W = 0.1f;
const unsigned int ButterflyDemo::MOEBIUS_N = 128;

const float ButterflyDemo::LAP_TIME = 10.0f;
const float ButterflyDemo::FLAP_TIME = 2.0f;
const float ButterflyDemo::WING_W = 0.15f;
const float ButterflyDemo::WING_H = 0.1f;
const float ButterflyDemo::WING_MAX_A = 8.0f * XM_PIDIV2 / 9.0f; //80 degrees

const unsigned int ButterflyDemo::BS_MASK = 0xffffffff;

const XMFLOAT4 ButterflyDemo::GREEN_LIGHT_POS = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
const XMFLOAT4 ButterflyDemo::BLUE_LIGHT_POS = XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f);
const XMFLOAT4 ButterflyDemo::COLORS[] = {
	XMFLOAT4(253.0f / 255.0f, 198.0f / 255.0f, 137.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(255.0f / 255.0f, 247.0f / 255.0f, 153.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(196.0f / 255.0f, 223.0f / 255.0f, 155.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(162.0f / 255.0f, 211.0f / 255.0f, 156.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(130.0f / 255.0f, 202.0f / 255.0f, 156.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(122.0f / 255.0f, 204.0f / 255.0f, 200.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(109.0f / 255.0f, 207.0f / 255.0f, 246.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(125.0f / 255.0f, 167.0f / 255.0f, 216.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(131.0f / 255.0f, 147.0f / 255.0f, 202.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(135.0f / 255.0f, 129.0f / 255.0f, 189.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(161.0f / 255.0f, 134.0f / 255.0f, 190.0f / 255.0f, 100.0f / 255.0f),
	XMFLOAT4(244.0f / 255.0f, 154.0f / 255.0f, 193.0f / 255.0f, 100.0f / 255.0f)
};
#pragma endregion

#pragma region Initalization
ButterflyDemo::ButterflyDemo(HINSTANCE hInstance)
	: Base(hInstance, 1280, 720, L"Motyl"),
	  m_cbWorld(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	  m_cbView(m_device.CreateConstantBuffer<XMFLOAT4X4, 2>()),
	  m_cbLighting(m_device.CreateConstantBuffer<Lighting>()),
	  m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>())

{
	//Projection matrix
	auto s = m_window.getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));
	m_cbProj = m_device.CreateConstantBuffer<XMFLOAT4X4>();
	UpdateBuffer(m_cbProj, m_projMtx);
	XMFLOAT4X4 cameraMtx;
	XMStoreFloat4x4(&cameraMtx, m_camera.getViewMatrix());
	UpdateCameraCB(cameraMtx);

	//Regular shaders
	auto vsCode = m_device.LoadByteCode(L"vs.cso");
	auto psCode = m_device.LoadByteCode(L"ps.cso");
	m_vs = m_device.CreateVertexShader(vsCode);
	m_ps = m_device.CreatePixelShader(psCode);

	//DONE : 0.3. Change input layout to match new vertex structure
	m_il = m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);

	//Billboard shaders
	vsCode = m_device.LoadByteCode(L"vsBillboard.cso");
	psCode = m_device.LoadByteCode(L"psBillboard.cso");
	m_vsBillboard = m_device.CreateVertexShader(vsCode);
	m_psBillboard = m_device.CreatePixelShader(psCode);
	D3D11_INPUT_ELEMENT_DESC elements[1] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	m_ilBillboard = m_device.CreateInputLayout(elements, vsCode);

	//Render states
	CreateRenderStates();

	//Meshes

	//DONE : 0.2. Create a shaded box model instead of colored one 
	m_box = Mesh::ShadedBox(m_device);

	m_pentagon = Mesh::Pentagon(m_device);
	m_triangle = Mesh::Triangle(m_device);
	m_square = Mesh::Square(m_device);
	m_wing = Mesh::DoubleRect(m_device, 2.0f);
	CreateMoebuisStrip();

	m_bilboard = Mesh::Billboard(m_device, 2.0f);

	//Model matrices
	PrepareShapeForRendering();

	SetShaders();
	ID3D11Buffer* vsb[] = { m_cbWorld.get(),  m_cbView.get(), m_cbProj.get() };
	m_device.context()->VSSetConstantBuffers(0, 3, vsb);
	ID3D11Buffer* psb[] = { m_cbSurfaceColor.get(), m_cbLighting.get() };
	m_device.context()->PSSetConstantBuffers(0, 2, psb);
}

void ButterflyDemo::CreateRenderStates()
//Setup render states used in various stages of the scene rendering
{
	DepthStencilDescription dssDesc;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; //Disable writing to depth buffer
	m_dssNoDepthWrite = m_device.CreateDepthStencilState(dssDesc); // depth stencil state for billboards

	//TODO : 1.20. Setup depth stencil state for writing to stencil buffer

	m_dssStencilWrite = m_device.CreateDepthStencilState(dssDesc);

	//TODO : 1.36. Setup depth stencil state for stencil test for billboards

	m_dssStencilTestNoDepthWrite = m_device.CreateDepthStencilState(dssDesc);

	//TODO : 1.21. Setup depth stencil state for stencil test for 3D objects

	m_dssStencilTest = m_device.CreateDepthStencilState(dssDesc);

	RasterizerDescription rsDesc;
	//DONE : 1.13. Setup rasterizer state with ccw front faces

	rsDesc.CullMode = D3D11_CULL_FRONT;
	m_rsCCW = m_device.CreateRasterizerState(rsDesc);

	BlendDescription bsDesc;
	//TODO : 1.26. Setup alpha blending state

	m_bsAlpha = m_device.CreateBlendState(bsDesc);

	//TODO : 1.30. Setup additive blending state

	m_bsAdd = m_device.CreateBlendState(bsDesc);
}

void ButterflyDemo::CreateTetrahedronMtx()
{
	float r = sqrt(6.0f) / 4.0f / sqrt(3.0f);
	float a = acos(1.0f / 3.0f);
	XMMATRIX matrix[4];
	matrix[0] = XMMatrixRotationX(XM_PI / 2.0f) * XMMatrixTranslation(0.0f, -r, 0.0f);
	matrix[1] = matrix[0] * XMMatrixRotationY(XM_PI) * XMMatrixRotationZ(-XM_PI + a);
	for (int i = 2; i < 4; i++)
	{
		matrix[i] = matrix[i - 1] * XMMatrixRotationY(XM_2PI / 3.0f);
	}
	for (int i = 0; i < 4; i++)
	{
		matrix[i] = matrix[i] * XMMatrixScaling(2.0f, 2.0f, 2.0f);
		XMStoreFloat4x4(&(m_dodecahedronMtx[i]), matrix[i]);
	}
}

void ButterflyDemo::CreateOctahedronMtx()
{
	float r = sqrt(6.0f) / 6.0f * sqrt(3.0f);
	float a = acos(1.0f / 3.0f);
	XMMATRIX matrix[8];
	matrix[0] = XMMatrixRotationX(XM_PI / 2.0f) * XMMatrixTranslation(0.0f, -r, 0.0f) * XMMatrixRotationZ((a-XM_PI)/ 2.0f);
	for (int i = 1; i < 4; i++)
	{
		matrix[i] = matrix[i - 1] * XMMatrixRotationY(XM_PI / 2.0f);
	}
	for (int i = 4; i < 8; i++)
	{
		matrix[i] = matrix[i - 4] * XMMatrixRotationZ(XM_PI);
	}
	for (int i = 0; i < 8; i++)
	{
		matrix[i] = matrix[i] * XMMatrixScaling(1.5f, 1.5f, 1.5f);
		XMStoreFloat4x4(&(m_dodecahedronMtx[i]), matrix[i]);
	}
}

void ButterflyDemo::CreateHexahedronMtx()
{
	float r = 1.0f;
	float a = acos(1.0f / sqrt(3.0f));
	XMMATRIX matrix[6];
	matrix[0] = XMMatrixRotationX(XM_PI / 2.0f) * XMMatrixTranslation(0.0f, -r, 0.0f) * XMMatrixRotationY(XM_PI / 4.0f) * XMMatrixRotationZ(XM_PI-a);
	for (int i = 1; i < 3; i++)
	{
		matrix[i] = matrix[i - 1] *XMMatrixRotationY(XM_2PI / 3.0f);
	}
	for (int i = 3; i < 6; i++)
	{
		matrix[i] = matrix[i - 3] *XMMatrixRotationZ(XM_PI);
	}
	for (int i = 0; i < 6; i++)
	{
		matrix[i] = matrix[i];// *XMMatrixScaling(1.5f, 1.5f, 1.5f);
		XMStoreFloat4x4(&(m_dodecahedronMtx[i]), matrix[i]);
	}
}

void ButterflyDemo::CreateDodecahedronMtx()
//Compute dodecahedronMtx and mirrorMtx
{
    //DONE : 1.01. calculate m_dodecahedronMtx matrices
	float pi = 3.14159265359f;
	float r = sqrt(3.0f/8.0f+sqrt(5.0f)/8.0f);
	float h = 1.0f + 2.0f * r;
	float a = acos(-sqrt(5.0f) / 5.0f);
	// M[0]
	XMMATRIX matrix[12];
	matrix[0] = XMMatrixRotationX(pi / 2.0f) * XMMatrixTranslation(0.0f, -h / 2.0f, 0.0f);
	XMStoreFloat4x4(&(m_dodecahedronMtx[0]), matrix[0]);
	// M[1]
	matrix[1] = matrix[0] * XMMatrixRotationY(pi) * XMMatrixRotationZ(a);
	XMStoreFloat4x4(&(m_dodecahedronMtx[1]), matrix[1]);
	// M[2...5]
	for (int i = 2; i < 6; i++)
	{
		matrix[i] = matrix[i-1] * XMMatrixRotationY(2.0f * pi / 5.0f);
		XMStoreFloat4x4(&(m_dodecahedronMtx[i]), matrix[i]);
	}
	// M[6...11]
	for (int i = 6; i < 12; i++)
	{
		matrix[i] = matrix[i-6] * XMMatrixRotationZ(pi);
		XMStoreFloat4x4(&(m_dodecahedronMtx[i]), matrix[i]);
	}
	//DONE : 1.12. calculate m_mirrorMtx matrices
	XMMATRIX inverseMatrix[12];
	for (int i = 0; i < 12; i++)
	{
		XMMATRIX v = XMMatrixInverse(nullptr, matrix[i]) * matrix[i];
		XMStoreFloat4x4(&(m_mirrorMtx[i]), v);
	}
}

void ButterflyDemo::CreateIcosahedronMtx()
//Compute dodecahedronMtx and mirrorMtx
{
    float pi = 3.14159265359f;
	float r = (3.0f + sqrt(5.0f)) / 4.0f;//2.0f / sqrt(3.0f) / 4.0f * sqrt(10.0f + 2.0f * sqrt(5.0f));
	float a = acos(-sqrt(5.0f) / 3.0f);
	float b = -acos(sqrt(1.0f / 3.0f + 2.0f * sqrt(5.0f) / 15.0f));
	// M[0]
	XMMATRIX matrix[20];
	matrix[0] = XMMatrixRotationX(pi / 2.0f) * XMMatrixTranslation(0.0f, -r, 0.0f) * XMMatrixRotationZ(-pi + b);
	XMStoreFloat4x4(&(m_dodecahedronMtx[0]), matrix[0]);
	// M[1..4]
	for (int i = 1; i < 5; i++)
	{
		matrix[i] = matrix[i-1] * XMMatrixRotationY(2.0f * pi / 5.0f);
	}
	// M[5]
	matrix[5] = XMMatrixRotationX(pi / 2.0f) * XMMatrixRotationY(pi) * XMMatrixTranslation(0.0f, -r, 0.0f) * XMMatrixRotationZ(( - pi + a + b));
	// M[6..9]
	for (int i = 6; i < 10; i++)
	{
		matrix[i] = matrix[i - 1] * XMMatrixRotationY(2.0f * pi / 5.0f);
	}
	// M[10..19]
	for (int i = 10; i < 20; i++)
	{
		matrix[i] = matrix[i - 10] * XMMatrixRotationZ(pi);
	}
	for (int i = 0; i < 20; i++)
	{
		XMStoreFloat4x4(&(m_dodecahedronMtx[i]), matrix[i]);
	}
}

void ButterflyDemo::PrepareShapeForRendering(int shape)
{
	static int currentShape = -1;
	// 1 - Tetrahedron
	// 2 - Octahedron
	// 3 - Hexahedron
	// 4 - Icosahedron
	// 5 - Dodecahedron
	if (currentShape != shape)
	{
		switch (shape)
		{
		case 1:
			CreateTetrahedronMtx();
			break;
		case 2:
			CreateOctahedronMtx();
			break;
		case 3:
			CreateHexahedronMtx();
			break;
		case 4:
			CreateIcosahedronMtx();
			break;
		case 5:
			CreateDodecahedronMtx();
			break;
		default:
			return;
		}
		currentShape = shape;
	}
}

XMFLOAT3 ButterflyDemo::MoebiusStripPos(float t, float s)
//DONE : 1.04. Compute the position of point on the Moebius strip for parameters t and s
{
	const float MOEBIUS_R = 1.0f;
	const float MOEBIUS_W = 0.1f;
	const float x = cos(t) * (MOEBIUS_R + MOEBIUS_W * s * cos(0.5f * t));
	const float y = sin(t) * (MOEBIUS_R + MOEBIUS_W * s * cos(0.5f * t));
	const float z = MOEBIUS_W * s * sin(0.5f * t);
	return { x, y, z };
}

XMVECTOR ButterflyDemo::MoebiusStripDs(float t, float s)
//DONE : 1.05. Return the s-derivative of point on the Moebius strip for parameters t and s
{
	const float x = cos(0.5f * t) * cos(t);
	const float y = cos(0.5f * t) * sin(t);
	const float z = sin(0.5f * t);
	return { x, y, z, 0.0f };
}

XMVECTOR ButterflyDemo::MoebiusStripDt(float t, float s)
//DONE : 1.06. Compute the t-derivative of point on the Moebius strip for parameters t and s
{
	const float MOEBIUS_R = 1.0f;
	const float MOEBIUS_W = 0.1f;
	const float x = -MOEBIUS_R * sin(t) - 0.5f * s * MOEBIUS_W * sin(0.5f * t) * cos(t) - MOEBIUS_W * s * cos(0.5f * t) * sin(t);
	const float y =  MOEBIUS_R * cos(t) - 0.5f * s * MOEBIUS_W * sin(0.5f * t) * sin(t) + MOEBIUS_W * s * cos(0.5f * t) * cos(t);
	const float z =  0.5f * MOEBIUS_W * cos(t);
	return { x, y, z, 0.0f };
}

void ButterflyDemo::CreateMoebuisStrip()
//DONE : 1.07. Create Moebius strip mesh
{
	std::vector<VertexPositionNormal> vertices;
	std::vector<unsigned short> indices;
	float t = 0.0f;
	for (int i = 0; i <= 256; i++)
	{
		for (float s = -1.0f; s < 2.0f; s+=2.0f)
		{
			t = XM_2PI / 128.0f  * (float)i;
			XMVECTOR tangent = XMVector3Normalize(MoebiusStripDt(t, s));
			XMVECTOR binormal = XMVector3Normalize(MoebiusStripDs(t, s));
			XMVECTOR normal = XMVector3Normalize(XMVector3Cross(binormal, tangent));
			XMFLOAT3 pos = MoebiusStripPos(t, s);
			XMVECTOR position = XMLoadFloat3(&pos);
			position = XMVectorScale(position, 0.5f);
			//XMMATRIX matrix = { tangent, normal, binormal, position };
			XMStoreFloat3(&pos, position);
			XMFLOAT3 nor;
			XMStoreFloat3(&nor, normal);
			vertices.push_back(VertexPositionNormal(pos, nor));
		}
		indices.push_back(i * 2);
		indices.push_back(i * 2 + 1);
	}
	m_moebius = Mesh::SimpleTriMesh(m_device, vertices, indices);
}
#pragma endregion

#pragma region Per-Frame Update
void ButterflyDemo::Update(const Clock& c)
{
	Base::Update(c);
	double dt = c.getFrameTime();
	if (HandleCameraInput(dt))
	{
		XMFLOAT4X4 cameraMtx;
		XMStoreFloat4x4(&cameraMtx, m_camera.getViewMatrix());
		UpdateCameraCB(cameraMtx);
	}
	UpdateButterfly(static_cast<float>(dt));
}

void ButterflyDemo::UpdateCameraCB(DirectX::XMFLOAT4X4 cameraMtx)
{
	XMMATRIX mtx = XMLoadFloat4x4(&cameraMtx);
	XMVECTOR det;
	auto invvmtx = XMMatrixInverse(&det, mtx);
	XMFLOAT4X4 view[2] = { cameraMtx };
	XMStoreFloat4x4(view + 1, invvmtx);
	UpdateBuffer(m_cbView, view);
}

void ButterflyDemo::UpdateButterfly(float dtime)
//DONE : 1.10. Compute the matrices for butterfly wings. Position on the strip is determined based on time
{
	static float lap = 0.0f;
	lap += dtime;
	while (lap > LAP_TIME)
		lap -= LAP_TIME;
	//Value of the Moebius strip t parameter
	float t = 2 * lap / LAP_TIME;
	//Angle between wing current and vertical position
	float a = t * WING_MAX_A;
	t *= XM_2PI;
	if (a > WING_MAX_A)
		a = 2 * WING_MAX_A - a;
	//Write the rest of code here
	XMMATRIX translationMtx = XMMatrixTranslation(-1.0f, 0.0f, 0.0f);
	for (int i = 0; i < 2; i++)
	{
		XMVECTOR tangent = XMVector3Normalize(MoebiusStripDt(t, 0.0f));
		XMVECTOR binormal = XMVector3Normalize(MoebiusStripDs(t, 0.0f));
		XMVECTOR normal = XMVector3Normalize(XMVector3Cross(binormal, tangent));
		XMFLOAT3 pos = MoebiusStripPos(t, 0.0f);
		XMVECTOR position = XMLoadFloat3(&pos);
		position = XMVectorScale(position, 0.5f);
		position = XMVectorSetW(position, 1.0f);
		XMMATRIX matrix = { binormal, tangent, normal, position };
		XMMATRIX rotationMtx = XMMatrixRotationY(XM_PI/2.0f) * XMMatrixRotationY(a*(float)(i*2-1));
		matrix = translationMtx * XMMatrixScaling(WING_H/2.0f, WING_W/2.0f, 1.0f) * rotationMtx * XMMatrixTranslation(0.0f, 0.0f, 0.01f) * matrix;
		XMStoreFloat4x4(&(m_wingMtx[i]), matrix);
	}
}
#pragma endregion

#pragma region Frame Rendering Setup
void ButterflyDemo::SetShaders()
{
	m_device.context()->VSSetShader(m_vs.get(), 0, 0);
	m_device.context()->PSSetShader(m_ps.get(), 0, 0);
	m_device.context()->IASetInputLayout(m_il.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ButterflyDemo::SetBillboardShaders()
{
	m_device.context()->VSSetShader(m_vsBillboard.get(), 0, 0);
	m_device.context()->PSSetShader(m_psBillboard.get(), 0, 0);
	m_device.context()->IASetInputLayout(m_ilBillboard.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ButterflyDemo::Set1Light()
//Setup one positional light at the camera
{
	Lighting l{
		/*.ambientColor = */ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		/*.surface = */ XMFLOAT4(0.2f, 0.8f, 0.8f, 200.0f),
		/*.lights =*/ {
			{ /*.position =*/ m_camera.getCameraPosition(), /*.color =*/ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) }
			//other 2 lights set to 0
		}
	};
	UpdateBuffer(m_cbLighting, l);
}

void ButterflyDemo::Set3Lights()
//Setup one white positional light at the camera
//TODO : 1.28. Setup two additional positional lights, green and blue.
{
	Lighting l{
		/*.ambientColor = */ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		/*.surface = */ XMFLOAT4(0.2f, 0.8f, 0.8f, 200.0f),
		/*.lights =*/{
			{ /*.position =*/ m_camera.getCameraPosition(), /*.color =*/ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
			//Write the rest of the code here

		}
	};

	//comment the following line when structure is filled
	ZeroMemory(&l.lights[1], sizeof(Light) * 2);

	UpdateBuffer(m_cbLighting, l);
}
#pragma endregion

#pragma region Drawing
void ButterflyDemo::DrawBox()
{
	XMFLOAT4X4 worldMtx;
	XMStoreFloat4x4(&worldMtx, XMMatrixIdentity());
	UpdateBuffer(m_cbWorld, worldMtx);
	m_box.Render(m_device.context());
}

void ButterflyDemo::DrawTetrahedron(bool colors)
{
	PrepareShapeForRendering(1);
	XMFLOAT3 colorList[12] = {
		{253.0f, 198.0f, 137.0f}, {255.0f, 247.0f, 153.0f}, {196.0f, 223.0f, 155.0f}, {162.0f, 211.0f, 156.0f},
		{130.0f, 202.0f, 156.0f}, {122.0f, 204.0f, 200.0f}, {109.0f, 207.0f, 246.0f}, {125.0f, 167.0f, 216.0f},
		{131.0f, 147.0f, 202.0f}, {135.0f, 129.0f, 189.0f}, {161.0f, 134.0f, 190.0f}, {244.0f, 154.0f, 193.0f} };
	XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
	for (int i = 0; i < 4; i++)
	{
		if (colors)
			color = XMFLOAT4(colorList[i % 10].x / 255.0f, colorList[i % 10].y / 255.0f, colorList[i % 10].z / 255.0f, 1.0f);
		UpdateBuffer(m_cbWorld, m_dodecahedronMtx[i]);
		UpdateBuffer(m_cbSurfaceColor, color);
		m_triangle.Render(m_device.context());
	}
}

void ButterflyDemo::DrawOctahedron(bool colors)
{
	PrepareShapeForRendering(2);
	XMFLOAT3 colorList[12] = {
		{253.0f, 198.0f, 137.0f}, {255.0f, 247.0f, 153.0f}, {196.0f, 223.0f, 155.0f}, {162.0f, 211.0f, 156.0f},
		{130.0f, 202.0f, 156.0f}, {122.0f, 204.0f, 200.0f}, {109.0f, 207.0f, 246.0f}, {125.0f, 167.0f, 216.0f},
		{131.0f, 147.0f, 202.0f}, {135.0f, 129.0f, 189.0f}, {161.0f, 134.0f, 190.0f}, {244.0f, 154.0f, 193.0f} };
	XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
	for (int i = 0; i < 8; i++)
	{
		if (colors)
			color = XMFLOAT4(colorList[i % 10].x / 255.0f, colorList[i % 10].y / 255.0f, colorList[i % 10].z / 255.0f, 1.0f);
		UpdateBuffer(m_cbWorld, m_dodecahedronMtx[i]);
		UpdateBuffer(m_cbSurfaceColor, color);
		m_triangle.Render(m_device.context());
	}
}

void ButterflyDemo::DrawHexahedron(bool colors)
{
	PrepareShapeForRendering(3);
	XMFLOAT3 colorList[12] = {
		{253.0f, 198.0f, 137.0f}, {255.0f, 247.0f, 153.0f}, {196.0f, 223.0f, 155.0f}, {162.0f, 211.0f, 156.0f},
		{130.0f, 202.0f, 156.0f}, {122.0f, 204.0f, 200.0f}, {109.0f, 207.0f, 246.0f}, {125.0f, 167.0f, 216.0f},
		{131.0f, 147.0f, 202.0f}, {135.0f, 129.0f, 189.0f}, {161.0f, 134.0f, 190.0f}, {244.0f, 154.0f, 193.0f} };
	XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
	for (int i = 0; i < 6; i++)
	{
		if (colors)
			color = XMFLOAT4(colorList[i % 10].x / 255.0f, colorList[i % 10].y / 255.0f, colorList[i % 10].z / 255.0f, 1.0f);
		UpdateBuffer(m_cbWorld, m_dodecahedronMtx[i]);
		UpdateBuffer(m_cbSurfaceColor, color);
		m_square.Render(m_device.context());
	}
}

void ButterflyDemo::DrawDodecahedron(bool colors)
//Draw dodecahedron. If color is true, use render faces with corresponding colors. Otherwise render using white color
{
	PrepareShapeForRendering(5);
	//DONE : 1.02. Draw all dodecahedron sides with colors - ignore function parameter for now
	XMFLOAT3 colorList[12] = {
		{253.0f, 198.0f, 137.0f}, {255.0f, 247.0f, 153.0f}, {196.0f, 223.0f, 155.0f}, {162.0f, 211.0f, 156.0f},
		{130.0f, 202.0f, 156.0f}, {122.0f, 204.0f, 200.0f}, {109.0f, 207.0f, 246.0f}, {125.0f, 167.0f, 216.0f},
		{131.0f, 147.0f, 202.0f}, {135.0f, 129.0f, 189.0f}, {161.0f, 134.0f, 190.0f}, {244.0f, 154.0f, 193.0f} };
	XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
	for (int i = 0; i < 12; i++)
	{
		if(colors)
			color = XMFLOAT4(colorList[i].x / 255.0f, colorList[i].y / 255.0f, colorList[i].z / 255.0f, 1.0f);
		UpdateBuffer(m_cbWorld, m_dodecahedronMtx[i]);
		UpdateBuffer(m_cbSurfaceColor, color);
		m_pentagon.Render(m_device.context());
	}
	//DONE : 1.14. Modify function so if colors parameter is set to false, all faces are drawn white instead
	
}

void ButterflyDemo::DrawIcosahedron(bool colors)
{
	PrepareShapeForRendering(4);
	XMFLOAT3 colorList[12] = {
		{253.0f, 198.0f, 137.0f}, {255.0f, 247.0f, 153.0f}, {196.0f, 223.0f, 155.0f}, {162.0f, 211.0f, 156.0f},
		{130.0f, 202.0f, 156.0f}, {122.0f, 204.0f, 200.0f}, {109.0f, 207.0f, 246.0f}, {125.0f, 167.0f, 216.0f},
		{131.0f, 147.0f, 202.0f}, {135.0f, 129.0f, 189.0f}, {161.0f, 134.0f, 190.0f}, {244.0f, 154.0f, 193.0f} };
	XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
	for (int i = 0; i < 20; i++)
	{
		if (colors)
			color = XMFLOAT4(colorList[i%10].x / 255.0f, colorList[i%10].y / 255.0f, colorList[i%10].z / 255.0f, 1.0f);
		UpdateBuffer(m_cbWorld, m_dodecahedronMtx[i]);
		UpdateBuffer(m_cbSurfaceColor, color);
		m_triangle.Render(m_device.context());
	}
}

void ButterflyDemo::DrawMoebiusStrip()
//DONE : 1.08. Draw the Moebius strip mesh
{
	UpdateBuffer(m_cbWorld, XMMatrixIdentity());
	m_moebius.RenderTriangleStrip(m_device.context());
}

void ButterflyDemo::DrawButterfly()
//DONE : 1.11. Draw the butterfly
{
	for (int i = 0; i < 2; i++)
	{
		UpdateBuffer(m_cbWorld, m_wingMtx[i]);
		m_wing.Render(m_device.context());
	}
}

void ButterflyDemo::DrawBillboards()
//Setup billboards rendering and draw them
{
	//TODO : 1.33. Setup shaders and blend state

	//TODO : 1.34. Draw both billboards with appropriate colors and transformations

	//TODO : 1.35. Restore rendering state to it's original values

}

void ButterflyDemo::DrawMirroredWorld(unsigned int i)
//Draw the mirrored scene reflected in the i-th dodecahedron face
{
	//TODO : 1.22. Setup render state for writing to the stencil buffer

	//TODO : 1.23. Draw the i-th face

	//TODO : 1.24. Setup depth stencil state for rendering mirrored world

	//TODO : 1.15. Setup rasterizer state and view matrix for rendering the mirrored world
	m_device.context()->RSSetState(m_rsCCW.get());
	XMMATRIX matrix = XMLoadFloat4x4(&(m_mirrorMtx[i])) * m_camera.getViewMatrix();
	XMFLOAT4X4 v;
	XMStoreFloat4x4(&v, matrix);
	UpdateBuffer(m_cbView, v);

	//TODO : 1.16. Draw 3D objects of the mirrored scene - dodecahedron should be drawn with only one light and no colors and without blending

	//TODO : 1.17. Restore rasterizer state to it's original value
	m_device.context()->RSSetState(nullptr);

	//TODO : 1.37. Setup depth stencil state for rendering mirrored billboards
	
	//TODO : 1.38. Draw mirrored billboards - they need to be drawn after restoring rasterizer state, but with mirrored view matrix

	//TODO : 1.18. Restore view matrix to its original value

	//TODO : 1.25. Restore depth stencil state to it's original value
}

void ButterflyDemo::Render()
{
	Base::Render();

	//render mirrored worlds
	for (int i = 0; i < 12; ++i)
		DrawMirroredWorld(i);

	//render dodecahedron with one light and alpha blending
	m_device.context()->OMSetBlendState(m_bsAlpha.get(), nullptr, BS_MASK);
	Set1Light();
	static int frame = 0;
	frame++;
	if (frame % 250 == 0)
	{
		m_shapeChosen++;
		if (m_shapeChosen >= 6)
			m_shapeChosen = 1;
	}
	//TODO : 1.19. Comment the following line for now
	switch (m_shapeChosen)
	{
	case 1:
		DrawTetrahedron(true);
		break;
	case 2:
		DrawOctahedron(true);
		break;
	case 3:
		DrawHexahedron(true);
		break;
	case 4:
		DrawIcosahedron(true);
		break;
	case 5:
		DrawDodecahedron(true);
		break;
	}
	//TODO : 1.27. Uncomment the above line again
	m_device.context()->OMSetBlendState(nullptr, nullptr, BS_MASK);

	//render the rest of the scene with all lights
	Set3Lights();
	UpdateBuffer(m_cbSurfaceColor, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	//DONE : 1.03. [optional] Comment the following line
	//DrawBox();
	DrawMoebiusStrip();
	DrawButterfly();
	m_device.context()->OMSetDepthStencilState(m_dssNoDepthWrite.get(), 0);
	DrawBillboards();
	m_device.context()->OMSetDepthStencilState(nullptr, 0);
}
#pragma endregion