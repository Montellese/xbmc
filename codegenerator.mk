TOPDIR ?= .
INTERFACES_DIR ?= xbmc/interfaces

JAVA ?= $(shell which java)
ifeq ($(JAVA),)
JAVA = java-not-found
endif

SWIG ?= $(shell which swig)
ifeq ($(SWIG),)
SWIG = swig-not-found
endif

DOXYGEN ?= $(shell which doxygen)
ifeq ($(DOXYGEN),)
DOXYGEN = doxygen-not-found
else
DOXY_XML_PATH=$(GENDIR)/doxygenxml
endif

JSON_BUILDER = tools/JsonSchemaBuilder/src/JsonSchemaBuilder
GENERATED_JSON = $(INTERFACES_DIR)/json-rpc/ServiceDescription.h
JSON_SRC =  $(INTERFACES_DIR)/json-rpc/schema/version.txt
JSON_SRC += $(INTERFACES_DIR)/json-rpc/schema/license.txt
JSON_SRC += $(INTERFACES_DIR)/json-rpc/schema/methods.json
JSON_SRC += $(INTERFACES_DIR)/json-rpc/schema/types.json
JSON_SRC += $(INTERFACES_DIR)/json-rpc/schema/notifications.json

GENDIR = $(INTERFACES_DIR)/python/generated
GROOVY_DIR = $(TOPDIR)/lib/groovy

GENERATED =  $(GENDIR)/AddonModuleXbmc.cpp
GENERATED += $(GENDIR)/AddonModuleXbmcgui.cpp
GENERATED += $(GENDIR)/AddonModuleXbmcplugin.cpp
GENERATED += $(GENDIR)/AddonModuleXbmcaddon.cpp
GENERATED += $(GENDIR)/AddonModuleXbmcvfs.cpp

GENERATE_DEPS += $(TOPDIR)/xbmc/interfaces/legacy/*.h $(TOPDIR)/xbmc/interfaces/python/typemaps/*.intm $(TOPDIR)/xbmc/interfaces/python/typemaps/*.outtm

vpath %.i $(INTERFACES_DIR)/swig

$(GENDIR)/%.cpp: $(GENDIR)/%.xml $(JAVA) $(SWIG) $(DOXY_XML_PATH)
	# Work around potential groovy bug reported at: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=733234
	$(JAVA) -cp "$(GROOVY_DIR)/groovy-all-2.1.7.jar:$(GROOVY_DIR)/commons-lang-2.6.jar:$(TOPDIR)/tools/codegenerator:$(INTERFACES_DIR)/python" \
          org.codehaus.groovy.tools.FileSystemCompiler -d $(TOPDIR)/tools/codegenerator $(TOPDIR)/tools/codegenerator/Helper.groovy  $(TOPDIR)/tools/codegenerator/SwigTypeParser.groovy $(INTERFACES_DIR)/python/MethodType.groovy $(INTERFACES_DIR)/python/PythonTools.groovy
	$(JAVA) -cp "$(GROOVY_DIR)/groovy-all-2.1.7.jar:$(GROOVY_DIR)/commons-lang-2.6.jar:$(TOPDIR)/tools/codegenerator:$(INTERFACES_DIR)/python" \
          groovy.ui.GroovyMain $(TOPDIR)/tools/codegenerator/Generator.groovy $< $(INTERFACES_DIR)/python/PythonSwig.cpp.template $@ $(DOXY_XML_PATH)
	rm $<

$(GENDIR)/%.xml: %.i $(SWIG) $(JAVA) $(GENERATE_DEPS)
	mkdir -p $(GENDIR)
	$(SWIG) -w401 -c++ -o $@ -xml -I$(TOPDIR)/xbmc -xmllang python $<

codegenerated: $(DOXYGEN) $(SWIG) $(JAVA) $(GENERATED) $(GENERATED_JSON)

$(DOXY_XML_PATH): $(SWIG) $(JAVA)
	cd $(INTERFACES_DIR)/python; ($(DOXYGEN) Doxyfile > /dev/null) 2>&1 | grep -v " warning: "
	touch $@

$(DOXYGEN):
	@echo "Warning: No doxygen installed. The Api will not have any docstrings."
	mkdir -p $(GENDIR)/doxygenxml

$(JAVA):
	@echo Java not found, it will be used if found after configure.
	@echo This is not necessarily an error.
	@false

$(SWIG):
	@echo Swig not found, it will be used if found after configure.
	@echo This is not necessarily an error.
	@false

$(GENERATED_JSON): $(JSON_BUILDER)
	@echo Generating JSON header
	$(JSON_BUILDER) $(JSON_SRC) && mv ServiceDescription.h $@

$(JSON_BUILDER):
	make -C tools/JsonSchemaBuilder/src
