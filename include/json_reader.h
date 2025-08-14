/*
 * MIT License
 *
 * Copyright (c) 2010 Serge Zaitsev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef JSMN_H
#define JSMN_H

#include <stddef.h>

#include "char_slice.h"

/**
 * JSON type identifier. Basic types are:
 * 	- Object
 * 	- Array
 * 	- String
 * 	- Other primitive: number, boolean (true/false) or null
 */
typedef enum {
    JSON_TYPE_UNDEFINED = 0,
    JSON_TYPE_OBJECT    = 1 << 0,
    JSON_TYPE_ARRAY     = 1 << 1,
    JSON_TYPE_STRING    = 1 << 2,
    JSON_TYPE_PRIMITIVE = 1 << 3
} JsonType;

typedef enum {
    JSON_PARSE_ERROR_OK = 0,
    /* Not enough tokens were provided */
    JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED = -1,
    /* Invalid character inside JSON string */
    JSON_PARSE_ERROR_INVALID = -2,
    /* The string is not a full JSON packet, more bytes expected */
    JSON_PARSE_ERROR_PARTIAL = -3
} JsonParseErr;

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * start	start position in JSON data string
 * end		end position in JSON data string
 */
typedef struct {
    JsonType type;
    size_t   start;
    int      end;
    size_t   childrenCount;
} JsonToken;

typedef struct {
    JsonToken* arr;
    size_t     len;
    size_t     cap;
} JsonTokens;

JsonToken* JsonTokens_At(const JsonTokens* ts, size_t index);

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string.
 */
typedef struct JsonParser {
    size_t offset;           /* offset in the JSON string */
    int    parentTokenIndex; /* superior token node, e.g. parent object or array */
} JsonParser;

/**
 * Create JSON parser over an array of tokens
 */
void JsonParser_Init(JsonParser* parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each
 * describing
 * a single JSON object.
 */
JsonParseErr JsonParser_Parse(JsonParser* parser, JsonTokens* dst, const char* s, size_t len);

#endif /* JSMN_H */
