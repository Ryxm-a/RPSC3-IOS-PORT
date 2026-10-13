// Qt-free glue between the iOS app shell and the RPCS3 core (mirrors headless_application).
#include "ios_glue.h"
#include "ios_gs_frame.h"

#include "stdafx.h"
#include "Emu/System.h"
#include "Emu/system_config.h"
#include "Emu/emu_callbacks.h"
#include "Emu/IdManager.h"
#include "Emu/RSX/Null/NullGSRender.h"
#include "Emu/RSX/VK/VKGSRender.h"
#include "Emu/Cell/Modules/cellMsgDialog.h"
#include "Emu/Cell/Modules/cellOskDialog.h"
#include "Emu/Cell/Modules/cellSaveData.h"
#include "Emu/Cell/Modules/sceNpTrophy.h"
#include "Emu/Io/Null/null_camera_handler.h"
#include "Emu/Io/Null/null_music_handler.h"
#include "Emu/Io/Null/NullKeyboardHandler.h"
#include "Emu/Io/Null/NullMouseHandler.h"
#include "Emu/Io/KeyboardHandler.h"
#include "Emu/Io/MouseHandler.h"
#include "Emu/Audio/AudioBackend.h"
#include "Emu/Audio/Null/NullAudioBackend.h"
#include "Emu/Audio/Null/null_enumerator.h"
#include "Emu/Audio/Cubeb/CubebBackend.h"
#include "Emu/Audio/Cubeb/cubeb_enumerator.h"
#include "Input/pad_thread.h"
#include "util/logs.hpp"

#include <dispatch/dispatch.h>
#include <clocale>

LOG_CHANNEL(ios_log, "iOS");

static void* g_view = nullptr;

// ---------------------------------------------------------------------------------------------
// Logging. The desktop frontend (rpcs3.cpp) installs a file listener at startup; the iOS shell
// has to do the same or nothing is ever written. The log lives in the app's Documents folder
// (visible in the Files app because UIFileSharingEnabled is set): Documents/RPCS3.log
// ---------------------------------------------------------------------------------------------
namespace
{
	std::unique_ptr<logs::listener> g_log_file;

	// Mirrors errors to stderr so they show up in Xcode / Console.app as well.
	struct ios_stderr_listener final : logs::listener
	{
		void log(u64 /*stamp*/, const logs::message& msg, std::string_view prefix, std::string_view text) override
		{
			const logs::level lvl = msg;
			if (lvl != logs::level::fatal && lvl != logs::level::error)
			{
				return;
			}

			std::string line = "RPCS3: ";

			if (!prefix.empty())
			{
				line += prefix;
				line += ": ";
			}

			if (msg->name && '\0' != *msg->name)
			{
				line += msg->name;
				line += ": ";
			}

			line += text;
			line += '\n';
			std::fwrite(line.data(), 1, line.size(), stderr);
		}
	};

	void init_ios_logging(const char* data_dir)
	{
		std::string dir = (data_dir && *data_dir) ? std::string(data_dir) : fs::get_log_dir();

		if (!dir.empty() && dir.back() != '/')
		{
			dir += '/';
		}

		// Limit the log to ~25% of the free space, but never less than 8 MB or more than 256 MB
		u64 budget = 256ull * 1024 * 1024;

		if (fs::device_stat stats{}; fs::statfs(dir, stats))
		{
			budget = std::min<u64>(budget, stats.avail_free / 4);
		}

		budget = std::max<u64>(budget, 8ull * 1024 * 1024);

		g_log_file = logs::make_file_listener(dir + "RPCS3.log", budget); // also registers itself
		logs::listener::add(new ios_stderr_listener());                    // lives for the whole process

		ios_log.always()("RPCS3 iOS port starting. Log file: %sRPCS3.log", dir);
	}
}


[[noreturn]] void report_fatal_error(std::string_view text, bool /*is_html*/, bool /*include_help_text*/)
{
	ios_log.fatal("%s", text);
	fprintf(stderr, "RPCS3 fatal error: %.*s\n", static_cast<int>(text.size()), text.data());
	logs::listener::sync_all(); // make sure the message reaches Documents/RPCS3.log
	std::abort();
}

static void create_ios_callbacks()
{
	g_emu_callbacks.call_from_main_thread = [](std::function<void()> func, atomic_t<u32>* wake_up)
	{
		dispatch_async(dispatch_get_main_queue(), ^{
			func();
			if (wake_up)
			{
				*wake_up = true;
				wake_up->notify_one();
			}
		});
	};

	g_emu_callbacks.try_to_quit = [](bool force_quit, std::function<void()> on_exit) -> bool
	{
		if (force_quit && on_exit) on_exit();
		return force_quit;
	};

	g_emu_callbacks.update_emu_settings = []() {};
	g_emu_callbacks.save_emu_settings = []()
	{
		Emulator::SaveSettings(g_cfg.to_string(), Emu.GetTitleID());
	};

	g_emu_callbacks.init_kb_handler = []() { ensure(g_fxo->init<KeyboardHandlerBase, NullKeyboardHandler>(Emu.DeserialManager())); };
	g_emu_callbacks.init_mouse_handler = []() { ensure(g_fxo->init<MouseHandlerBase, NullMouseHandler>(Emu.DeserialManager())); };

	g_emu_callbacks.init_pad_handler = [](std::string_view title_id)
	{
		// pad_thread takes opaque thread/window pointers (Qt only on desktop builds)
		ensure(g_fxo->init<named_thread<pad_thread>>(nullptr, nullptr, title_id));
		while (!pad::g_started) std::this_thread::sleep_for(1ms);
	};

	g_emu_callbacks.init_gs_render = [](utils::serial* ar)
	{
		switch (const video_renderer type = g_cfg.video.renderer)
		{
		case video_renderer::null:
			g_fxo->init<rsx::thread, named_thread<NullGSRender>>(ar);
			break;
		case video_renderer::vulkan:
			g_fxo->init<rsx::thread, named_thread<VKGSRender>>(ar);
			break;
		default:
			fmt::throw_exception("Renderer %s is not available on iOS (use Vulkan/MoltenVK)", type);
		}
	};

	g_emu_callbacks.get_gs_frame = []() -> std::unique_ptr<GSFrameBase>
	{
		ensure(g_view);
		return std::make_unique<ios_gs_frame>(g_view);
	};
	g_emu_callbacks.close_gs_frame = []() {};

	g_emu_callbacks.get_audio = []() -> std::shared_ptr<AudioBackend>
	{
		std::shared_ptr<AudioBackend> result;
		if (g_cfg.audio.renderer == audio_renderer::cubeb) result = std::make_shared<CubebBackend>();
		else result = std::make_shared<NullAudioBackend>();
		if (!result->Initialized()) result = std::make_shared<NullAudioBackend>();
		return result;
	};
	g_emu_callbacks.get_audio_enumerator = [](u64 renderer) -> std::shared_ptr<audio_device_enumerator>
	{
		if (static_cast<audio_renderer>(renderer) == audio_renderer::cubeb) return std::make_shared<cubeb_enumerator>();
		return std::make_shared<null_enumerator>();
	};

	g_emu_callbacks.get_camera_handler = []() -> std::shared_ptr<camera_handler_base> { return std::make_shared<null_camera_handler>(); };
	g_emu_callbacks.get_music_handler  = []() -> std::shared_ptr<music_handler_base>  { return std::make_shared<null_music_handler>(); };

	g_emu_callbacks.get_msg_dialog                 = []() -> std::shared_ptr<MsgDialogBase> { return {}; };
	g_emu_callbacks.get_osk_dialog                 = []() -> std::shared_ptr<OskDialogBase> { return {}; };
	g_emu_callbacks.get_save_dialog                = []() -> std::unique_ptr<SaveDialogBase> { return {}; };
	g_emu_callbacks.get_trophy_notification_dialog = []() -> std::unique_ptr<TrophyNotificationBase> { return {}; };

	g_emu_callbacks.on_run    = [](bool) {};
	g_emu_callbacks.on_pause  = []() {};
	g_emu_callbacks.on_resume = []() {};
	g_emu_callbacks.on_stop   = []() {};
	g_emu_callbacks.on_ready  = []() {};
	g_emu_callbacks.on_missing_fw = []() { ios_log.error("Firmware (PS3UPDAT.PUP) is not installed"); };
	g_emu_callbacks.on_emulation_stop_no_response = [](std::shared_ptr<atomic_t<bool>> ok, int)
	{
		if (!ok || !*ok) report_fatal_error("Stopping emulator took too long.");
	};
	g_emu_callbacks.on_save_state_progress = [](std::shared_ptr<atomic_t<bool>>, stx::shared_ptr<utils::serial>, stx::atomic_ptr<std::string>*, std::shared_ptr<void>) {};
	g_emu_callbacks.enable_disc_eject  = [](bool) {};
	g_emu_callbacks.enable_disc_insert = [](bool) {};
	g_emu_callbacks.handle_taskbar_progress = [](s32, s32) {};
	g_emu_callbacks.get_localized_string    = [](localized_string_id, const char*) -> std::string { return {}; };
	g_emu_callbacks.get_localized_u32string = [](localized_string_id, const char*) -> std::u32string { return {}; };
	g_emu_callbacks.get_localized_setting   = [](const cfg::_base*, u32) -> std::string { return {}; };
	g_emu_callbacks.play_sound = [](const std::string&, std::optional<f32>) {};
	g_emu_callbacks.add_breakpoint = [](u32) {};
	g_emu_callbacks.display_sleep_control_supported = []() { return false; };
	g_emu_callbacks.enable_display_sleep = [](bool) {};
	g_emu_callbacks.check_microphone_permissions = []() {};
	g_emu_callbacks.make_video_source = []() { return nullptr; };
	g_emu_callbacks.get_image_info = [](const std::string&, std::string&, s32&, s32&, s32&) { return false; };
	g_emu_callbacks.get_scaled_image = [](const std::string&, s32, s32, s32&, s32&, u8*, bool) { return false; };
	g_emu_callbacks.get_font_dirs = []() { return std::vector<std::string>{}; };
	g_emu_callbacks.on_install_pkgs = [](const std::vector<std::string>&, bool) { return false; };
	g_emu_callbacks.get_photo_path = [](std::string_view) { return std::string{}; };
	g_emu_callbacks.get_database_config = [](const std::string&) { return std::string{}; };
	g_emu_callbacks.enable_gamemode = [](bool) {};
}

extern "C" void rpcs3_ios_set_view(void* uiview) { g_view = uiview; }

extern "C" void rpcs3_ios_init(const char* data_dir)
{
	if (data_dir && *data_dir) setenv("HOME", data_dir, 1);
	init_ios_logging(data_dir);
	create_ios_callbacks();
	Emu.SetHasGui(false);
	Emu.SetHeadless(false);
	Emu.SetUsr("00000001");
	Emu.Init();
	std::setlocale(LC_NUMERIC, "C");
}

extern "C" bool rpcs3_ios_boot(const char* path)
{
	const game_boot_result r = Emu.BootGame(path, "", true);
	ios_log.notice("BootGame result: %d", static_cast<int>(r));
	return r == game_boot_result::no_errors;
}

extern "C" void rpcs3_ios_stop(void)
{
	Emu.GracefulShutdown(false);
	logs::listener::sync_all();
}

extern "C" void rpcs3_ios_flush_logs(void)
{
	logs::listener::sync_all();
}
