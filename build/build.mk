# vim: noexpandtab

CFLAGS +=
CXXFLAGS := $(CFLAGS) -std=gnu++17
DEFINES +=
INCLUDES := $(shell pkg-config --cflags libcurl)
LIBS += $(shell pkg-config --libs libcurl) -lpthread
PROJECT := rssfix
IMAGE := $(PROJECT)-$(BUILD)

SD := ../src

SRC := \
	$(SD)/OpenSslThreading.cpp \
	$(SD)/RssFix.cpp

OBJDIRBASE := obj/$(BUILD)
OBJDIR := $(OBJDIRBASE)/o

OBJ := $(addprefix $(OBJDIR)/,$(SRC:%.cpp=%.o))

all: $(IMAGE)

$(OBJDIR)/%.o: %.cpp
	$(CXX) -c $(INCLUDES) $(CXXFLAGS) $(DEFINES) $< -o $@

$(OBJDIR)/%.d : %.cpp
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CXX) -MM $(INCLUDES) $(CXXFLAGS) $(DEFINES) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(OBJDIR)/$(<:.cpp=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(IMAGE): $(OBJ) $(OBJ2) $(OBJ3)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(OBJ) $(OBJ2) $(OBJ3) $(LIBS) -o $@

ifneq "$(MAKECMDGOALS)" "clean"
-include $(addprefix $(OBJDIR)/,$(SRC:.cpp=.d))
endif

clean:
	rm -rf $(OBJDIRBASE) $(IMAGE)*

.PHONY: clean all
