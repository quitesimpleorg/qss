qss
=========
qss creates a poor-man full-text search for your files using a
sqlite database.

You need the python "chardet" package, since it will try to convert the 
encoding of the files in case initial utf-8 decoding fails.

pdftext is needed to search in .pdf files..

A simple gui (C++ with Qt5) for search  is found in gui/. 
It shows preview pages of documents where your search keywords have 
been found. This allows you to search all indexed pdfs and take a look
at the pages side by side in an instant.

Build
-----
TO BE WRITTEN

Setup
-----
sqlite3 qss.db < create.sql

TO BE WRITTEN




