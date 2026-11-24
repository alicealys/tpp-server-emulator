#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include "../server.hpp"

#include "database/database.hpp"
#include "database/auth.hpp"

#include "database/models/players.hpp"
#include "database/models/player_data.hpp"
#include "database/models/steam_users.hpp"

#include "scripting/engine.hpp"

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

	int params::get_int(const size_t index) const
	{
		if (this->tokens_.size() <= index)
		{
			return 0;
		}

		return std::atoi(this->tokens_[index].data());
	}

	float params::get_float(const size_t index) const
	{
		if (this->tokens_.size() <= index)
		{
			return 0;
		}

		return static_cast<float>(std::atof(this->tokens_[index].data()));
	}

	std::uint64_t params::get_uint64(const size_t index) const
	{
		if (this->tokens_.size() <= index)
		{
			return 0;
		}

		return std::strtoull(this->tokens_[index].data(), nullptr, 0);
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

			if (i < this->size() - 1)
			{
				buffer.append(" ");
			}
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

	void add(const std::string& name, const callback_narg& cb)
	{
		add(name, [cb](const command::params&)
		{
			cb();
		});
	}

	std::optional<std::string> find_command_name(const std::string& input)
	{
		if (input.size() < 2)
		{
			return {};
		}

		const auto lower = utils::string::to_lower(input);

		for (const auto& [name, command] : commands)
		{
			if (name.starts_with(lower))
			{
				return {name};
			}
		}

		return {};
	}

	class component final : public component_interface
	{
	public:
		void post_start() override
		{
			add("quit", [](const params& params)
			{
				emulator::stop_server();
			});

			add("max_resources", [](const params& params)
			{
				if (params.size() < 2)
				{
					return;
				}

				const auto arg = params.get(1);
				const auto player_id = std::strtoull(arg.data(), nullptr, 10);
				database::player_data::resource_arrays_t resource_arrays{};

				for (auto i = 0; i < game::RESOURCE_TYPE_COUNT; i++)
				{
					resource_arrays[game::processed_server][i] = game::server_processed_resource_caps[i];
					resource_arrays[game::unprocessed_server][i] = game::server_unprocessed_resource_caps[i];
					resource_arrays[game::processed_local][i] = game::local_processed_resource_caps[i];
					resource_arrays[game::unprocessed_local][i] = game::local_processed_resource_caps[i];
				}

				database::player_data::set_resources_gmp(player_id, resource_arrays, database::vars.max_local_gmp, database::vars.max_server_gmp);
			});

			add("query", [](const params& params)
			{
				if (database::get_database_type() != database::database_mysql)
				{
					console::print("this command is only supported for mysql");
					return;
				}

#ifdef MYSQL_SUPPORTED
				const auto query = params.join(1);
				const auto start = std::chrono::high_resolution_clock::now();
				database::access([&](const database::database_t& db)
				{
					const auto handle = db.get_mysql()->get_handle();

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
							for (auto i = 0u; i < max_field_lengths[f] + 2; i++)
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

								for (auto i = 0u; i < diff; i++)
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

							for (auto i = 0u; i < result->row_count; i++)
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
#endif
			});

			add("reload_lists", auth::reload_lists);

			add("reload_scripts", emulator::scripting::reload);

			add("update_has_fob", []()
			{
				const auto count = static_cast<std::uint32_t>(database::players::get_player_count());
				const auto players = database::players::get_player_list(count);
				for (auto& player : players)
				{
					auto has_an_fob = false;
					const auto fobs = database::fobs::get_fob_list(player.get_id());
					for (auto& fob : fobs)
					{
						const auto& clusters = fob.get_cluster_param();
						for (auto i = 0; i < game::fob_clusters_count; i++)
						{
							if (clusters.param->build.fields.platform_count != 0)
							{
								has_an_fob = true;
								break;
							}
						}
					}

					database::player_records::set_has_fob(player.get_id(), has_an_fob);
				}
			});

			add("delete_user", [](const params& params)
			{
				const auto arg = params.get(1);
				const auto account_id = std::strtoull(arg.data(), nullptr, 10);
				if (!database::steam_users::delete_all_user_data(account_id))
				{
					console::warning("user %lli not found\n", account_id);
					return;
				}

				console::print("deleted user %lli\n", account_id);
			});

			add("list_player_resources", [](const params& params)
			{
				if (params.size() < 2)
				{
					console::print("usage: list_player_resources <player_id>\n");
					return;
				}

				const auto player_id = params.get_uint64(1);

				database::player_data::resource_arrays_t resource_arrays{};
				if (!database::player_data::get_resource_arrays(player_id, resource_arrays))
				{
					console::print("player not found\n");
					return;
				}

				console::print("---------- usable resources ----------\n");
				for (auto i = 0; i < game::RESOURCE_TYPE_COUNT; i++)
				{
					console::print("%02i: %-28s: %i\n", i, game::resource_type_names[i].data(), resource_arrays[game::processed_server][i]);
				}

				console::print("-------- processing resources --------\n");
				for (auto i = 0; i <= game::PRECIOUS_METAL; i++)
				{
					console::print("%02i: %-28s: %i\n", i, game::resource_type_names[i].data(), resource_arrays[game::unprocessed_server][i]);
				}
			});

			add("find_player", [](const params& params)
			{
				if (params.size() < 2)
				{
					console::print("usage: find_player <steam_id>\n");
					return;
				}

				const auto steam_id = params.get_uint64(1);
				const auto player = database::players::find_from_account(steam_id);

				if (!player.has_value())
				{
					console::print("player not found\n");
					return;
				}

				console::print("id: %lli\n", player->get_id());
				console::print("account_id: %lli\n", player->get_account_id());
				console::print("creation_time: %lli\n", player->get_creation_time().count());
				console::print("last_update: %lli\n", player->get_last_update().count());
				console::print("online: %i\n", !player->is_session_expired());
			});

			add("set_player_resource_usable", [](const params& params)
			{
				if (params.size() < 3)
				{
					console::print("usage: set_player_resource_usable <player_id> <resource_id> <value>\n");
					return;
				}

				const auto player_id = params.get_uint64(1);
				const auto resource_id = params.get_int(2);
				const auto value = params.get_int(3);

				if (resource_id >= game::RESOURCE_TYPE_COUNT)
				{
					console::print("invalid resource id\n");
					return;
				}

				database::player_data::resource_arrays_t resource_arrays{};
				if (!database::player_data::get_resource_arrays(player_id, resource_arrays))
				{
					console::print("player not found\n");
					return;
				}

				const auto capped = game::cap_resource_value(game::processed_server, resource_id, value);
				const auto old_value = resource_arrays[game::unprocessed_server][resource_id];
				resource_arrays[game::processed_server][resource_id] = capped;
				database::player_data::set_resources(player_id, resource_arrays);
				console::print("changed resource %s: %i -> %i\n", game::resource_type_names[resource_id].data(), old_value, capped);
			});

			add("set_player_resource_processing", [](const params& params)
			{
				if (params.size() < 3)
				{
					console::print("usage: set_player_resource_processing <player_id> <resource_id> <value>\n");
					return;
				}

				const auto player_id = params.get_uint64(1);
				const auto resource_id = params.get_int(2);
				const auto value = params.get_int(3);

				if (resource_id >= game::RESOURCE_TYPE_COUNT)
				{
					console::print("invalid resource id\n");
					return;
				}

				database::player_data::resource_arrays_t resource_arrays{};
				if (!database::player_data::get_resource_arrays(player_id, resource_arrays))
				{
					console::print("player not found\n");
					return;
				}

				const auto capped = game::cap_resource_value(game::unprocessed_server, resource_id, value);
				const auto old_value = resource_arrays[game::unprocessed_server][resource_id];
				resource_arrays[game::unprocessed_server][resource_id] = capped;
				database::player_data::set_resources(player_id, resource_arrays);
				console::print("changed resource %s: %i -> %i\n", game::resource_type_names[resource_id].data(), old_value, capped);
			});

			add("list_nukes", []()
			{
				const auto players = database::player_data::find_all_with_nukes();

				for (auto i = 0ull; i < players.size(); i++)
				{
					const auto& player = players[i].first;
					const auto count = players[i].second;
					console::print("%lli: player_id: %lli account_id: %lli: %i\n", i, player.get_id(), player.get_account_id(), count);
				}
			});
		}
	};
}

REGISTER_COMPONENT(command::component)
