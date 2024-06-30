#pragma once

#include <array>
#include <memory>

#include <SDL.h>

class FrameDebugWindow final {
	struct Context;
	std::unique_ptr<Context> m_context;

	struct ShadowSampleItem final {
		const char* title = nullptr;
		unsigned value = 0;
	};
	std::array<ShadowSampleItem, 4> m_shadowSampleItems;
	size_t m_selectedShadowSample = 0;

	FrameDebugWindow();

	void UpdateShadowTexture(class IDirect3DDevice9* lpD3DDevice);
	void AddShadowMapItem(class IDirect3DDevice9* lpD3DDevice);
	void DrawWindow(class IDirect3DDevice9* lpD3DDevice);

public:
	static FrameDebugWindow& Get();

	void Init(SDL_Window* window, class IDirect3DDevice9* lpD3DDevice);
	void HandleEvent(const SDL_Event* event);
	void Draw(class IDirect3DDevice9* lpD3DDevice);
	void Toggle();
};
