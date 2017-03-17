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
#include <string.h>
#include <stdio.h>
#include "patlist.h"

static inline size_t stcmpl(const char* str,size_t len,const char* pref){
	size_t i,pl = strlen(pref);
	if(pl>len)return 0;
	for(i=0;i<pl;++i)
		if(str[i]!=pref[i])return 0;
	return pl; 
}
static inline int pfcomp(const char* a,const char* b){
	size_t A = strlen(a);
	size_t B = strlen(b);
	size_t C = A<B?A:B;
	if(memcmp(a,b,C)) return 0;
	if(A<B) return 1;
	if(A>B) return 2;
	return 3;
}


prefix_ptr prefix_ptr_new(){
	return create( sizeof(struct prefix_list) );
}
void prefix_print(prefix_ptr ptr) {
	while(ptr){
		printf("%p (%p %p) '%s'\n",ptr,ptr->next,ptr->sub,ptr->prefix);
		ptr = ptr->sub;
	}
}

void prefix_insert(prefix_ptr ptr,const char* data) {
	prefix_ptr last = 0;
	prefix_ptr limit = 0;
	prefix_ptr parent = 0;
	prefix_ptr np;
	int n;
	
	while(ptr->prefix) {
		if(ptr==limit) return;
		n = pfcomp(data,ptr->prefix);
		switch(n) {
		case 0:
			if(ptr->next==limit){
				ptr = parent;
				np = prefix_ptr_new();
				np->next = np->sub = ptr->sub;
				np->prefix = data;
				ptr->sub = np;
				return;
			}
			last = ptr;
			ptr = last->next;
			continue;
		case 1:
			np = prefix_ptr_new();
			*np = *ptr;
			ptr->prefix = data;
			ptr->sub = np;
			return;
		case 2:
			if(ptr->next==ptr->sub){
				np = prefix_ptr_new();
				np->next = np->sub = ptr->sub;
				np->prefix = data;
				ptr->sub = np;
				return;
			}
			parent = last = ptr;
			ptr = last->sub;
			limit = last->next;
			continue;
		default:
			np = 0;
			return;
		}
	}
	if(!ptr->prefix) {
		np = prefix_ptr_new();
		ptr->next = ptr->sub = np;
		ptr->prefix = data;
	}
}

size_t prefix_check(prefix_ptr ptr,const char* data,size_t len){
	size_t res=0,cur;
	while(ptr->prefix){
		cur = stcmpl(data,len,ptr->prefix);
		if(cur>res){
			res = cur;
			ptr = ptr->sub;
		} else {
			ptr = ptr->next;
		}
		if(!ptr)break;
	}
	return res;
}

