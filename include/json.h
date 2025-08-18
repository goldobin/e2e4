#ifndef JSON_H
#define JSON_H

#include "char_slice.h"

/**
 * JSON Parse Functions
 *
 * The parsing part of this file is based on [JSMN library](https://github.com/zserge/jsmn).
 *
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

/**
 * JSON type identifier. Basic types are:
 * 	- Object
 * 	- Array
 * 	- String
 * 	- Other primitive: number, boolean (true/false) or null
 */
typedef enum {
    JSON_TOKEN_TYPE_UNDEFINED = 0,
    JSON_TOKEN_TYPE_OBJECT    = 1 << 0,
    JSON_TOKEN_TYPE_ARRAY     = 1 << 1,
    JSON_TOKEN_TYPE_STRING    = 1 << 2,
    JSON_TOKEN_TYPE_PRIMITIVE = 1 << 3
} JsonTokenType;

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * offset	offset position in JSON data string
 * end		end position in JSON data string
 */
typedef struct {
    JsonTokenType type;
    size_t        offset;
    size_t        len;
    size_t        childrenCount;
    bool          finished;
} JsonToken;

typedef struct {
    JsonToken *arr;
    size_t     len;
    size_t     cap;
} JsonTokens;

JsonToken *JsonTokens_At(const JsonTokens *ts, size_t index);
JsonToken *JsonTokens_Grow(JsonTokens *ts, size_t len);

typedef enum {
    JSON_PARSE_ERROR_OK = 0,
    /* Not enough tokens were provided */
    JSON_PARSE_ERROR_TOKEN_POOL_EXHAUSTED = -1,
    /* Invalid character inside JSON string */
    JSON_PARSE_ERROR_INVALID = -2,
    /* The string is not a full JSON packet, more bytes expected */
    JSON_PARSE_ERROR_PARTIAL = -3
} JsonParseErr;

typedef struct {
    JsonParseErr err;
    size_t       read;
} JsonParseResult;

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each
 * describing
 * a single JSON object.
 */
JsonParseResult JsonParse(JsonTokens *dst, const char *s, size_t len);

/**
 * JSON Write Functions
 */
typedef enum {
    JSON_NODE_TYPE_UNDEFINED = 0,
    JSON_NODE_TYPE_OBJECT    = 1,
    JSON_NODE_TYPE_ARRAY     = 2,
    JSON_NODE_TYPE_FIELD     = 3,
} JsonNodeType;

typedef struct {
    JsonNodeType type;
    bool         needsComma;
} JsonNode;

typedef struct {
    JsonNode *arr;
    size_t    len;
    size_t    cap;
} JsonStack;

typedef struct {
    JsonStack stack;
    CharSlice dst;
} JsonWriter;

JsonNode *JsonStack_Top(const JsonStack *s);
JsonNode *JsonStack_Push(JsonStack *s);
JsonNode  JsonStack_Pull(JsonStack *s);

size_t CharSlice_WriteJsonStart(CharSlice *dst, JsonStack *s, char bracket);
size_t CharSlice_WriteJsonEnd(CharSlice *dst, JsonStack *s);
size_t CharSlice_WriteJsonKey(CharSlice *dst, JsonStack *s, CharSlice key);
size_t CharSlice_WriteJsonValue(CharSlice *dst, JsonStack *s, bool isString, CharSlice value);

#endif /* JSON_H */
