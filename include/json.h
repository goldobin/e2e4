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
    JSON_TOKEN_TYPE_NONE = 0,
    JSON_TOKEN_TYPE_OBJECT,
    JSON_TOKEN_TYPE_ARRAY,
    JSON_TOKEN_TYPE_STRING,
    JSON_TOKEN_TYPE_PRIMITIVE
} JsonTokenType;

/**
 * JSON token description.
 * type		type (object, array, string etc.)
 * offset	offset position in JSON data string
 * end		end position in JSON data string
 */
typedef struct {
    size_t        offset;
    size_t        len;
    JsonTokenType type;
    size_t        childrenCount;
    bool          finished;
} JsonToken;

CharSlice JsonToken_View(const JsonToken *t, CharSlice src);

typedef struct {
    JsonToken *arr;
    size_t     len;
    size_t     cap;
} JsonTokens;

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
    size_t       offset;
} JsonParseResult;

#define JsonTokens_Make(len1, cap1) \
    (((cap1) > 0) ? (JsonTokens){.arr = (JsonToken[cap1]){}, .len = (len1), .cap = (cap1)} : (JsonTokens){})

JsonToken      *JsonTokens_At(const JsonTokens *ts, size_t index);
JsonToken      *JsonTokens_Push(JsonTokens *ts);
size_t          JsonTokens_Skip(const JsonTokens *ts, size_t index);
JsonParseResult JsonTokens_Parse(JsonTokens *dst, CharSlice src);

/**
 * JSON Write Functions
 */
typedef enum {
    JSON_STACK_ENTRY_TYPE_NONE   = 0,
    JSON_STACK_ENTRY_TYPE_OBJECT = 1,
    JSON_STACK_ENTRY_TYPE_ARRAY  = 2,
    JSON_STACK_ENTRY_TYPE_FIELD  = 3,
} JsonStackEntryType;

typedef struct {
    JsonStackEntryType type;
    bool               needsComma;
} JsonStackEntry;

typedef struct {
    JsonStackEntry *arr;
    size_t          len;
    size_t          cap;
} JsonStack;

#define JsonStack_Make(len1, cap1) \
    ((cap1) > 0 ? ((JsonStack){.arr = (JsonStackEntry[cap1]){}, .len = (len1), .cap = (cap1)}) : (JsonStack){})

size_t CharSlice_WriteJsonStart(CharSlice *dst, JsonStack *s, char bracket);
size_t CharSlice_WriteJsonEnd(CharSlice *dst, JsonStack *s);
size_t CharSlice_WriteJsonKey(CharSlice *dst, JsonStack *s, CharSlice key);
size_t CharSlice_WriteJsonString(CharSlice *dst, JsonStack *s, CharSlice value);
size_t CharSlice_WriteJsonBool(CharSlice *dst, JsonStack *s, bool value);
size_t CharSlice_WriteJsonNull(CharSlice *dst, JsonStack *s);
size_t CharSlice_WriteJsonNumeric(CharSlice *dst, JsonStack *s, CharSlice value);

typedef enum {
    JSON_INTERPRET_ERR_OK = 0,
} JsonInterpretErr;

typedef struct {
    JsonInterpretErr err;
    size_t           tokenOffset;
} JsonInterpretResult;

typedef struct {
    const CharSlice   charSlice;
    const JsonTokens *tokens;
    size_t            index;
} JsonSource;

void             JsonSource_Reset(JsonSource *s);
bool             JsonSource_Skip(JsonSource *s);
const JsonToken *JsonSource_Token(const JsonSource *s);
CharSlice        JsonSource_Value(const JsonSource *s);

#endif /* JSON_H */
