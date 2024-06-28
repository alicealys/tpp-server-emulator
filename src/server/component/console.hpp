#pragma once

namespace console
{
	enum console_type
	{
		con_type_print = 0,
		con_type_error = 1,
		con_type_debug = 2,
		con_type_warning = 3,
		con_type_info = 4
	};

	void dispatch_print(const int type, const char* fmt, ...);

	template <typename... Args>
	void print(const char* fmt, Args&&... args)
	{
		dispatch_print(con_type_print, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void log(const char* fmt, Args&&... args)
	{
		dispatch_print(con_type_info, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void warning(const char* fmt, Args&&... args)
	{
		dispatch_print(con_type_warning, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void error(const char* fmt, Args&&... args)
	{
		dispatch_print(con_type_error, fmt, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void debug([[maybe_unused]] const char* fmt, [[maybe_unused]] Args&&... args)
	{
#ifdef DEBUG
		dispatch_print(con_type_debug, fmt, std::forward<Args>(args)...);
#endif
	}
}
