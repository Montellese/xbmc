JSON_BUILDER = ../../../tools/JsonSchemaBuilder/src/JsonSchemaBuilder
GENERATED_JSON = ServiceDescription.h
JSON_SRC =  schema/version.txt schema/license.txt schema/methods.json schema/types.json schema/notifications.json


SRCS=AddonsOperations.cpp \
     ApplicationOperations.cpp \
     AudioLibrary.cpp \
     FavouritesOperations.cpp \
     FileItemHandler.cpp \
     FileOperations.cpp \
     GUIOperations.cpp \
     InputOperations.cpp \
     JSONRPC.cpp \
     JSONServiceDescription.cpp \
     PlayerOperations.cpp \
     PlaylistOperations.cpp \
     ProfilesOperations.cpp \
     PVROperations.cpp \
     SettingsOperations.cpp \
     SystemOperations.cpp \
     TextureOperations.cpp \
     VideoLibrary.cpp \
     XBMCOperations.cpp \

LIB=json-rpc.a

all: $(GENERATED_JSON) $(LIB)

$(GENERATED_JSON): $(JSON_BUILDER) $(JSON_SRC)
	$(JSON_BUILDER) $(JSON_SRC)

$(JSON_BUILDER):
	make -C ../../../tools/JsonSchemaBuilder/src

-include ../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))
