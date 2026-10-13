#pragma once
#include "Emu/RSX/GSFrameBase.h"

// GSFrameBase implementation backed by a UIView whose layer is a CAMetalLayer.
// Vulkan (MoltenVK) presents straight to the layer, so flip()/present_frame() are no-ops.
class ios_gs_frame final : public GSFrameBase
{
	void* m_view; // UIView*

public:
	explicit ios_gs_frame(void* uiview) : m_view(uiview) {}

	void close() override {}
	void reset() override {}
	bool shown() override { return true; }
	void hide() override {}
	void show() override {}
	void toggle_fullscreen() override {}

	void delete_context(draw_context_t) override {}
	draw_context_t make_context() override { return nullptr; }
	void set_current(draw_context_t) override {}
	void flip(draw_context_t, bool = false) override {}
	int client_width() override;
	int client_height() override;
	f64 client_display_rate() override;
	bool has_alpha() override { return false; }

	display_handle_t handle() const override { return m_view; }

	bool can_consume_frame() const override { return false; }
	void present_frame(std::vector<u8>&&, u32, u32, u32, bool) const override {}
	void take_screenshot(std::vector<u8>&&, u32, u32, bool) override {}

	void update_title(double = 0.0) override {}
};
