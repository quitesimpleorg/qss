qss
=========
qss creates a poor-man full-text search for your files using a
sqlite database.

A simple gui (C++ with Qt5) for search  is found in gui/. 
It shows preview pages of documents where your search keywords have 
been found. This allows you to search all indexed pdfs and take a look
at the pages side by side in an instant.

Build
-----
### Ubuntu 20.04
```
sudo apt install build-essential qt5-default libpoppler-qt5-dev libuchardet-dev libquazip5-dev
qmake
make
```


Setup
-----
sqlite3 qss.db < create.sql

TO BE WRITTEN
