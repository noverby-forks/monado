// Copyright 2019-2023, Collabora, Ltd.
// SPDX-License-Identifier: BSL-1.0
/*!
 * @file
 * @brief  Small debug helpers.
 * @author Jakob Bornecrantz <jakob@collabora.com>
 * @ingroup aux_util
 *
 * Debug get option helpers heavily inspired from mesa ones.
 */

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "util/u_debug.h"
#include "util/u_logging.h"

#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


DEBUG_GET_ONCE_BOOL_OPTION(print, "XRT_PRINT_OPTIONS", false)


/*
 *
 * Helpers
 *
 */

static const char *
os_getenv(const char *name)
{
	return getenv(name);
}

static const char *
level_to_str(enum u_logging_level level)
{
	switch (level) {
	case U_LOGGING_TRACE: return "trace";
	case U_LOGGING_DEBUG: return "debug";
	case U_LOGGING_INFO: return "info";
	case U_LOGGING_WARN: return "warn";
	case U_LOGGING_ERROR: return "error";
	default: return "invalid";
	}
}

static const char *
tristate_to_str(enum debug_tristate_option tristate)
{
	switch (tristate) {
	case DEBUG_TRISTATE_OFF: return "OFF";
	case DEBUG_TRISTATE_AUTO: return "AUTO";
	case DEBUG_TRISTATE_ON: return "ON";
	default: return "invalid";
	}
}

/*!
 * This function checks @p str if it matches @p matches, it returns true as long
 * as the complete @p str is in the starts of @p matches. Empty string does not
 * match.
 */
static bool
is_str_in_start_of(const char *str, const char *matches)
{
	if (str[0] == '\0') {
		return false;
	}

	for (int i = 0; str[i] != '\0'; i++) {
		if (matches[i] == '\0') {
			return false;
		}
		if (matches[i] != tolower(str[i])) {
			return false;
		}
	}

	return true;
}


/*
 *
 * 'Exported' conversion functions.
 *
 */

bool
debug_string_to_bool(const char *string)
{
	if (string == NULL) {
		return false;
	} else if (!strcmp(string, "false")) {
		return false;
	} else if (!strcmp(string, "FALSE")) {
		return false;
	} else if (!strcmp(string, "off")) {
		return false;
	} else if (!strcmp(string, "OFF")) {
		return false;
	} else if (!strcmp(string, "no")) {
		return false;
	} else if (!strcmp(string, "NO")) {
		return false;
	} else if (!strcmp(string, "n")) {
		return false;
	} else if (!strcmp(string, "N")) {
		return false;
	} else if (!strcmp(string, "f")) {
		return false;
	} else if (!strcmp(string, "F")) {
		return false;
	} else if (!strcmp(string, "0")) {
		return false;
	} else {
		return true;
	}
}

enum debug_tristate_option
debug_string_to_tristate(const char *string)
{
	if (string == NULL) {
		return DEBUG_TRISTATE_AUTO;
	} else if (!strcmp(string, "AUTO")) {
		return DEBUG_TRISTATE_AUTO;
	} else if (!strcmp(string, "auto")) {
		return DEBUG_TRISTATE_AUTO;
	} else if (!strcmp(string, "a")) {
		return DEBUG_TRISTATE_AUTO;
	} else if (!strcmp(string, "A")) {
		return DEBUG_TRISTATE_AUTO;
	} else {
		if (debug_string_to_bool(string)) {
			return DEBUG_TRISTATE_ON;
		} else {
			return DEBUG_TRISTATE_OFF;
		}
	}
}

long
debug_string_to_num(const char *string, long _default)
{
	if (string == NULL) {
		return _default;
	}

	char *endptr;
	long ret = strtol(string, &endptr, 0);

	// Restore the default value when no digits were found.
	if (string == endptr) {
		return _default;
	}

	return ret;
}

float
debug_string_to_float(const char *string, float _default)
{
	if (string == NULL) {
		return _default;
	}

	char *endptr;
	float ret = strtof(string, &endptr);

	// Restore the default value when no digits were found.
	if (string == endptr) {
		return _default;
	}

	return ret;
}

enum u_logging_level
debug_string_to_log_level(const char *string, enum u_logging_level _default)
{
	if (string == NULL) {
		return _default;
	} else if (is_str_in_start_of(string, "trace")) {
		return U_LOGGING_TRACE;
	} else if (is_str_in_start_of(string, "debug")) {
		return U_LOGGING_DEBUG;
	} else if (is_str_in_start_of(string, "info")) {
		return U_LOGGING_INFO;
	} else if (is_str_in_start_of(string, "warn")) {
		return U_LOGGING_WARN;
	} else if (is_str_in_start_of(string, "error")) {
		return U_LOGGING_ERROR;
	} else {
		return _default;
	}
}


/*
 *
 * 'Exported' debug value getters.
 *
 */

const char *
debug_get_option(const char *name, const char *_default)
{
	const char *raw = os_getenv(name);
	const char *ret;

	if (raw == NULL) {
		ret = _default;
	} else {
		ret = raw;
	}

	if (debug_get_bool_option_print()) {
		U_LOG_RAW("%s=%s (%s)", name, ret, raw == NULL ? "nil" : raw);
	}

	return ret;
}

bool
debug_get_bool_option(const char *name, bool _default)
{
	const char *raw = os_getenv(name);
	bool ret = raw == NULL ? _default : debug_string_to_bool(raw);

	if (debug_get_bool_option_print()) {
		U_LOG_RAW("%s=%s (%s)", name, ret ? "TRUE" : "FALSE", raw == NULL ? "nil" : raw);
	}

	return ret;
}

enum debug_tristate_option
debug_get_tristate_option(const char *name)
{
	const char *raw = os_getenv(name);
	enum debug_tristate_option ret = debug_string_to_tristate(raw);

	if (debug_get_bool_option_print()) {
		const char *pretty_val = tristate_to_str(ret);
		U_LOG_RAW("%s=%s (%s)", name, pretty_val, raw == NULL ? "nil" : raw);
	}

	return ret;
}

long
debug_get_num_option(const char *name, long _default)
{
	const char *raw = os_getenv(name);
	long ret = debug_string_to_num(raw, _default);

	if (debug_get_bool_option_print()) {
		U_LOG_RAW("%s=%li (%s)", name, ret, raw == NULL ? "nil" : raw);
	}

	return ret;
}

float
debug_get_float_option(const char *name, float _default)
{
	const char *raw = os_getenv(name);
	float ret = debug_string_to_float(raw, _default);

	if (debug_get_bool_option_print()) {
		U_LOG_RAW("%s=%f (%s)", name, ret, raw == NULL ? "nil" : raw);
	}

	return ret;
}

enum u_logging_level
debug_get_log_option(const char *name, enum u_logging_level _default)
{
	const char *raw = os_getenv(name);
	enum u_logging_level ret = debug_string_to_log_level(raw, _default);

	if (debug_get_bool_option_print()) {
		const char *pretty_val = level_to_str(ret);
		U_LOG_RAW("%s=%s (%s)", name, pretty_val, raw == NULL ? "nil" : raw);
	}

	return ret;
}
