# vim: noexpandtab

CFLAGS +=
CXXFLAGS := $(CFLAGS) -std=gnu++17
DEFINES +=
INCLUDES := $(shell pkg-config --cflags libcurl tidy pugixml)
LIBS += $(shell pkg-config --libs libcurl tidy pugixml) -lpthread
PROJECT := rssfix
IMAGE := $(PROJECT)-$(BUILD)

SD := ../src
CD := ../contrib

SRC := \
	$(SD)/Apod.cpp \
	$(SD)/Curl.cpp \
	$(SD)/Engine.cpp \
	$(SD)/Handler.cpp \
	$(SD)/OpenSslThreading.cpp \
	$(SD)/RssFix.cpp \
	$(CD)/ini/ini.c


OBJDIRBASE := obj/$(BUILD)
OBJDIR := $(OBJDIRBASE)/o

OBJ_1 := $(addprefix $(OBJDIR)/,$(SRC:%.cpp=%.o))
OBJ := $(OBJ_1:%.c=%.o)

all: $(IMAGE)

$(OBJDIR)/%.o: %.cpp
	$(CXX) -c $(INCLUDES) $(CXXFLAGS) $(DEFINES) $< -o $@

$(OBJDIR)/%.o: %.c
	$(CC) -c $(INCLUDES) $(CFLAGS) $(DEFINES) $< -o $@

$(OBJDIR)/%.d : %.cpp
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CXX) -MM $(INCLUDES) $(CXXFLAGS) $(DEFINES) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(OBJDIR)/$(<:.cpp=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OBJDIR)/%.d : %.c
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CC) -MM $(INCLUDES) $(CFLAGS) $(DEFINES) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(OBJDIR)/$(<:.c=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(IMAGE): $(OBJ)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(OBJ) $(LIBS) -o $@

ifneq "$(MAKECMDGOALS)" "clean"
DEPS_1 := $(addprefix $(OBJDIR)/,$(SRC:.cpp=.d))
DEPS := $(DEPS_1:.c=.d)
-include $(DEPS)
endif

clean:
	rm -rf $(OBJDIRBASE) $(IMAGE)*

.PHONY: clean all
