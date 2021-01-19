SR_PLUGIN_LUA:=0
SR_PROTO_HTTP_VERSION:=1.1
SR_SOCK_RXBUF_SIZE:=1024
SR_AGENT_VAL:=5
SR_REPORTER_NUM:=512
SR_REPORTER_VAL:=400
SR_REPORTER_RETRIES:=9
SR_CURL_SIGNAL:=1
SR_SSL_VERIFYCERT:=1
SR_FILEBUF_PAGE_SCALE:=3

BUILD:=debug
include init.mk

SRC_DIR:=src
BUILD_DIR:=build
SRC:=$(wildcard $(SRC_DIR)/*.cc)
MQTT_SRC_DIR:=ext/pahomqtt/MQTTPacket/src
MQTT_SRC:=$(wildcard $(MQTT_SRC_DIR)/*.c)

LIB_DIR:=lib
LIBNAME:=libsera.so
SONAME:=$(LIBNAME).1
REALNAME:=$(SONAME).2.7

CPPFLAGS+=-Iinclude -DSR_SOCK_RXBUF_SIZE=$(SR_SOCK_RXBUF_SIZE)
CPPFLAGS+=-DSR_AGENT_VAL=$(SR_AGENT_VAL) -DSR_REPORTER_NUM=$(SR_REPORTER_NUM)
CPPFLAGS+=-DSR_REPORTER_VAL=$(SR_REPORTER_VAL)
CPPFLAGS+=-DSR_REPORTER_RETRIES=$(SR_REPORTER_RETRIES)
CPPFLAGS+=-DSR_CURL_SIGNAL=$(SR_CURL_SIGNAL)
CPPFLAGS+=-DSR_SSL_VERIFYCERT=$(SR_SSL_VERIFYCERT)
CPPFLAGS+=-DSR_FILEBUF_PAGE_SCALE=$(SR_FILEBUF_PAGE_SCALE)
CFLAGS+=-fPIC -pipe -MMD
CXXFLAGS+=-std=c++11 -fPIC -pipe -pthread -MMD
LDFLAGS+=-Wl,-soname,$(SONAME) -Wl,--no-undefined -shared
LDLIBS+=-pthread -llua -llualib -llua5.2-c++

ifeq ($(SR_PLUGIN_LUA), 0)
SRC:=$(filter-out src/srluapluginmanager.cc,$(SRC))
endif

ifeq ($(SR_PROTO_HTTP_VERSION), 1.0)
CPPFLAGS+=-DSR_HTTP_1_0
endif

OBJ:=$(addprefix $(BUILD_DIR)/,$(notdir $(SRC:.cc=.o)))
OBJ+=$(addprefix $(BUILD_DIR)/,$(notdir $(MQTT_SRC:.c=.o)))

ifeq ($(BUILD), release)
CPPFLAGS+=-DNDEBUG
CXXFLAGS+=-O2 -ffast-math -flto
CFLAGS+=-O2 -ffast-math -flto
LDFLAGS+=-O2 -s
else
CPPFLAGS+=-DDEBUG
CXXFLAGS+=-O0 -g
CFLAGS+=-O0 -g
LDFLAGS+=-O0 -g
endif

.PHONY: all release clean test test_run

all: $(LIB_DIR)/$(REALNAME) bin/srwatchdogd
	@:

release:
	@make -s "BUILD=release"

WDT_LDFLAGS:=-flto -fno-exceptions -fno-rtti -fno-stack-protector
bin/srwatchdogd: $(SRC_DIR)/watchdog/srwatchdogd.cc
	@mkdir -p bin/
	@echo "(LD) $@"
	$(CXX) -std=c++11 -Os -s -DNDEBUG $^ $(WDT_LDFLAGS) -o $@

$(LIB_DIR)/$(REALNAME): $(OBJ)
	@mkdir -p $(LIB_DIR)
	@echo "(LD) $@"
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@cd $(LIB_DIR) && ln -fs $(REALNAME) $(SONAME)
	@cd $(LIB_DIR) && ln -fs $(SONAME) $(LIBNAME)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(BUILD_DIR)
	@echo "(CXX) $@"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(MQTT_SRC_DIR)/%.c
	@echo "(CC) $@"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

TEST_SRC:=$(wildcard tests/test_*.cc)
TEST_BIN:=$(addprefix bin/,$(notdir $(TEST_SRC:.cc=)))

bin/test_%: tests/test_%.cc
	@mkdir -p bin
	@$(CXX) $(CPPFLAGS) -DSR_AGENT_VAL=$(SR_AGENT_VAL) -DSR_REPORTER_NUM=$(SR_REPORTER_NUM) -g -pthread -std=c++11 -Iinclude -Llib $< -lsera -o $@

test: $(TEST_BIN)
	@$(foreach var,$^,LD_LIBRARY_PATH=lib $(var);)

clean:
	@rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d $(LIB_DIR)/$(LIBNAME)* bin/*

-include $(OBJ:.o=.d)
