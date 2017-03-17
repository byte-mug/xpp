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
#include "output.h"
OutputStream FILE_asStream(FILE* f){
	OutputStream s = create(sizeof(sOutputStream));
	s->type = OutputStream_FILE;
	s->data = f;
	return s;
}
OutputStream OutputStream_new(){
	OutputStream s = create(sizeof(sOutputStream));
	s->type = OutputStream_sds;
	s->data = smust(sdsempty());
	return s;
}

void OutputStream_write(OutputStream stream, const char* data, size_t len){
	sds i;
	switch(stream->type){
	case OutputStream_FILE:
		fwrite(data,len,1,stream->data);
		break;
	case OutputStream_sds:
		i = (sds)(stream->data);
		i = smust(sdscatlen(i,data,len));
		stream->data = i;
		break;
	}
}
void OutputStream_detroy(OutputStream stream){
	switch(stream->type){
	case OutputStream_FILE:
		fclose(stream->data);
		break;
	case OutputStream_sds:
		sdsfree((sds)(stream->data));
		break;
	}
	free(stream);
}


