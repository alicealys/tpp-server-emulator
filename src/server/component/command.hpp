#pragma once

namespace command
{
	class params
	{
	public:
		params(const std::vector<std::string>& tokens);

		std::string get(const size_t index) const;
		std::string operator[](const size_t index) const;

		size_t size() const;

		std::string join(const size_t index) const;

	private:
		std::vector<std::string> tokens_;

	};

	using callback = std::function<void(const params& args)>;

	void run_frame();

	void execute(const std::string& cmd);
	void add(const std::string& name, const callback& cb);
}
