#pragma once

#include <string>
#include <optional>
#include <future>

namespace utils::http
{
	using headers = std::unordered_map<std::string, std::string>;

	struct http_result
	{
		std::string buffer;
		std::uint32_t code;
		std::uint32_t response_code;
	};

	std::optional<std::string> get_data(const std::string& url, const headers& headers = {}, 
		const std::function<void(size_t, size_t, size_t)>& callback = {});
	std::optional<http_result> post_data(const std::string& url, const std::string& data,
		const headers& headers = {}, const std::function<void(size_t, size_t, size_t)>& callback = {});

	std::future<std::optional<std::string>> get_data_async(const std::string& url, const headers& headers = {});
}
