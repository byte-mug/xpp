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
#include <stdio.h>
#include <string.h>
#include "patlist.h"
#include "strtest.h"
#include "tokenizer.h"
#include "luab.h"
#include "parser.h"
#include "luasrc/lauxlib.h"
#include "buildmacro.h"
#include <unistd.h>

int main(int argc,char** argv){
	int c;
	int code;
	struct tokenizer tok;
	OutputStream dest = 0;
	tok.buffer = smust(sdsempty());
	tok.source = stdin;
	
	lua_State* L = create_lua();
	lua_settop(L,0);
	
	while ( (c = getopt(argc, argv, "I:L:")) != -1) {
		switch(c) {
		case 'I':
			
			lua_getglobal(L,"INCLUDES");
			lua_pushstring(L,optarg);
			luab_append(L);
			lua_settop(L,0);
			break;
		case 'L':
			code = luaL_dofile(L,optarg);
			lua_settop(L,0);
			if(code==LUA_OK) break;
			fprintf(stderr,"Loading library file: %s -> %d\n",optarg,code);
			return -1;
		}
	}
	if(optind < argc) {
		tok.source = fopen(argv[optind++],"r");
		if(!tok.source) { perror("Open input file"); return -1; }
	}
	if(optind < argc) {
		FILE * f = fopen(argv[optind++],"w");
		if(!f) { perror("Open input file"); return -1; }
		dest = FILE_asStream(f);
	}
	if(!dest) dest = FILE_asStream(stdout);
	
	//luab_setmacro(L,"_Include","local name = ...; return include_file(find_include(name)); ",MT_ARGS);
	//luab_setmacro(L,"_Import","local name = ...; return import_file(find_include(name)); ",MT_ARGS);
	
	tokenizer_init();
	
	parser_parse(&tok,L,dest);
	
	return 0;
}



