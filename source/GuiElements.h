#pragma once
#include <G3D/G3D.h>
#include "ConfigFiles.h"
#include "TargetEntity.h"

class App;

struct WaypointDisplayConfig {
	// Formatting parameters
	int tree_display_width_px = 400;
	int tree_display_height_px = 400;
	float tree_height = 15;
	float tree_indent = 16;
	float idx_column_width_px = 50;
	float time_column_width_px = 100;
	float xyz_column_width_px = 250;

	WaypointDisplayConfig(	int width = 400, 
							int height = 400, 
							float line_height=15, 
							float indent = 16, 
							float idx_column_width = 50, 
							float time_column_width = 100, 
							float xyz_column_width = 250) {
		tree_display_width_px = width;
		tree_display_height_px = height;
		tree_height = line_height;
		tree_indent = indent;
		idx_column_width_px = idx_column_width;
		time_column_width_px = time_column_width;
		xyz_column_width_px = xyz_column_width;
	}
};

class WaypointDisplay : public GuiWindow {
protected:

	// Tree class (copied from ProfilerWindow)
	class TreeDisplay : public GuiControl {
	public:
		shared_ptr<GFont> m_icon;
		int m_selectedIdx = -1;
		WaypointDisplayConfig m_config;
		shared_ptr<Array<Destination>> m_waypoints;

		virtual bool onEvent(const GEvent& event) override;
		TreeDisplay(GuiWindow* w, WaypointDisplayConfig config, shared_ptr<Array<Destination>> waypoints);
		virtual void render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const override;

	};

	GuiScrollPane*	m_scrollPane;
	TreeDisplay*	m_treeDisplay;
	App* m_app;

	WaypointDisplay(App* app, const shared_ptr<GuiTheme>& theme, WaypointDisplayConfig config, shared_ptr<Array<Destination>> waypoints);
public:

	int getSelected() {
		return m_treeDisplay->m_selectedIdx;
	}

	void setSelected(int idx) {
		m_treeDisplay->m_selectedIdx = idx;
	}

	virtual void setManager(WidgetManager* manager);
	static shared_ptr<WaypointDisplay> create(App* app, const shared_ptr<GuiTheme>& theme, WaypointDisplayConfig config, shared_ptr<Array<Destination>> waypoints) {
		return createShared<WaypointDisplay>(app, theme, config, waypoints);

	}
};

class PlayerControls : public GuiWindow {
protected:
	PlayerControls(SessionConfig& config, std::function<void()> exportCallback,
		const shared_ptr<GuiTheme>& theme, float width = 400.0f, float height = 10.0f);

public:
	static shared_ptr<PlayerControls> create(SessionConfig& config, std::function<void()> exportCallback,
		const shared_ptr<GuiTheme>& theme, float width = 400.0f, float height = 10.0f) {
		return createShared<PlayerControls>(config, exportCallback, theme, width, height);
	}
};

class RenderControls : public GuiWindow {
protected:
	RenderControls(SessionConfig& config, UserConfig& user, bool& drawFps, bool& turbo, const int numReticles, float& brightness,
		const shared_ptr<GuiTheme>& theme, const int maxFrameDelay = 360, const float minFrameRate = 1.0f, const float maxFrameRate=1000.0f, float width=400.0f, float height=10.0f);
public:
	static shared_ptr<RenderControls> create(SessionConfig& config, UserConfig &user, bool& drawFps, bool& turbo, const int numReticles, float& brightness,
		const shared_ptr<GuiTheme>& theme, const int maxFrameDelay = 360, const float minFrameRate = 1.0f, const float maxFrameRate=1000.0f, float width = 400.0f, float height = 10.0f) {
		return createShared<RenderControls>(config, user, drawFps, turbo, numReticles, brightness, theme, maxFrameDelay, minFrameRate, maxFrameRate, width, height);
	}
};

class WeaponControls : public GuiWindow {
protected:
	WeaponControls(WeaponConfig& config, const shared_ptr<GuiTheme>& theme, float width = 400.0f, float height = 10.0f);
public:
	static shared_ptr<WeaponControls> create(WeaponConfig& config, const shared_ptr<GuiTheme>& theme, float width = 400.0f, float height = 10.0f) {
		return createShared<WeaponControls>(config, theme, width, height);
	}
};