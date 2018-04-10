/*
 * Copyright (c) 2017-2018 Simon Schmidt
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
#include "luasrc/lualib.h"
#include "luasrc/lauxlib.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include "buildmacro.h"

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
static int normalize (lua_State *L){
	int i,n;
	n = lua_gettop(L);
	for(i=1;i<=n;i++){
		switch(lua_type(L,i)){
		case LUA_TNIL:
		case LUA_TBOOLEAN:
		case LUA_TTABLE:
		case LUA_TFUNCTION:
		case LUA_TUSERDATA:
		case LUA_TTHREAD:
		case LUA_TLIGHTUSERDATA:
			lua_pushliteral(L,"");
			lua_replace(L,i);
			break;
		case LUA_TNUMBER:
			lua_tostring(L,i);
			break;
		}
	}
	lua_settop(L,n);
	return 1;
}
static int import_file (lua_State *L){
	lua_settop(L,1);
	const char* fp = lua_tostring(L,1);
	switch(luaL_loadfile(L,fp)) {
		case LUA_OK: goto noerr;
		case LUA_ERRFILE:
		fprintf(stderr,"File not found: %s",fp);
		lua_pushfstring(L,"File not found: %s",fp);
		break;
		default:
		fprintf(stderr,"Error parsing file: %s",fp);
		lua_pushfstring(L,"Error parsing file: %s",fp);
	}
	return lua_error(L);
	noerr:
	lua_call(L,0,0);
	lua_settop(L,0);
	return 0;
}
static int include_file(lua_State *L){
	lua_settop(L,1);
	const char* fp = lua_tostring(L,1);
	struct tokenizer tok;
	tok.buffer = smust(sdsempty());
	tok.source = ptrmust(fopen(fp,"r"));
	OutputStream dest = OutputStream_new();
	lua_settop(L,0);
	parser_parse(&tok,L,dest);
	lua_settop(L,0);
	sdsfree(tok.buffer);
	fclose((FILE*)tok.source);
	luab_pushsds(L,(sds)(dest->data) );
	OutputStream_detroy(dest);
	return 1;
}
static int bdmcr_build_macro_lua (lua_State *L){
	int i,j,m,n;
	static sds margs[1024];
	sds *args=0;
	sds *allc=0;
	sds body,result;
	
	n = lua_gettop(L);
	if(n<1){
		lua_pushliteral(L,"");
		return 1;
	}
	body = smust(luab_tosds(L,1));
	m = n-1;
	if(m<1024) args = margs;
	else args=allc=malloc(sizeof(sds)*m); /* XXX: Overflow hazard. */
	
	for(i=0,j=2;i<m;++i) {
		args[i] = smust(luab_tosds(L,j));
	}
	
	result = bdmcr_build_macro(args,m,body);
	
	sdsfree(body);
	for(i=0,j=2;i<m;++i)
		sdsfree(args[i]);
	
	if(allc)free(allc);
	
	lua_settop(L,0);
	
	luab_pushsds(L,result);
	sdsfree(result);
	return 1;
}
static int stringify (lua_State *L){
	static const char hex[]="0123456789ABCDEF";
	char buf[] = "\\ ";
	char buf2[2];
	int x;
	const char* ccin;
	size_t i;
	size_t ccins;
	OutputStream dest;
	lua_settop(L,1);
	
	dest = OutputStream_new();
	ccin = lua_tolstring(L,1,&ccins);
	
	OutputStream_write(dest,"\"",1);
	
	
	for(i=0;i<ccins;++i) switch(x=(ccin[i]&0xff) ){
		case '\\':
		case '\"':
		case '\'':
			buf[1]=x;
			OutputStream_write(dest,buf,2);
			continue;
		case 0 ... 31:
		case 0x7f ... 0xff:
			switch(x){
			case '\r': OutputStream_write(dest,"\\r",2); continue;
			case '\n': OutputStream_write(dest,"\\n",2); continue;
			case '\t': OutputStream_write(dest,"\\t",2); continue;
			}
			buf2[0]=hex[(x>>4)&0xf];
			buf2[1]=hex[(x   )&0xf];
			OutputStream_write(dest,"\\x",2);
			OutputStream_write(dest,buf2,2);
			continue;
		default:
			OutputStream_write(dest,ccin+i,1);
	}
	
	OutputStream_write(dest,"\"",1);
	
	lua_settop(L,0);
	luab_pushsds(L,(sds)(dest->data) );
	
	OutputStream_detroy(dest);
	return 1;
}

static int append (lua_State *L){
	lua_Integer li;
	lua_len(L,-2);
	li = lua_tointeger(L,-1);
	lua_pop(L,1);
	lua_rawseti(L,-2,li+1);
	return 0;
}
void luab_append(lua_State* L){
	append(L);
}
static int checkfile(lua_State *L){
	FILE * fobj;
	const char* fp;
	int ok = 0;
	lua_settop(L,1);
	fp = lua_tostring(L,1);
	if(!fp) goto done;
	fobj = fopen(fp,"r");
	if(!fobj) goto done;
	fclose(fobj);
	ok = 1;
	
	done:
	
	lua_settop(L,0);
	lua_pushboolean(L,ok);
	return 1;
}


#define register_const(L,name) (lua_pushinteger(L, name),lua_setglobal(L, #name ))
lua_State* create_lua(){
	lua_State *L = (lua_State*)ptrmust(luaL_newstate());
	luaL_openlibs(L);
	lua_settop(L,0);
	lua_register(L,"solve",solve);
	lua_register(L,"normalize",normalize);
	lua_register(L,"import_file",import_file);
	lua_register(L,"include_file",include_file);
	lua_register(L,"bdmcr_build_macro",bdmcr_build_macro_lua);
	lua_register(L,"stringify",stringify);
	lua_register(L,"append",append);
	lua_register(L,"checkfile",checkfile);
	
	lua_newtable(L);
	lua_setglobal(L,"MACROS");
	lua_newtable(L);
	lua_setglobal(L,"MACROT");
	//luaL_loadstring(L, "function(n,f,t) MACROS[n]=f; MACROT[n]=t; return nil end");
	luaL_loadstring(L, "local n,f,t = ...; MACROS[n]=f; MACROT[n]=t; return nil");
	lua_setglobal(L,"REGISTER");
	
	lua_newtable(L);
	lua_setglobal(L,"INCLUDES");
	luaL_loadstring(L, "local incl = ...; local i,n; for i = 1, #INCLUDES do n=INCLUDES[i]..'/'..incl; if checkfile(n) then return n end; end; return incl ");
	lua_setglobal(L,"find_include");
	
	register_const(L, MT_ARGS);
	register_const(L, MT_BODY);
	register_const(L, MT_SEMI);
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
	return i;
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
	lua_settop(L,0);
}
sds luab_eval(lua_State* L,sds code){
	luaL_loadstring(L, code);
	lua_pcall(L,0,1,0);
	const char* c = lua_tostring(L,-1);
	if(!c) c="";
	sds ret = smust(sdsnew(c));
	lua_settop(L,0);
	return ret;
}

