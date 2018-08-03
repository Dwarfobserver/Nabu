
#pragma once

#include <logger.hpp>
#include <sqlite3.h>
#include <stdexcept>


namespace nabu {

// Errors when interacting with the database.
class database_error : public std::logic_error {
public:
    explicit database_error(std::string const& str) :
        std::logic_error{ str } {}
};

// Error when executing a SQL query.
class sql_error : public database_error {
public:
    sql_error(std::string const& command, std::string const& str) :
        database_error{ "SQL command '" + command + "' failed with the message '" + str + "'" } {}
};

namespace db {

// Base class which is inherited along with mixins.
class database {
public:
	explicit database(std::string const& file_name) noexcept;
	database(database&& db) = delete;
	~database();

	bool has_table(std::string const& name);

	template <class F>
	void execute(std::string const& command, F&& callback);

	void execute(char const* command) {
		execute(command, [](int, char**, char**) {});
	}
private:
	sqlite3 * db_;

	template <class F>
	static int sqlite_callback(void* cb_ptr, int nb_fields, char** values, char** columns);
};

template <class F>
void database::execute(std::string const& command, F&& callback) {

	constexpr auto exec_cb = sqlite_callback<std::remove_reference_t<F>>;

	char* error_msg = nullptr;
	auto const code = sqlite3_exec(db_, command, exec_cb, &callback, &error_msg);

	if (code != SQLITE_OK) {
		auto const excetion_msg = std::string{ error_msg };
		sqlite3_free(error_msg);
		throw sql_error{ command, excetion_msg };
	}
}

template <class F>
int database::sqlite_callback(
		void*  const cb_ptr,
		int    const nb_fields,
		char** const values,
		char** const columns)
{
	auto& cb = *static_cast<F*>(cb_ptr);
	cb(nb_fields, values, columns);
	return SQLITE_OK;
}

} // db

} // nabu
