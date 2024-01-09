ifeq "$(ROOT)" ""
where-am-i = $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
THIS_MAKEFILE := $(call where-am-i)
ROOT := $(dir $(THIS_MAKEFILE))
endif
ROOT:=$(shell echo $(ROOT) | sed -e 's/\/\//\//g' -e 's/\/$$//g' )

$(info ROOT $(ROOT))

all:
	@$(MAKE) -C $(ROOT)/src ROOT=$(ROOT) all

clean:
	@$(MAKE) -C $(ROOT)/src ROOT=$(ROOT) clean

