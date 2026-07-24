#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "console.hpp"
#include "command.hpp"

#include <cstdarg>

#ifndef _WIN32
#include <sys/poll.h>
#include <sys/eventfd.h>
#include <sys/ioctl.h>

#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

namespace console
{
	namespace
	{
		constexpr auto max_console_history = 100;

		struct
		{
			std::atomic_bool kill;
			std::thread thread;
			int pipe_fd[2];
			char buffer[512]{};
			int cursor;
			std::deque<std::string> history;
			std::int32_t history_index = -1;
		} con{};

		std::string format(va_list* ap, const char* message)
		{
			static thread_local char buffer[0x1000];

			const auto count = vsnprintf(buffer, sizeof(buffer), message, *ap);
			if (count < 0)
			{
				return {};
			}

			return {buffer, static_cast<size_t>(count)};
		}

		void set_cursor_pos(int x)
		{
			winsize ws{};

			if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0)
			{
				printf("\033[%i;%iH", ws.ws_row, x + 1);
			}
		}

		void show_cursor(const bool show)
		{
			if (show)
			{
				printf("\033[?25h");
			}
			else
			{
				printf("\033[?25l");
			}
		}

		const char* get_attribute(const int type)
		{
			switch (type)
			{
			case con_type_info:
				return "\e[0;37m"; // white
			case con_type_warning:
				return "\e[0;33m"; // yellow
			case con_type_error:
				return "\e[0;31m"; // red
			case con_type_debug:
				return "\e[0;36m"; // cyan
			}

			return "\e[0;37m";
		}

		auto& get_print_mutex()
		{
			static std::recursive_mutex print_mutex;
			return print_mutex;
		}

		void update()
		{
			std::lock_guard _0(get_print_mutex());

			show_cursor(false);
			set_cursor_pos(0);
			printf("%s", con.buffer);
			set_cursor_pos(con.cursor);
			show_cursor(true);
		}

		void clear_output()
		{
			std::lock_guard _0(get_print_mutex());

			show_cursor(false);
			set_cursor_pos(0);

			for (auto i = 0u; i < std::strlen(con.buffer); i++)
			{
				printf(" ");
			}

			set_cursor_pos(con.cursor);
			show_cursor(true);
		}

		int dispatch_message(const int type, const std::string& message)
		{
			std::lock_guard _0(get_print_mutex());

			clear_output();
			set_cursor_pos(0);

			printf("%s", get_attribute(type));
			const auto res = printf("%s", message.data());
			printf("%s", get_attribute(con_type_info));

			if (message.size() <= 0 || message[message.size() - 1] != '\n')
			{
				printf("\n");
			}

			update();
			return res;
		}

		void clear()
		{
			std::lock_guard _0(get_print_mutex());

			clear_output();
			strncpy(con.buffer, "", sizeof(con.buffer));

			con.cursor = 0;
			set_cursor_pos(0);
		}

		size_t get_max_input_length()
		{
			winsize ws{};

			if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != 0)
			{
				return 0;
			}

			const auto columns = static_cast<size_t>(ws.ws_xpixel - 1);
			return std::max(size_t(0), std::min(columns, sizeof(con.buffer)));
		}

		void handle_ansi(int c)
		{
			std::lock_guard _0(get_print_mutex());

			switch (c)
			{
			case 0x41:
			{
				if (++con.history_index >= static_cast<int>(con.history.size()))
				{
					con.history_index = static_cast<int>(con.history.size()) - 1;
				}

				clear();

				if (con.history_index != -1)
				{
					strncpy(con.buffer, con.history.at(con.history_index).data(), sizeof(con.buffer));
					con.cursor = static_cast<int>(strlen(con.buffer));
				}

				update();
				break;
			}
			case 0x42:
			{
				if (--con.history_index < -1)
				{
					con.history_index = -1;
				}

				clear();

				if (con.history_index != -1)
				{
					strncpy(con.buffer, con.history.at(con.history_index).data(), sizeof(con.buffer));
					con.cursor = static_cast<int>(strlen(con.buffer));
				}

				update();
				break;
			}
			case 0x44:
			{
				if (con.cursor > 0)
				{
					con.cursor--;
					set_cursor_pos(con.cursor);
				}

				break;
			}
			case 0x43:
			{
				if (con.cursor < static_cast<int>(std::strlen(con.buffer)))
				{
					con.cursor++;
					set_cursor_pos(con.cursor);
				}

				break;
			}
			}
		}

		bool handle_input(int c)
		{
			std::lock_guard _0(get_print_mutex());

			switch (c)
			{
			case 0x1B:
				return false;
			case 0xA:
			{
				if (con.history_index != -1)
				{
					const auto itr = con.history.begin() + con.history_index;

					if (*itr == con.buffer)
					{
						con.history.erase(con.history.begin() + con.history_index);
					}
				}

				if (con.buffer[0])
				{
					con.history.push_front(con.buffer);
				}

				if (con.history.size() > max_console_history)
				{
					con.history.erase(con.history.begin() + max_console_history);
				}

				con.history_index = -1;

				command::execute(con.buffer);

				con.cursor = 0;

				clear_output();
				strncpy(con.buffer, "", sizeof(con.buffer));
				break;
			}
			case 0x7F:
			{
				if (con.cursor <= 0)
				{
					break;
				}

				clear_output();

				std::memmove(con.buffer + con.cursor - 1, con.buffer + con.cursor,
					strlen(con.buffer) + 1 - con.cursor);
				con.cursor--;

				update();
				break;
			}
			default:
			{
				const auto c_ = static_cast<char>(c);
				if (c_ == 0 || std::strlen(con.buffer) + 1 >= get_max_input_length())
				{
					return false;
				}

				std::memmove(con.buffer + con.cursor + 1,
					con.buffer + con.cursor, std::strlen(con.buffer) + 1 - con.cursor);
				con.buffer[con.cursor] = c_;
				con.cursor++;

				update();
			}
			}

			return true;
		}

		void handle_escape()
		{
			std::lock_guard _0(get_print_mutex());
			con.cursor = 0;
			clear_output();
			strncpy(con.buffer, "", sizeof(con.buffer));
		}

		std::string get_prefix(const int type)
		{
			switch (type)
			{
			case con_type_info:
				return "[*] ";
			case con_type_warning:
				return "[!] ";
			case con_type_error:
				return "[-] ";
			case con_type_debug:
				return "[+] ";
			}

			return {};
		}

		template <typename F>
		void stdin_iter(F&& cb)
		{
			auto c = getchar();
			auto res = true;
			while (c != EOF && c != '\n')
			{
				if (res)
				{
					res = cb(c);
				}
				c = getchar();
			}
		}

		bool null_iter(int)
		{
			return true;
		}

		void console_thread()
		{
			termios old_attr{};
			termios new_attr{};

			tcgetattr(STDIN_FILENO, &old_attr);
			new_attr = old_attr;
			new_attr.c_lflag &= ~(ICANON | ECHO);
			tcsetattr(STDIN_FILENO, TCSANOW, &new_attr);

			auto flags = fcntl(STDIN_FILENO, F_GETFL, 0);
			fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

			pollfd handles[2]{};

			handles[0].fd = STDIN_FILENO;
			handles[0].events = POLLIN;

			handles[1].fd = con.pipe_fd[0];
			handles[1].events = POLLIN;

			while (!con.kill)
			{
				auto res = poll(handles, 2, -1);
				if (res < 0 || con.kill)
				{
					break;
				}

				auto c = getchar();

				if (c == 0x1B)
				{
					if (getchar() != '[')
					{
						handle_escape();
						stdin_iter(null_iter);
						continue;
					}

					handle_ansi(getchar());
					stdin_iter(null_iter);
				}
				else
				{
					handle_input(c);
					stdin_iter(handle_input);
				}

				fflush(stdout);
			}

			tcsetattr(STDIN_FILENO, TCSANOW, &old_attr);
		}

		void send_kill_event()
		{
			auto data = 1;
			write(con.pipe_fd[1], &data, sizeof(data));
		}


		void sigint_handler(int)
		{
			command::execute("quit");
		}

		void set_sigint_handler()
		{
			struct sigaction sa {};
			sa.sa_handler = sigint_handler;
			sigaction(SIGINT, &sa, NULL);
		}
	}

	void dispatch_print(const int type, const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		const auto result = format(&ap, fmt);
		va_end(ap);

		const auto text = std::format("{}{}", get_prefix(type), result);
		dispatch_message(type, text);
	}

	class component final : public component_interface
	{
	public:
		void pre_start() override
		{
			printf("\033]0;%s\007", "tpp-server-emulator");

			set_sigint_handler();
			pipe(con.pipe_fd);

			con.thread = std::thread(console_thread);
		}

		void pre_destroy() override
		{
			con.kill = true;
			send_kill_event();

			if (con.thread.joinable())
			{
				con.thread.join();
			}
		}
	};
}

REGISTER_COMPONENT(console::component)
#else
#define OUTPUT_HANDLE GetStdHandle(STD_OUTPUT_HANDLE)

namespace console
{
	namespace
	{
		constexpr auto max_console_history = 100;

		struct
		{
			std::atomic_bool kill;
			std::thread thread;
			HANDLE kill_event;
			char buffer[512]{};
			int cursor;
			std::deque<std::string> history;
			std::int32_t history_index = -1;
		} con{};

		void set_cursor_pos(int x)
		{
			CONSOLE_SCREEN_BUFFER_INFO info{};
			GetConsoleScreenBufferInfo(OUTPUT_HANDLE, &info);
			info.dwCursorPosition.X = static_cast<short>(x);
			SetConsoleCursorPosition(OUTPUT_HANDLE, info.dwCursorPosition);
		}

		void show_cursor(const bool show)
		{
			CONSOLE_CURSOR_INFO info{};
			GetConsoleCursorInfo(OUTPUT_HANDLE, &info);
			info.bVisible = show;
			SetConsoleCursorInfo(OUTPUT_HANDLE, &info);
		}

		std::string format(va_list* ap, const char* message)
		{
			static thread_local char buffer[0x1000];

			const auto count = _vsnprintf_s(buffer, sizeof(buffer), sizeof(buffer), message, *ap);
			if (count < 0)
			{
				return {};
			}

			return {buffer, static_cast<size_t>(count)};
		}

		uint8_t get_attribute(const int type)
		{
			switch (type)
			{
			case con_type_info:
				return 7; // white
			case con_type_warning:
				return 6; // yellow
			case con_type_error:
				return 4; // red
			case con_type_debug:
				return 3; // cyan
			}

			return 7;
		}

		auto& get_print_mutex()
		{
			static std::recursive_mutex print_mutex;
			return print_mutex;
		}

		void update()
		{
			std::lock_guard _0(get_print_mutex());

			show_cursor(false);
			set_cursor_pos(0);
			printf("%s", con.buffer);
			set_cursor_pos(con.cursor);
			show_cursor(true);
		}

		void clear_output()
		{
			std::lock_guard _0(get_print_mutex());

			show_cursor(false);
			set_cursor_pos(0);

			for (auto i = 0; i < std::strlen(con.buffer); i++)
			{
				printf(" ");
			}

			set_cursor_pos(con.cursor);
			show_cursor(true);
		}

		int dispatch_message(const int type, const std::string& message)
		{
			std::lock_guard _0(get_print_mutex());

			clear_output();
			set_cursor_pos(0);

			SetConsoleTextAttribute(OUTPUT_HANDLE, get_attribute(type));
			const auto res = printf("%s", message.data());
			SetConsoleTextAttribute(OUTPUT_HANDLE, get_attribute(con_type_info));

			if (message.size() <= 0 || message[message.size() - 1] != '\n')
			{
				printf("\n");
			}

			update();
			return res;
		}

		void clear()
		{
			std::lock_guard _0(get_print_mutex());

			clear_output();
			strncpy_s(con.buffer, "", sizeof(con.buffer));

			con.cursor = 0;
			set_cursor_pos(0);
		}

		size_t get_max_input_length()
		{
			CONSOLE_SCREEN_BUFFER_INFO info{};
			GetConsoleScreenBufferInfo(OUTPUT_HANDLE, &info);
			const auto columns = static_cast<size_t>(info.srWindow.Right - info.srWindow.Left - 1);
			return std::max(size_t(0), std::min(columns, sizeof(con.buffer)));
		}

		void handle_resize()
		{
			clear();
			update();
		}

		void handle_input(const INPUT_RECORD record)
		{
			if (record.EventType == WINDOW_BUFFER_SIZE_EVENT)
			{
				handle_resize();
				return;
			}

			if (record.EventType != KEY_EVENT || !record.Event.KeyEvent.bKeyDown)
			{
				return;
			}

			std::lock_guard _0(get_print_mutex());

			const auto key = record.Event.KeyEvent.wVirtualKeyCode;
			switch (key)
			{
			case VK_UP:
			{
				if (++con.history_index >= con.history.size())
				{
					con.history_index = static_cast<int>(con.history.size()) - 1;
				}

				clear();

				if (con.history_index != -1)
				{
					strncpy_s(con.buffer, con.history.at(con.history_index).data(), sizeof(con.buffer));
					con.cursor = static_cast<int>(strlen(con.buffer));
				}

				update();
				break;
			}
			case VK_DOWN:
			{
				if (--con.history_index < -1)
				{
					con.history_index = -1;
				}

				clear();

				if (con.history_index != -1)
				{
					strncpy_s(con.buffer, con.history.at(con.history_index).data(), sizeof(con.buffer));
					con.cursor = static_cast<int>(strlen(con.buffer));
				}

				update();
				break;
			}
			case VK_LEFT:
			{
				if (con.cursor > 0)
				{
					con.cursor--;
					set_cursor_pos(con.cursor);
				}

				break;
			}
			case VK_RIGHT:
			{
				if (con.cursor < std::strlen(con.buffer))
				{
					con.cursor++;
					set_cursor_pos(con.cursor);
				}

				break;
			}
			case VK_RETURN:
			{
				if (con.history_index != -1)
				{
					const auto itr = con.history.begin() + con.history_index;

					if (*itr == con.buffer)
					{
						con.history.erase(con.history.begin() + con.history_index);
					}
				}

				if (con.buffer[0])
				{
					con.history.push_front(con.buffer);
				}

				if (con.history.size() > max_console_history)
				{
					con.history.erase(con.history.begin() + max_console_history);
				}

				con.history_index = -1;

				command::execute(con.buffer);

				con.cursor = 0;

				clear_output();
				strncpy_s(con.buffer, "", sizeof(con.buffer));
				break;
			}
			case VK_BACK:
			{
				if (con.cursor <= 0)
				{
					break;
				}

				clear_output();

				std::memmove(con.buffer + con.cursor - 1, con.buffer + con.cursor,
					strlen(con.buffer) + 1 - con.cursor);
				con.cursor--;

				update();
				break;
			}
			case VK_ESCAPE:
			{
				con.cursor = 0;
				clear_output();
				strncpy_s(con.buffer, "", sizeof(con.buffer));
				break;
			}
			default:
			{
				const auto c = record.Event.KeyEvent.uChar.AsciiChar;
				if (!c)
				{
					break;
				}

				if (std::strlen(con.buffer) + 1 >= get_max_input_length())
				{
					break;
				}

				std::memmove(con.buffer + con.cursor + 1,
					con.buffer + con.cursor, std::strlen(con.buffer) + 1 - con.cursor);
				con.buffer[con.cursor] = c;
				con.cursor++;

				update();
				break;
			}
			}
		}

		BOOL WINAPI console_ctrl_handler(DWORD ctrl_type)
		{
			if (ctrl_type == CTRL_CLOSE_EVENT)
			{
				command::execute("quit");
				while (!con.kill)
				{
					std::this_thread::sleep_for(10ms);
				}

				return TRUE;
			}

			return FALSE;
		}

		std::string get_prefix(const int type)
		{
			switch (type)
			{
			case con_type_info:
				return "[*] ";
			case con_type_warning:
				return "[!] ";
			case con_type_error:
				return "[-] ";
			case con_type_debug:
				return "[+] ";
			}

			return {};
		}
	}

	void dispatch_print(const int type, const char* fmt, ...)
	{
		va_list ap;
		va_start(ap, fmt);
		const auto result = format(&ap, fmt);
		va_end(ap);

		const auto text = std::format("{}{}", get_prefix(type), result);
		dispatch_message(type, text);
	}

	class component final : public component_interface
	{
	public:
		void pre_start() override
		{
			SetConsoleTitle("tpp-server-emulator");

			SetConsoleCtrlHandler(console_ctrl_handler, TRUE);

			con.kill_event = CreateEvent(NULL, TRUE, FALSE, NULL);

			con.thread = std::thread([]()
			{
				const auto handle = GetStdHandle(STD_INPUT_HANDLE);
				HANDLE handles[2] = {handle, con.kill_event};
				MSG msg{};

				INPUT_RECORD record{};
				DWORD num_events{};

				while (!con.kill)
				{
					const auto result = MsgWaitForMultipleObjects(2, handles, FALSE, INFINITE, QS_ALLINPUT);
					if (con.kill)
					{
						return;
					}

					switch (result)
					{
					case WAIT_OBJECT_0:
					{
						if (!ReadConsoleInput(handle, &record, 1, &num_events) || num_events == 0)
						{
							break;
						}

						handle_input(record);
						break;
					}
					case WAIT_OBJECT_0 + 1:
					{
						if (!PeekMessageA(&msg, GetConsoleWindow(), NULL, NULL, PM_REMOVE))
						{
							break;
						}

						TranslateMessage(&msg);
						DispatchMessage(&msg);
						break;
					}
					}
				}
			});
		}

		void pre_destroy() override
		{
			con.kill = true;
			SetEvent(con.kill_event);

			if (con.thread.joinable())
			{
				con.thread.join();
			}
		}
	};
}

REGISTER_COMPONENT(console::component)
#endif
