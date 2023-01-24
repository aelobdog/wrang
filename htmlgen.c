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

void WRANG_gen(TreeNode* root, FILE* html, u8 inlist) {
  if (root == NULL) return;

  switch (root->node_type) {

  case ROOT:
    fprintf(html, "\n<body>\n<div class=\"content\">\n");
    WRANG_gen(root->left_child->right_sibling, html, 0); // skip over the first newline
    WRANG_gen(root->right_sibling, html, 0); // a root with a sibling !?
    fprintf(html, "\n</div>\n</body>\n");
    break;

  case PLAINTEXT:
    {
      Token* token = root->content.word->token;
      fwrite(token->ptr, 1, token->len, html);
      WRANG_gen(root->left_child, html, 0);
      WRANG_gen(root->right_sibling, html, 0);
    }
    break;

  case HEADING:
    fprintf(html, "\n<h%d>\n", root->content.heading->level);
    WRANG_gen(root->left_child, html, 0);
    fprintf(html, "\n</h%d>\n", root->content.heading->level);
    WRANG_gen(root->right_sibling, html, 0);
    break;

  case LINE:
    fprintf(html, "\n<hr>\n");
    WRANG_gen(root->left_child, html, 0); // this shouldn't technically do anything ... remove it ?
    WRANG_gen(root->right_sibling, html, 0);
    break;

  case NEWLINE:
    fprintf(html, "\n<span style=\"display: block; margin-bottom: 1.5em\"></span>\n");
    WRANG_gen(root->left_child, html, 0);
    WRANG_gen(root->right_sibling, html, 0);
    break;

  case BOLD:
    fprintf(html, "\n<strong>\n");
    WRANG_gen(root->left_child, html, 0);
    fprintf(html, "\n</strong>\n");
    WRANG_gen(root->right_sibling, html, 0);
    break;

  case ITALICS:
    fprintf(html, "\n<em>\n");
    WRANG_gen(root->left_child, html, 0);
    fprintf(html, "\n</em>\n");
    WRANG_gen(root->right_sibling, html, 0);
    break;

  case UNDERLINE:
    fprintf(html, "\n<u>\n");
    WRANG_gen(root->left_child, html, 0);
    fprintf(html, "\n</u>\n");
    WRANG_gen(root->right_sibling, html, 0);
    break;

  case CODE:
    fprintf(html, "\n<code>\n");
    WRANG_gen(root->left_child, html, 0);
    fprintf(html, "\n</code>\n");
    WRANG_gen(root->right_sibling, html, 0);
    break;

  case LINK:
    fprintf(html, "\n<a href=\"");
    WRANG_gen(root->content.link->url, html, 0);
    fprintf(html, "\">");
    WRANG_gen(root->left_child, html, 0);
    fprintf(html, "</a>\n");
    WRANG_gen(root->right_sibling, html, 0);
    break;

  case IMAGE:
    {
      Token* token = root->content.image->css_class;
      fprintf(html, "\n<img src=\"");
      WRANG_gen(root->content.link->url, html, 0);
      fprintf(html, "\" class=\"");
      fwrite(token->ptr, 1, token->len, html);
      fprintf(html, "\" alt=\"");
      WRANG_gen(root->left_child, html, 0);
      fprintf(html, "\">\n");
      WRANG_gen(root->right_sibling, html, 0);
    }
    break;

  case LIST:
    {
      TreeNode* right = root->right_sibling;
      TreeNode* right_iter = root->right_sibling;
      if (! inlist) fprintf(html, "\n<ul>\n");

      fprintf(html, "<li>");
      WRANG_gen(root->left_child, html, 0);
      fprintf(html, "</li>\n");

      if (! inlist) {
	while (right_iter->node_type == LIST) {
	  right_iter = right_iter->right_sibling;
	}
      }

      if (right->node_type == LIST) {
	WRANG_gen(right, html, inlist+1);
      }

      if (! inlist) fprintf(html, "</ul>\n");
      if (! inlist) WRANG_gen(right_iter, html, 0);
    }
    break;

  case URL:
    WRANG_gen(root->left_child, html, 0);
    WRANG_gen(root->right_sibling, html, 0); // this should not happen no?
    break;

  }

}
