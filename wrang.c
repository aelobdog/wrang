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

void usage() {
  printf("usage:\n");
  printf("\n  wrang input_file output_file [ --css stylesheet ] [ --title \"title text\" ]\n\n");
}

int main(int argc, char** argv) {
  // usage: wrang input.wr output.html
  if (argc < 3) { usage(); return 1; }

  TokenList* tk_list;
  struct stat statbuf;
  char* data;

  // reading the wrang source file
  int infile = open(argv[1], O_RDONLY);
  if (infile < 0) { printf("error: could not open file '%s'\n", argv[1]); return 1; }

  fstat(infile, &statbuf);

  data = mmap(NULL, statbuf.st_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, infile, 0);
  if(data == MAP_FAILED){ printf("error: could not read contents of file '%s'\n", argv[1]); return 1; }

  close(infile);

  tk_list = tokenlist_new();

  WRANG_lex(data, tk_list);
  // token_list_print(tk_list);

  TreeNode* ast = WRANG_parse(tk_list);
  // tree_print(ast, 0);

  FILE* html = fopen(argv[2], "w");

  fprintf(html, "<html>\n");
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

  return 0;
}
