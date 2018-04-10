/*
 * Copyright (c) 2017 Simon Schmidt
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#pragma once

#include "luasrc/lua.h"
#include "baselib.h"

enum {
	MT_ARGS = 1<<5,
	MT_BODY = 1<<6,
	MT_SEMI = 1<<7, /* ARGS: Semicolon instead of comma. */
};

lua_State* create_lua();
sds luab_tosds(lua_State* L,int index);
void luab_pushsds(lua_State* L,sds str);
void luab_call(lua_State* L);
int  luab_getmacro(lua_State* L, sds macro);
void luab_setmacro(lua_State* L, const char* macro, const char* code,int type);

sds luab_eval(lua_State* L,sds code);

/*
 * Does the equivalent of t[(#t)+1] = v,
 * where v is the value at the top of the stack,
 * and t is the value just below the top.
 *
 * This function pops both the table and the value from the stack.
**/
void luab_append(lua_State* L);

