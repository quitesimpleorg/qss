qss
=========
quite simple search (qss) creates a poor-man full-text search for your files using a sqlite database.

A simple gui renders the pages of the documents where your search keywords have been found. Currently, this allows you search all indexed pdfs and take a look at the pages side by side in an instant.

The name of the project will probably be changed.

Screenshots
-----------
Coming soon™


Goals
=====
 * **Find & Preview**. Instead of merely telling you where your search phrase has been found, it should also render  the corresponding portion/pages of the documents and highlight the searched words.
 * **No daemons**. As other solutions are prone to have annoying daemons running that eat system resources away, this solution should make do without daemons if possible.
 * **Easy setup**. Similiarly, there should be no need for heavy-weight databases. Instead, this solution tries to squeeze out the most from simple approaches. In particular, it relies on sqlite. 
 * **GUI & CLI**. Provide CLI interfaces and GUI interfaces


Build
-----
### Ubuntu 20.04
```
sudo apt install build-essential qt5-default libpoppler-qt5-dev libuchardet-dev libquazip5-dev
qmake
make
```

Documentation
-------------
Coming soon™

Packages
-----
Coming soon™
