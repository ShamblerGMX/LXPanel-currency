#Edit these variables.

LXDEBUILD_PREFIX=.

LXDEBUILD_PLUGIN_NAME=currency

LXDEBUILD_SOURCEFILES=currency.c
 

#These variables should probably not be edited, they are for convenience only.

LXDEBUILD_DATA_DIR=${LXDEBUILD_PREFIX}/share

LXDEBUILD_LIB_DIR=${LXDEBUILD_PREFIX}/lib

LXDEBUILD_LXPANEL_PLUGIN_DIR=${LXDEBUILD_LIB_DIR}/lxpanel/plugins

LXDEBUILD_DEPENDENCIES=glib-2.0 gtk+-2.0 gthread-2.0 lxpanel libcurl
 

#Environment variables, these should also not be edited.

export PKG_CONFIG_PATH=${LXDEBUILD_LIB_DIR}/pkgconfig

export LD_LIBRARY_PATH=${LXDEBUILD_LIB_DIR}

export LD_RUN_PATH=${LXDEBUILD_LIB_DIR}
 

#Phony target, invoked when make is run with no arguments.

#In this case it invokes the lxpanelplugin target without doing anything else.

all: lxpanelplugin
 

#Target to build your LXPanel plugin.

lxpanelplugin:
	gcc -shared -fPIC -g -O2  \
	-DPIC -DHAVE_CONFIG_H -DPACKAGE_DATA_DIR=\""${LXDEBUILD_DATA_DIR}"\" \
	`pkg-config --libs --cflags ${LXDEBUILD_DEPENDENCIES}` \
	-o ${LXDEBUILD_PLUGIN_NAME}.so \
	-Wl,-soname -Wl,${LXDEBUILD_PLUGIN_NAME}.so \
	${LXDEBUILD_SOURCEFILES}
