// Qt-free replacements for pieces of the desktop frontend that the emulator core links against.
// (iOS build, RPCS3_NO_QT). Everything here is normally provided by the Qt frontend.
#include "stdafx.h"
#include "Input/mouse_gyro_handler.h"
#include "Emu/Io/pad_types.h"

#include <algorithm>
#include <string>

// Defined in rpcs3.cpp on desktop (set from the --input-config command line option).
// Read by Emu/Io/pad_config.cpp. Empty = use the normal per-game / global pad config.
std::string g_input_config_override;

LOG_CHANNEL(gui_log, "GUI");

// ---------------------------------------------------------------------------------------------
// mouse_gyro_handler: same state machine as Input/mouse_gyro_handler.cpp, minus the Qt event
// handling (that file includes <QEvent>/<QMouseEvent>/... and cannot be built without Qt).
// pad_thread owns an instance and calls set_enabled()/apply_gyro(), so these must exist.
// ---------------------------------------------------------------------------------------------
void mouse_gyro_handler::clear()
{
	m_active = false;
	m_reset = false;
	m_gyro_x = DEFAULT_MOTION_X;
	m_gyro_y = DEFAULT_MOTION_Y;
	m_gyro_z = DEFAULT_MOTION_Z;
}

bool mouse_gyro_handler::toggle_enabled()
{
	m_enabled = !m_enabled;
	clear();
	return m_enabled;
}

void mouse_gyro_handler::set_enabled(bool enabled)
{
	m_enabled = enabled;
	clear();
}

void mouse_gyro_handler::set_gyro_active()
{
	gui_log.notice("Mouse-based gyro activated");
	m_active = true;
}

void mouse_gyro_handler::set_gyro_reset()
{
	gui_log.notice("Mouse-based gyro deactivated");
	m_active = false;
	m_reset = true;
}

void mouse_gyro_handler::set_gyro_xz(s32 off_x, s32 off_y)
{
	if (!m_active)
		return;

	m_gyro_x = static_cast<u16>(std::clamp(off_x, 0, DEFAULT_MOTION_X * 2 - 1));
	m_gyro_z = static_cast<u16>(std::clamp(off_y, 0, DEFAULT_MOTION_Z * 2 - 1));
}

void mouse_gyro_handler::set_gyro_y(s32 steps)
{
	if (!m_active)
		return;

	m_gyro_y = static_cast<u16>(std::clamp(m_gyro_y + steps, 0, DEFAULT_MOTION_Y * 2 - 1));
}

// No Qt events and no mouse on iOS: nothing to do.
void mouse_gyro_handler::handle_event(QEvent*, const QWindow&)
{
}

void mouse_gyro_handler::apply_gyro(const std::shared_ptr<Pad>& pad)
{
	if (!m_enabled || !pad || !pad->is_connected())
		return;

	if (m_reset)
	{
		pad->m_sensors[0].m_value = DEFAULT_MOTION_X;
		pad->m_sensors[1].m_value = DEFAULT_MOTION_Y;
		pad->m_sensors[2].m_value = DEFAULT_MOTION_Z;
		clear();
	}
	else
	{
		pad->m_sensors[0].m_value = m_gyro_x;
		pad->m_sensors[1].m_value = m_gyro_y;
		pad->m_sensors[2].m_value = m_gyro_z;
	}
}
