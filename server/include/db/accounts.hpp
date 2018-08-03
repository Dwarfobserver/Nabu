
#pragma once

#include <db/core.hpp>
#include <thread_pool.hpp>
#include <optional>


namespace nabu::db {

// Represents a player account.
struct account {
    std::string name;
    std::string password;
};

// Mixin adding two asynchronous functions to nabu::database.
template <class Derived>
class accounts_table {
public:
	// Callback do not take arguments.
	template <class Callback>
	void add_account(account const& acc, Callback&& f);

	// Callback receives an std::optional<nabu::db::account>.
	template <class Callback>
	void try_get_account(std::string const& name, Callback&& f);
protected:
	accounts_table();
private:
	using Derived::self;
};

template <class Derived>
accounts_table<Derived>::accounts_table() {
	if (self().has_table("accounts")) return;

	self().execute("create table accounts ("
		"name     TEXT primary key not null,"
		"password TEXT not null "
		")");
	logger.debug("accounts table created");
}

template <class Derived>
template <class Callback>
void accounts_table<Derived>::add_account(account const& acc, Callback&& f) {
	thread_pool.post([this, acc] {
		auto const sql = fmt::format("insert into accounts values ('{}', '{}')", acc.name, acc.password);
		self().execute(sql.c_str());
	}, std::forward<Callback>(f);
}

template <class Derived>
template <class Callback>
void accounts_table<Derived>::try_get_account(std::string const& name, Callback&& f) {
	thread_pool.post([this, name] {
		std::optional<account> opt;
		auto const sql = fmt::format("select name, password from accounts where name = '{}'", name);
		self().execute(sql.c_str(), [&](int, char** values, char**) {
			opt = { values[0], values[1] };
		});
		return opt;
	}, std::forward<Callback>(f));
}

} // nabu::db
