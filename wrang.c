/*	-----------------------------------------------------------------------------
 *	WRANG, the writing language
 *	-----------------------------------------------------------------------------
 *
 *	MIT License
 *
 *	Copyright (c) 2022 Ashwin Godbole
 *	
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 *	-----------------------------------------------------------------------------
 */

#include "header.h"
#include <stddef.h>
#include <stdio.h>

void usage() {
  printf("usage:\n");
  printf("\n  wrang input_file output_file [ --css stylesheet ] [ --title \"title text\" ]\n\n");
}

int main(int argc, char** argv) {
  // usage: wrang input.wr output.html
  if (argc < 3) { usage(); return 1; }

  TokenList* tk_list;
  char* data;
  size_t filesize = 0;

  // reading the wrang source file
  FILE* infile = fopen(argv[1], "rb");
  fseek(infile, 0L, SEEK_END);
  filesize = ftell(infile);
  fseek(infile, 0L, SEEK_SET);
  data = malloc(filesize + 1);
  if (! data) { fprintf(stderr, "ERROR: could not allocate memory for file's contents.\n"); return 1; }
  if (! fread(data,1,filesize,infile)) { fprintf(stderr, "ERROR: could not read file's contents.\n"); return 1; }
  data[filesize]=0;
  fclose(infile);

  tk_list = tokenlist_new();

  WRANG_lex(data, tk_list);
  // token_list_print(tk_list);

  TreeNode* ast = WRANG_parse(tk_list);
  // tree_print(ast, 0);

  FILE* html = fopen(argv[2], "w");

  fprintf(html, "<html>\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
  if (argc > 3) {
    fprintf(html, "<head>\n");
    for (int arg = 2; arg < argc;) {
      if (! strcmp(argv[arg], "-css")) {
	if (arg == argc - 1) {
	  printf("ERROR: missing path to stylesheet.\n");
	  usage();
	  fclose(html);
	  return 1;
	}
	fprintf(html, "<link rel=\"stylesheet\" href=\"%s\">\n", argv[arg + 1]);
	++arg;
      }

      else if (! strcmp(argv[arg], "-title")) {
	if (arg == argc - 1) {
	  printf("ERROR: missing title.\n");
	  usage();
	  fclose(html);
	  return 1;
	}
	fprintf(html, "<title>%s</title>\n", argv[arg + 1]);
	++arg;
      }
      ++arg;
    }
    fprintf(html, "</head>\n");
  }

  WRANG_gen(ast, html, 0);
  fprintf(html, "</html>");
  fclose(html);
  printf("done.\n");

  WRANG_clean(ast, tk_list);
  free(data);
  return 0;
}
