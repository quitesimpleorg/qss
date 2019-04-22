TEMPLATE = subdirs
SUBDIRS = gui cli shared

cli.depends = shared
gui.depends = shared
