
#include <database.hpp>
#include <filesystem>


namespace nabu {

namespace db {

database::database(std::string const& file_name) noexcept :
	db_{ nullptr }
{
	auto const action = std::filesystem::exists(file_name)
		? "opened"
		: "created";

	auto const code = sqlite3_open(file_name.c_str(), &db_);
	if (code != SQLITE_OK) {
		logger.error("Error while database was {} : {}", action, sqlite3_errmsg(db_));
		std::terminate();
	}

	logger.debug("database {}", action);
}

database::~database() {
	sqlite3_close(db_);
}

bool database::has_table(std::string const& name) {
	auto const command = fmt::format(
		"select name from sqlite_master "
		"where type = 'table' "
		"and   name = '{}' "
		"limit 1",
		name);

	auto found = false;
	execute(command, [&](int, char**, char**) { found = true; });
	return found;
}

} // db

NABU_IMPLEMENT_GLOBAL(database, "nabu_data.db");

} // nabu
