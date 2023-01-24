#1 Wrang, the writing language

#3 Background
Not long before starting this project I was working on SITEFL, a simple markup language intended for conversion
of text to html documents. I use this project extensively for my own webpages. I started out in Go, and while
that worked for me, it was sloppily written and was in need of a successor. I wrote one called LFET, but to my
dismay, I found out recently that I'd deleted that directory with its source code accidently. All that remains
of it is its binary, carefully stashed away in another folder on my system. But that got me thinking, why not
rewrite the tool properly? with a lexer, parser and everything? Why not just rewrite it in C? This project is a
result of my response to that train of thought. The syntax will remain unchanged so that I don't have to manually
refactor all my existing webpages.

#3 Goals and Non-Goals
There is only one goal with the project really... and that is to have a markdown like text to html tool so that
setting up websites is easy. I would like to support most basic requirements that I have from such a tool, like
the ability to have headings, paragraphs, bold/italics/underlined text, bullets, code blocks, links and images.

Since I am the only user of this tool, I don't intend to support all the different things that something like
github flavored markdown supports for example. I don't use tables almost ever, so those aren't supported. I also
don't need complicated formatting, which is why there is no support for labelled units (like classes and id's)
to customize the CSS. The generated HTML will have no DIVs, no SPANs, no classes, no id's, etc.

Error reporting is also going to be quite limited as good error messages are not high up in the priotity list at
the moment. Maybe once the tool works the way is should, I will try and implement better error reporting.
