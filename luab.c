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
#include "luab.h"
#include "luasrc/lauxlib.h"
#include "parser.h"
#include <stdio.h>

#if 0
static int this_is_not_a_function (lua_State *L){
	return 0;
}
#endif
static int solve(lua_State *L){
	struct tokenizer tok;
	tok.buffer = smust(luab_tosds(L,1));
	tok.source = 0;
	OutputStream dest = OutputStream_new();
	lua_settop(L,0);
	parser_parse(&tok,L,dest);
	lua_settop(L,0);
	sdsfree(tok.buffer);
	luab_pushsds(L,(sds)(dest->data) );
	OutputStream_detroy(dest);
	return 1;
}

#define register_const(L,name) (lua_pushinteger(L, name),lua_setglobal(L, #name ))
lua_State* create_lua(){
	lua_State *L = (lua_State*)ptrmust(luaL_newstate());
	luaL_openlibs(L);
	lua_settop(L,0);
	lua_register(L,"solve",solve);
	lua_newtable(L);
	lua_setglobal(L,"MACROS");
	lua_newtable(L);
	lua_setglobal(L,"MACROT");
	luaL_loadstring(L, "function(n,f,t) MACROS[n]=f; MACROT[n]=t; return nil end");
	lua_setglobal(L,"REGISTER");
	register_const(L, MT_ARGS);
	register_const(L, MT_BODY);
	return L;
}
sds luab_tosds(lua_State* L,int index){
	size_t s;
	const char* cs = lua_tolstring(L,index,&s);
	if(!cs) return (sds)0;
	return smust(sdsnewlen(cs,s));
}
void luab_pushsds(lua_State* L,sds str){
	lua_pushlstring(L,str,sdslen(str));
}
void luab_call(lua_State* L){
	int top = lua_gettop(L);
	lua_call(L,top-1,1);
}
int  luab_getmacro(lua_State* L, sds macro){
	int i;
	lua_settop(L,0);
	lua_getglobal(L,"MACROT");
	lua_getfield(L,1,macro);
	i = lua_tointeger(L,2);
	lua_settop(L,0);
	lua_getglobal(L,"MACROS");
	if(LUA_TFUNCTION!=lua_getfield(L,1,macro))
		return -1;
	lua_remove(L,1);
}
void luab_setmacro(lua_State* L, const char* macro, const char* code,int type){
	int rc;
	lua_settop(L,0);
	lua_getglobal(L,"MACROS");
	rc = luaL_loadstring(L,code);
	switch(rc){
	case LUA_OK: break;
	case LUA_ERRSYNTAX: fprintf(stderr,"LUA_ERRSYNTAX:%s\n",code); abort(); break;
	case LUA_ERRMEM: fprintf(stderr,"LUA_ERRMEM:%s\n",code); abort(); break;
	case LUA_ERRGCMM: fprintf(stderr,"LUA_ERRGCMM:%s\n",code); abort(); break;
	default: printf("%d:%s\n",rc,code); abort(); break;
	}
	lua_setfield(L,1,macro);
	lua_settop(L,0);
	lua_getglobal(L,"MACROT");
	lua_pushinteger(L, type);
	lua_setfield(L,1,macro);
	printf("gettop %d\n",lua_gettop(L));
	lua_settop(L,0);
	
	
}

