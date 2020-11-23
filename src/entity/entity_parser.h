#include <vector>
#include <functional>
#include "entity.h"

///*
using EntityParseCallback = std::function<void(BaseEntity *)>;

///*
// Reads text and converts it to entities, the lifetime of the
// BaseEntity pointer given to the EntityParseCallback is only
// valid for that call.
void parse_entities_and_do(const char *data, const char *filename, EntityParseCallback);

// TODO(ed): Add a way to serialize entities.
