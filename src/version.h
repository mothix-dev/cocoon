#pragma once

// https://stackoverflow.com/a/32548446

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0
#define VERSION_SPECIAL "-dev"

#define STRINGIFY0(s) # s
#define STRINGIFY(s) STRINGIFY0(s)

#define VERSION STRINGIFY(VERSION_MAJOR) "." STRINGIFY(VERSION_MINOR) "." STRINGIFY(VERSION_PATCH) "" VERSION_SPECIAL

#define PROGRAM_NAME "cocoon"

#define NAME_VERSION_INFO PROGRAM_NAME " " VERSION " (built on " __DATE__ " at " __TIME__ " with " __VERSION__ ")"
