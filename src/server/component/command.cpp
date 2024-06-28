#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include "../server.hpp"

#include "database/database.hpp"

#include <utils/io.hpp>
#include <utils/string.hpp>

namespace command
{
	namespace
	{
		std::unordered_map<std::string, callback> commands;

		std::mutex queue_mutex;
		std::vector<std::string> command_queue;
	}

	params::params(const std::vector<std::string>& tokens)
		: tokens_(tokens)
	{
	}

	std::string params::get(const size_t index) const
	{
		if (this->tokens_.size() <= index)
		{
			return {};
		}

		return this->tokens_[index];
	}

	std::string params::operator[](const size_t index) const
	{
		return this->get(index);
	}

	size_t params::size() const
	{
		return this->tokens_.size();
	}

	std::string params::join(const size_t index) const
	{
		std::string buffer;

		for (auto i = index; i < this->size(); i++)
		{
			buffer.append(this->tokens_[i]);
			buffer.append(" ");
		}

		return buffer;
	}

	void execute_single(const std::string& cmd)
	{
		const auto args = utils::string::split(cmd, ' ');
		if (args.size() == 0)
		{
			return;
		}

		const auto name = utils::string::to_lower(args[0]);
		const auto command = commands.find(name);
		if (command == commands.end())
		{
			console::warning("Command \"%s\" not found\n", name.data());
			return;
		}

		try
		{
			command->second(args);
		}
		catch (const std::exception& e)
		{
			console::error("Error executing command: %s\n", e.what());
		}
	}

	void run_frame()
	{
		std::vector<std::string> queue_copy;

		{
			std::lock_guard _0(queue_mutex);
			queue_copy = command_queue;
			command_queue.clear();
		}

		for (const auto& cmd : queue_copy)
		{
			execute_single(cmd);
		}
	}

	void execute(const std::string& cmd)
	{
		std::lock_guard _0(queue_mutex);
		command_queue.emplace_back(cmd);
	}

	void add(const std::string& name, const callback& cb)
	{
		console::log("Registering console command \"%s\"\n", name.data());
		commands.insert(std::make_pair(name, cb));
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			add("quit", [](const params& params)
			{
				tpp::stop_server();
			});

			add("query", [](const params& params)
			{
				const auto query = params.join(1);
				const auto start = std::chrono::high_resolution_clock::now();
				database::access([&](const database::database_t& db)
				{
					const auto handle = db->get_handle();

					console::print("> %s\n", query.data());

					if (mysql_query(handle, query.data()) != 0)
					{
						console::error("ERROR %i: %s\n", mysql_errno(handle), mysql_error(handle));
						return;
					}

					const auto end = std::chrono::high_resolution_clock::now();
					const auto secs = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) / 1000.f;

					const auto result = mysql_store_result(handle);
					const auto _0 = gsl::finally([&]
					{
						mysql_free_result(result);
					});

					const auto affected_rows = mysql_affected_rows(handle);

					if (result != nullptr)
					{
						const auto num_rows = mysql_num_rows(result);
						const auto num_fields = mysql_num_fields(result);
						const auto fields = mysql_fetch_fields(result);

						std::vector<std::size_t> max_field_lengths;
						std::vector<std::string> fields_str;
						std::vector<std::vector<std::string>> rows;

						rows.resize(num_rows);
						max_field_lengths.resize(num_fields);
						fields_str.resize(num_fields);

						for (auto f = 0u; f < num_fields; f++)
						{
							const auto field = &fields[f];
							const std::string field_str = {field->name, field->name_length};
							fields_str[f] = field_str;
							max_field_lengths[f] = std::max(max_field_lengths[f], field_str.size());
						}

						for (auto i = 0u; i < num_rows; i++)
						{
							const auto row = mysql_fetch_row(result);
							const auto lengths = mysql_fetch_lengths(result);
							rows[i].resize(num_fields);

							for (auto f = 0u; f < num_fields; f++)
							{
								const auto field = &fields[f];

								const std::string field_str = {field->name, field->name_length};
								const std::string row_str = {row[f], lengths[f]};

								rows[i][f] = row_str;
								max_field_lengths[f] = std::max(max_field_lengths[f], row_str.size());
							}
						}

						auto line_length = static_cast<std::size_t>(num_fields) * 3 + 1;
						for (const auto& max_length : max_field_lengths)
						{
							line_length += max_length;
						}

						std::string separator;
						separator.reserve(line_length);

						for (auto f = 0u; f < num_fields; f++)
						{
							separator.push_back('+');
							for (auto i = 0; i < max_field_lengths[f] + 2; i++)
							{
								separator.push_back('-');
							}

							if (f == num_fields - 1)
							{
								separator.push_back('-');
							}
						}

						const auto print_separator = [&]()
						{
							console::print("%s\n", separator.data());
						};

						const auto print_values = [&](const std::vector<std::string> values)
						{
							std::string row_buffer;
							row_buffer.reserve(line_length);

							for (auto f = 0u; f < num_fields; f++)
							{
								row_buffer.append("| ");
								const auto& field_value = values[f];
								const auto diff = max_field_lengths[f] - field_value.size();

								row_buffer.append(field_value);

								for (auto i = 0; i < diff; i++)
								{
									row_buffer.push_back(' ');
								}

								row_buffer.push_back(' ');

								if (f == num_fields - 1)
								{
									row_buffer.push_back('|');
								}
							}

							console::print("%s\n", row_buffer.data());
						};

						if (result->row_count > 0)
						{
							print_separator();
							print_values(fields_str);
							print_separator();

							for (auto i = 0; i < result->row_count; i++)
							{
								print_values(rows[i]);
							}

							print_separator();

							console::print("%i rows in set (%.04f sec)", result->row_count, secs);
						}
					}
					else if (affected_rows > 0)
					{
						console::print("Query OK, %i row affected (%.04f sec)", affected_rows, secs);
					}
				});
			});
		}
	};
}

REGISTER_COMPONENT(command::component)
