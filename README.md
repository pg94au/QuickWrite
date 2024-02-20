# QuickWrite
A UNIX curses-based text editor built in C as a university team project

This application was originally written as a third-year university project whose goal was to build a text editor application.  It originally targeted SGI IRIX and leveraged the curses library to aid in text rendering.  For this project, we imposed a limitation of only supporting an 80x25 console, but because of that limitation we were able to support automatic word-wrap (paragraphs were delineated by a blank line).  Curses also aided in the implementation of a pull-down menu system.

The back end supported paging to handle text files larger than what the system's memory would support.  Probably never necessary, but it was a requirement for the project.
