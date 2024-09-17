#include "entry.h"

auto Entry::create(const std::string &path, bool is_directory,
                   const std::string &db_name) -> std::expected<Entry, std::string> {}