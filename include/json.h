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
 * 	- Another primitive: number, boolean (true/false) or null
 */
typedef enum {
    JSON_NODE_TYPE_UNSPECIFIED = 0,
    JSON_NODE_TYPE_OBJECT,
    JSON_NODE_TYPE_ARRAY,
    JSON_NODE_TYPE_STRING,
    JSON_NODE_TYPE_PRIMITIVE
} JsonNodeType;

/**
 * JSON node description.
 * type		object, array, string etc
 * offset	position in JSON string
 * len		length in JSON string
 */
typedef struct {
    JsonNodeType type;
    size_t       childrenCount;
    bool         finished;
    size_t       offset;
    size_t       len;
} JsonNode;

typedef struct {
    JsonNode *arr;
    size_t    len;
    size_t    cap;
} JsonNodes;

typedef enum {
    JSON_PARSE_ERROR_OK = 0,
    JSON_PARSE_ERROR_NODE_POOL_EXHAUSTED,
    JSON_PARSE_ERROR_INVALID,
    JSON_PARSE_ERROR_PARTIAL
} JsonParseErr;

typedef struct {
    JsonParseErr err;
    size_t       offset;
} JsonParseResult;

typedef enum {
    JSON_STACK_ENTRY_TYPE_UNSPECIFIED = 0,
    JSON_STACK_ENTRY_TYPE_OBJECT      = 1,
    JSON_STACK_ENTRY_TYPE_ARRAY       = 2,
    JSON_STACK_ENTRY_TYPE_FIELD       = 3,
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

typedef enum {
    JSON_INTERPRET_ERR_OK = 0,
} JsonInterpretErr;

typedef struct {
    JsonInterpretErr err;
    size_t           offset;
} JsonInterpretResult;

typedef struct {
    const Str       str;
    const JsonNodes nodes;
    size_t          index;
} JsonSource;

typedef enum {
    JSON_TYPE_UNSPECIFIED = 0,
    JSON_TYPE_OBJECT,
    JSON_TYPE_KEY,
    JSON_TYPE_ARRAY,
    JSON_TYPE_NULL,
    JSON_TYPE_BOOL,
    JSON_TYPE_NUMERIC,
    JSON_TYPE_STRING,
} JsonType;

JsonNode       *JsonNodes_At(JsonNodes nodes, size_t index);
JsonNode       *JsonNodes_Push(JsonNodes *dst);
JsonParseResult JsonNodes_Parse(JsonNodes *dst, Str src);
void            JsonSource_Reset(JsonSource *s);
bool            JsonSource_Next(JsonSource *s);
bool            JsonSource_Skip(JsonSource *s);
JsonType        JsonSource_Type(const JsonSource *s);
size_t          JsonSource_ChildrenCount(const JsonSource *s);
Str             JsonSource_Value(const JsonSource *s);
bool            JsonSource_BoolValue(const JsonSource *s);
size_t          CharSlice_WriteJsonParseErr(CharSlice *dst, JsonParseErr err);
size_t          CharSlice_WriteJsonParseResult(CharSlice *dst, const JsonParseResult *r);
size_t          CharSlice_WriteJsonStart(CharSlice *dst, JsonStack *s, char bracket);
size_t          CharSlice_WriteJsonEnd(CharSlice *dst, JsonStack *s);
size_t          CharSlice_WriteJsonKey(CharSlice *dst, JsonStack *s, Str key);
size_t          CharSlice_WriteJsonStr(CharSlice *dst, JsonStack *s, Str value);
size_t          CharSlice_WriteJsonBool(CharSlice *dst, JsonStack *s, bool value);
size_t          CharSlice_WriteJsonNull(CharSlice *dst, JsonStack *s);
size_t          CharSlice_WriteJsonNumeric(CharSlice *dst, JsonStack *s, Str value);

#define JsonNodes_Make(len1, cap1) \
    (((cap1) > 0) ? (JsonNodes){.arr = (JsonNode[cap1]){}, .len = (len1), .cap = (cap1)} : (JsonNodes){})

#define JsonStack_Make(len1, cap1) \
    ((cap1) > 0 ? ((JsonStack){.arr = (JsonStackEntry[cap1]){}, .len = (len1), .cap = (cap1)}) : (JsonStack){})

#endif /* JSON_H */
