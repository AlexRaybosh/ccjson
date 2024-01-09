
$(info COMPILING FOR $(ARCH))
BUILD:=$(ROOT)/build$(SUFFIX)/$(ARCH)
JSON_CONF:=$(ROOT)/default.conf.json
JSON_CONF_OBJ:=$(BUILD)/conf_json.o

$(shell mkdir -p $(BUILD))
SRC:=$(wildcard *.cc)
OBJS:=$(patsubst %.cc, $(BUILD)/%.o, $(SRC))
UNIT_TESTS_SRC:=$(wildcard unit_tests/*.cc)
UNIT_TESTS_OBJS:=$(patsubst %.cc, $(BUILD)/%.o, $(UNIT_TESTS_SRC))
UNIT_TESTS_EXE:=$(patsubst %.cc, $(BUILD)/%, $(UNIT_TESTS_SRC))
ALL_NO_MAIN_OBJS:=$(filter-out $(BUILD)/main.o,$(OBJS))

ADHOC_TESTS_SRC:=$(wildcard adhoc_tests/*.cc)
ADHOC_TESTS_OBJS:=$(patsubst %.cc, $(BUILD)/%.o, $(ADHOC_TESTS_SRC))
ADHOC_TESTS_EXE:=$(patsubst %.cc, $(BUILD)/%, $(ADHOC_TESTS_SRC))


H:=$(wildcard *.h)
$(info Sources: $(SRC))
$(info Headers: $(H))
$(info Objects: $(OBJS))
$(info All non-main objects: $(ALL_NO_MAIN_OBJS))

$(info unit_tests Sources: $(UNIT_TESTS_SRC))
$(info unit_tests Objects: $(UNIT_TESTS_OBJS))
$(info unit_tests Exec: $(UNIT_TESTS_EXE))




$(info adhoc_tests Sources: $(UNIT_TESTS_SRC))
$(info adhoc_tests Objects: $(UNIT_TESTS_OBJS))
$(info adhoc_tests Exec: $(UNIT_TESTS_EXE))


SYSROOT:=$(ROOT)/sysroot/$(ARCH)
LDFLAGS:=--sysroot=$(SYSROOT)
CXXFLAGS:=$(OPT) \
	-I$(ROOT)/src \
	-Wall -fmessage-length=0 -Wno-pessimizing-move -Wno-unused-result \
	--sysroot=$(SYSROOT) \
	-I$(SYSROOT)/usr/include \
	-D_REENTRANT \
	-D_POSIX_C_SOURCE=202001L -D_XOPEN_SOURCE=600 -fPIC -pthread \
	-std=c++17

ifeq "$(ARCH)" "armhf"
	CXX:=arm-linux-gnueabihf-g++
	AR:=arm-linux-gnueabihf-ar
	LDFLAGS:=$(LDFLAGS) -L$(SYSROOT)/lib/arm-linux-gnueabihf -L$(SYSROOT)/usr/lib/arm-linux-gnueabihf
	LDFLAGS:=$(LDFLAGS) -static-libstdc++ -static-libgcc -Wl,-Bstatic -ljansson -Wl,-Bdynamic
	OBJCOPY:=/usr/bin/arm-linux-gnueabihf-objcopy
	OBJCOPY_ARGS:= -v --input-target binary --output-target elf32-littlearm
	TESTER:= qemu-arm
	STRIP:=arm-linux-gnueabihf-strip
else ifeq "$(ARCH)" "armel"
	CXX:=arm-linux-gnueabi-g++
	AR:=arm-linux-gnueabi-ar
	LDFLAGS:=$(LDFLAGS) -L /usr/lib/arm-linux-gnueabi -L$(SYSROOT)/lib/arm-linux-gnueabihf
	LDFLAGS:=$(LDFLAGS) -Wl,-Bstatic -ljansson -Wl,-Bdynamic	
	OBJCOPY:=/usr/bin/arm-linux-gnueabi-objcopy
	OBJCOPY_ARGS:= --input-target binary --output-target elf32-littlearm
	TESTER:= qemu-arm
	STRIP:=arm-linux-gnueabi-strip
else ifeq "$(ARCH)" "x86_64"
	CXX:=g++
	AR:=ar
	LDFLAGS:=-L /usr/lib/x86_64-linux-gnu -Wl,-Bstatic -ljansson -Wl,-Bdynamic	
	OBJCOPY:=objcopy
	OBJCOPY_ARGS:= --input-target binary --output-target elf64-x86-64
	TESTER:=
	STRIP:=strip
	CXXFLAGS:=$(OPT) \
		-I$(ROOT)/src \
		-Wall -fmessage-length=0 -Wno-pessimizing-move -Wno-unused-result \
		-D_REENTRANT \
		-D_POSIX_C_SOURCE=202001L -D_XOPEN_SOURCE=600 -fPIC -pthread \
		-std=c++17	
endif

$(info CXX $(CXX))
$(info LDFLAGS $(LDFLAGS))
$(info CXXFLAGS $(CXXFLAGS))

$(JSON_CONF_OBJ) : $(JSON_CONF)
	sh -c "cp $(JSON_CONF) $(BUILD)/conf_json && cd $(BUILD) && $(OBJCOPY) $(OBJCOPY_ARGS) conf_json $@ && rm conf_json"

$(BUILD)/%.o : %.cc
	@echo "Processing " $< " into " $@
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/unit_tests/%.o : unit_tests/%.cc
	echo "Processing " $< " into " $@
	mkdir -p $(BUILD)/unit_tests
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/unit_tests/% : $(BUILD)/unit_tests/%.o $(ALL_NO_MAIN_OBJS)
	echo "Processing " $< " into " $@
	$(CXX) -o $@ $(ALL_NO_MAIN_OBJS) $(JSON_CONF_OBJ) $< $(LDFLAGS)
	@echo "Built: " $@
	@$(TESTER) $@ || { echo Test $@ Failed!!! ; exit 1; }


$(BUILD)/adhoc_tests/%.o : adhoc_tests/%.cc
	echo "Processing " $< " into " $@
	mkdir -p $(BUILD)/adhoc_tests
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/adhoc_tests/% : $(BUILD)/adhoc_tests/%.o $(ALL_NO_MAIN_OBJS)
	echo "Processing " $< " into " $@
	$(CXX) -o $@ $(ALL_NO_MAIN_OBJS) $(JSON_CONF_OBJ) $< $(LDFLAGS)
	@echo "Built: " $@

#mkdir -p $(@D)
	

$(BUILD)/unit_tests/% : $(OBJ)

$(BUILD)/adhoc_tests/% : $(OBJ)  

$(OBJS) : $(SRC) $(H) Makefile arch.mk

$(UNIT_TESTS_OBJS) : $(UNIT_TESTS_SRC) $(OBJS) $(JSON_CONF_OBJ) $(SRC) $(H) Makefile arch.mk

$(ADHOC_TESTS_OBJS) : $(ADHOC_TESTS_SRC) $(OBJS) $(JSON_CONF_OBJ) $(SRC) $(H) Makefile arch.mk


$(BUILD)/libccjson.a : $(ALL_NO_MAIN_OBJS)
	$(AR) -rc $@  $(ALL_NO_MAIN_OBJS)

$(BUILD)/libccjson.so : $(ALL_NO_MAIN_OBJS)
	$(CXX) -shared -o $@ $< $(LDFLAGS)
ifeq "$(SUFFIX)" ""
	$(STRIP) $@
endif

#$(UNIT_TESTS_EXE) : $(EXECUTABLE)

all: $(UNIT_TESTS_EXE) $(ADHOC_TESTS_EXE) $(BUILD)/libccjson.a $(BUILD)/libccjson.so


clean:
	rm -rf $(BUILD)/*
	
.PHONY: clean
