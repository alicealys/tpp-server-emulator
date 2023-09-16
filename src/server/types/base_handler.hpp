#pragma once

namespace tpp
{
	template <typename HandlerType>
	class base_handler
	{
	public:
		template <typename T, typename... Args>
		void register_handler(const std::string& endpoint, Args&&... args)
		{
			auto handler = std::make_unique<T>(std::forward<Args>(args)...);
			this->handlers_.insert(std::make_pair(endpoint, std::move(handler)));
		}

	protected:
		std::unordered_map<std::string, std::unique_ptr<HandlerType>> handlers_;
	};
}
