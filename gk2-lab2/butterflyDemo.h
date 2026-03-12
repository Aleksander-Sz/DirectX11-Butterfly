#pragma once
#include "dxApplication.h"
#include "mesh.h"

namespace mini::gk2
{
	//Position and color of a single light
	struct Light
	{
		//Light position coordinates [x, y, z, 1.0f]
		DirectX::XMFLOAT4 position;
		//Light color [r, g, b, 1.0f] - on
		//or [r, g, b, 0.0f] - off
		DirectX::XMFLOAT4 color;
	};

	//Lighting parameters (except surface color)
	struct Lighting
	{
		//ambient reflection color [r, g, b, a]
		DirectX::XMFLOAT4 ambientColor;
		//surface reflection coefficients [ka, kd, ks, m]
		DirectX::XMFLOAT4 surface;
		//Light positions and colors
		Light lights[3];
	};

	class ButterflyDemo : public DxApplication
	{
	public:
		using Base = DxApplication;

		explicit ButterflyDemo(HINSTANCE hInstance);

	protected:
		void Update(const Clock& dt) override;
		void Render() override;

	private:
#pragma region CONSTANTS
		//radius of a circle inscribed i dodecahedron face
		static const float DODECAHEDRON_R;
		//distance between parallel faces
		static const float DODECAHEDRON_H;
		//diheral angle between to neighbouring faces
		static const float DODECAHEDRON_A;

		//radius of thw Moebius strip
		static const float MOEBIUS_R;
		//width of the Moebius strip
		static const float MOEBIUS_W;
		//Number of parts the Moebius strip is divided to
		static const unsigned int MOEBIUS_N;

		//time needed to complete a full lap around the Moebius strip
		static const float LAP_TIME;
		//time of a complete wings flap
		static const float FLAP_TIME;
		//width of a wing
		static const float WING_W;
		//height of a wing
		static const float WING_H;
		//maximum angle between wings
		static const float WING_MAX_A;

		//Various D3D constants
		static const unsigned int BS_MASK;

		//Table of colors for dodecahedron's faces
		static const DirectX::XMFLOAT4 COLORS[12];
		void CreateDodecahadronMtx();
		void CreateRenderStates();

		//Static light positions
		static const DirectX::XMFLOAT4 GREEN_LIGHT_POS;
		static const DirectX::XMFLOAT4 BLUE_LIGHT_POS;
#pragma endregion

#pragma region D3D Resources
		//Shader's constant buffer containing Local -> World matrix
		dx_ptr<ID3D11Buffer> m_cbWorld;
		//ConstantBuffer<DirectX::XMFLOAT4X4> m_cbWorld;
		//Shader's constant buffer containing World -> Camera and Camera -> World matrix
		dx_ptr<ID3D11Buffer> m_cbView;
		//ConstantBuffer<DirectX::XMFLOAT4X4, 2> m_cbView;
		//Shader's constant buffer containing projection matrix
		dx_ptr<ID3D11Buffer> m_cbProj;
		//ConstantBuffer<DirectX::XMFLOAT4X4> m_cbProj;
		//Shader's constant buffer containing lighting parameters (except surface color)
		dx_ptr<ID3D11Buffer> m_cbLighting;
		//ConstantBuffer<Lighting> m_cbLighting;
		//Shader's constant buffer containing surface color
		dx_ptr<ID3D11Buffer> m_cbSurfaceColor;
		//ConstantBuffer<DirectX::XMFLOAT4> m_cbSurfaceColor;

		//Shaders and Input layout for 3D geometry
		dx_ptr<ID3D11VertexShader> m_vs;
		dx_ptr<ID3D11PixelShader> m_ps;
		dx_ptr<ID3D11InputLayout> m_il;

		//Shaders and Input layout for billboards
		dx_ptr<ID3D11VertexShader> m_vsBillboard;
		dx_ptr<ID3D11PixelShader> m_psBillboard;
		dx_ptr<ID3D11InputLayout> m_ilBillboard;

		//Box mesh
		Mesh m_box;
		//Pentagon (side of dodecahedron) mesh
		Mesh m_pentagon;
		//Wing mesh
		Mesh m_wing;
		//Moebuis strip mesh
		Mesh m_moebius;
		//Billboard mesh
		Mesh m_bilboard;

		//Depth stencil state used for drawing billboards without writing to the depth buffer
		dx_ptr<ID3D11DepthStencilState> m_dssNoDepthWrite;
		//Depth stencil state used to fill the stencil buffer
		dx_ptr<ID3D11DepthStencilState> m_dssStencilWrite;
		//Depth stencil state used to perform stencil test when drawing mirrored scene
		dx_ptr<ID3D11DepthStencilState> m_dssStencilTest;
		//Depth stencil state used to perform stencil test when drawing mirrored billboards without writing to the depth buffer
		dx_ptr<ID3D11DepthStencilState> m_dssStencilTestNoDepthWrite;
		//Rasterizer state used to define front faces as counter-clockwise, used when drawing mirrored scene
		dx_ptr<ID3D11RasterizerState> m_rsCCW;
		//Blend state used to draw dodecahedron faced with alpha blending.
		dx_ptr<ID3D11BlendState> m_bsAlpha;
		//Blend state used to draw billboards.
		dx_ptr<ID3D11BlendState> m_bsAdd;
#pragma endregion

#pragma region Matrices
		DirectX::XMFLOAT4X4 m_projMtx;
		DirectX::XMFLOAT4X4 m_dodecahedronMtx[12];
		DirectX::XMFLOAT4X4 m_mirrorMtx[12];
		DirectX::XMFLOAT4X4 m_wingMtx[2];
#pragma endregion

		DirectX::XMFLOAT3 MoebiusStripPos(float t, float s);
		DirectX::XMVECTOR MoebiusStripDs(float t, float s);
		DirectX::XMVECTOR MoebiusStripDt(float t, float s);
		void CreateMoebuisStrip();
		void UpdateCameraCB(DirectX::XMFLOAT4X4 cameraMtx);
		void UpdateButterfly(float dtime);
		void SetShaders();
		void SetBillboardShaders();
		void Set1Light();
		void Set3Lights();
		void DrawBox();
		void DrawDodecahedron(bool colors);
		void DrawMoebiusStrip();
		void DrawButterfly();
		void DrawBillboards();
		void DrawMirroredWorld(unsigned int i);
	};
}