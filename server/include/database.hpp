
#pragma once

#include <db/accounts.hpp>
#include <logger.hpp>
#include <environment.hpp>


namespace nabu {

template <template <class> class...Tables>
class basic_database : public db::database, Tables<basic_database<Tables...>>... {
public:
	database&       self()       noexcept { return *static_cast<database*      >(this); }
	database const& self() const noexcept { return *static_cast<database const*>(this); }

	explicit basic_database(std::string const& name) : 
		database{ name }, Tables<basic_database<Tables...>>{}... {}
};

using database_type = basic_database<db::accounts_table>;
NABU_DECLARE_GLOBAL(database_type, database);

} // nabu
