#TODO conditional compile directives in this file

TEMPLATE = subdirs

SRC_SUBDIRS += plvcore
SRC_SUBDIRS += plvgui
SRC_SUBDIRS += plvopencv
SRC_SUBDIRS += plvpluginexample
SRC_SUBDIRS += plvgraphical
SRC_SUBDIRS += plvconsole
SRC_SUBDIRS += plvtcpserver
SRC_SUBDIRS += plvtest
SRC_SUBDIRS += plvsynchrony
SRC_SUBDIRS += plvfreenect
SRC_SUBDIRS += plvmskinect
SRC_SUBDIRS += plvadvancedtracking

SUBDIRS += $$SRC_SUBDIRS

