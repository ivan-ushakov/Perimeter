#include "FrameDebugWindow.h"

#include <string_view>

#include <initguid.h>
#include <D3DX9Shader.h>
#include <d3d9.h>

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_win32.h>

#include "StdAfxRD.h"
#include "D3DRender.h"
#include "DrawBuffer.h"

namespace shader {
	const std::string_view VertexShader{ R"(
struct VS_INPUT
{
    float4 pos : POSITION;
    float2 t0 : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 pos : POSITION;
    float2 t0 : TEXCOORD0;
};

VS_OUTPUT main(const VS_INPUT v)
{
    VS_OUTPUT o;

    o.pos = v.pos;
    o.t0 = v.t0;

    return o;
}
)" };

	const std::string_view PixelShader{ R"(
struct VS_OUTPUT
{
    float4 pos : POSITION;
    float2 t0 : TEXCOORD0;
};

texture DepthTexture;
sampler2D DepthSampler = sampler_state
{
    Texture = (DepthTexture);
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

int NumberOfSteps = 10;

float4 main(VS_OUTPUT In) : COLOR
{
    float4 r = 0;
    float depth = 0;
	float stepWeight = 1.0 / NumberOfSteps;
    for (int i = 0; i < NumberOfSteps; i++)
    {
        r += tex2Dproj(DepthSampler, float4(In.t0.x, In.t0.y, depth, 1.0)) * stepWeight;
        depth += stepWeight;
    }
    return r;
}
)" };
}

struct FrameDebugWindow::Context final
{
	bool isEnabled = false;
	LPDIRECT3DVERTEXSHADER9 vertexShader = nullptr;
	LPDIRECT3DPIXELSHADER9 pixelShader = nullptr;
	LPD3DXCONSTANTTABLE pixelShaderConstants = nullptr;
	cTexture* renderTarget = nullptr;
	D3DSURFACE_DESC shadowTextureDesc{};
	IDirect3DTexture9* shadowTexture = nullptr;
	D3DXHANDLE shadowShampleCount = nullptr;

	~Context() {
		RELEASE(shadowTexture);
		RELEASE(pixelShader);
		RELEASE(vertexShader);
	}
};

FrameDebugWindow::FrameDebugWindow() : m_context(new Context()) {
	m_shadowSampleItems = {
		ShadowSampleItem{"5", 5},
		ShadowSampleItem{"10", 10},
		ShadowSampleItem{"15", 15},
		ShadowSampleItem{"20", 20}
	};
}

FrameDebugWindow& FrameDebugWindow::Get() {
	static FrameDebugWindow window;
	return window;
}

void FrameDebugWindow::Init(SDL_Window* window, IDirect3DDevice9* lpD3DDevice) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForD3D(window);
	ImGui_ImplDX9_Init(lpD3DDevice);

	LPD3DXBUFFER vertexShaderBuffer = nullptr;
	RDCALL(D3DXCompileShader(
		shader::VertexShader.data(),
		shader::VertexShader.size(),
		nullptr,
		nullptr,
		"main",
		"vs_3_0",
		0,
		&vertexShaderBuffer,
		nullptr,
		nullptr
	));
	RDCALL(lpD3DDevice->CreateVertexShader(
		reinterpret_cast<const DWORD *>(vertexShaderBuffer->GetBufferPointer()),
		&m_context->vertexShader)
	);

	LPD3DXBUFFER pixelShaderBuffer = nullptr;
	RDCALL(D3DXCompileShader(
		shader::PixelShader.data(),
		shader::PixelShader.size(),
		nullptr,
		nullptr,
		"main",
		"ps_3_0",
		0,
		&pixelShaderBuffer,
		nullptr,
		&m_context->pixelShaderConstants
	));
	RDCALL(lpD3DDevice->CreatePixelShader(
		reinterpret_cast<const DWORD*>(pixelShaderBuffer->GetBufferPointer()),
		&m_context->pixelShader)
	);

	m_context->shadowShampleCount = m_context->pixelShaderConstants->
		GetConstantByName(nullptr, "NumberOfSteps");
}

void FrameDebugWindow::HandleEvent(const SDL_Event* event) {
	ImGui_ImplSDL2_ProcessEvent(event);
}

void FrameDebugWindow::Draw(IDirect3DDevice9* lpD3DDevice) {
	if (!m_context->isEnabled) {
		return;
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	DrawWindow(lpD3DDevice);

	// Rendering
	ImGui::EndFrame();

	lpD3DDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	lpD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	lpD3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	if (lpD3DDevice->BeginScene() >= 0) {
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		lpD3DDevice->EndScene();
	}
}

void FrameDebugWindow::Toggle() {
	m_context->isEnabled = !m_context->isEnabled;
}

void FrameDebugWindow::UpdateShadowTexture(IDirect3DDevice9* lpD3DDevice) {
	if (m_context->renderTarget == nullptr) {
		if (!gb_RenderDevice3D->GetShadowZBuffer().d3d) {
			return;
		}

		auto surface = gb_RenderDevice3D->GetShadowZBuffer().d3d;
		RDCALL(surface->GetDesc(&m_context->shadowTextureDesc));

		void* pContainer = nullptr;
		HRESULT hr = surface->GetContainer(IID_IDirect3DTexture9, &pContainer);
		if (FAILED(hr) || !pContainer) {
			return;
		}
		m_context->shadowTexture = reinterpret_cast<IDirect3DTexture9*>(pContainer);

		m_context->renderTarget = gb_RenderDevice3D->GetTexLibrary()->CreateRenderTexture(
			m_context->shadowTextureDesc.Width,
			m_context->shadowTextureDesc.Height,
			TEXTURE_RENDER32,
			false
		);
	}

	RDCALL(lpD3DDevice->BeginScene());

	gb_RenderDevice3D->SetRenderTarget(m_context->renderTarget, SurfaceImage::NONE);

	RDCALL(lpD3DDevice->SetVertexShader(m_context->vertexShader));
	RDCALL(lpD3DDevice->SetPixelShader(m_context->pixelShader));

	RDCALL(m_context->pixelShaderConstants->SetInt(
		lpD3DDevice,
		m_context->shadowShampleCount,
		m_shadowSampleItems[m_selectedShadowSample].value)
	);
	RDCALL(lpD3DDevice->SetTexture(0, m_context->shadowTexture));

	auto db = gb_RenderDevice3D->GetDrawBuffer(sVertexXYZT1::fmt, PT_TRIANGLES);
	sVertexXYZT1* v = db->LockQuad<sVertexXYZT1>(1);

	v[0].z = v[1].z = v[2].z = v[3].z = 0;

	v[0].x = -1;
	v[0].y = 1;

	v[1].x = -1;
	v[1].y = -1;

	v[2].x = 1;
	v[2].y = 1;

	v[3].x = 1;
	v[3].y = -1;

	v[0].u1() = 0.0f; v[0].v1() = 0.0f;
	v[1].u1() = 0.0f; v[1].v1() = 1.0f;
	v[2].u1() = 1.0f; v[2].v1() = 0.0f;
	v[3].u1() = 1.0f; v[3].v1() = 1.0f;

	db->Unlock();
	db->Draw();

	RDCALL(lpD3DDevice->EndScene());

	gb_RenderDevice3D->RestoreRenderTarget();
}

void FrameDebugWindow::AddShadowMapItem(IDirect3DDevice9* lpD3DDevice) {
	if (!ImGui::BeginTabItem("Shadow map")) {
		return;
	}

	if (ImGui::BeginCombo("Samples: ", m_shadowSampleItems[m_selectedShadowSample].title)) {
		for (size_t i = 0; i < m_shadowSampleItems.size(); i++) {
			if (ImGui::Selectable(m_shadowSampleItems[i].title, m_selectedShadowSample == i)) {
				m_selectedShadowSample = i;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();
	if (ImGui::Button("Update")) {
		UpdateShadowTexture(lpD3DDevice);
	}

	if (m_context->renderTarget) {
		ImGui::Image(m_context->renderTarget->GetFrameImage(0)->d3d, ImVec2(400, 400));
	}

	ImGui::EndTabItem();
}

void FrameDebugWindow::DrawWindow(IDirect3DDevice9* lpD3DDevice) {
	ImGui::Begin("Frame debug");
	ImGui::SetWindowSize(ImVec2(800.0f, 600.0f));

	ImGuiTabBarFlags flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("Frame", flags)) {
		AddShadowMapItem(lpD3DDevice);

		if (ImGui::BeginTabItem("Light map")) {
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}
