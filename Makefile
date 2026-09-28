#---------------------------------------------------------------------------------
# nextendo-nx — Makefile (devkitA64 + libnx + Aether GUI, trimmed romfs).
#
# Aether (lib/Aether) is built as a static library and linked in. It pulls SDL2 +
# SDL2_ttf/gfx/image and FreeType from the portlibs. Exceptions and RTTI are ON
# (Aether needs them) — unlike the old Prelude which disabled both.
#
# NOTE: this project has NOT been compiled in the environment it was authored in
# (no devkitPro/Aether toolchain there). See NOTES.md. Build with:
#   make -C lib/Aether        # build libaether.a first
#   make -j$(nproc)
#---------------------------------------------------------------------------------
.SUFFIXES:

ifeq ($(strip $(DEVKITPRO)),)
$(error "Set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/libnx/switch_rules

#---------------------------------------------------------------------------------
TARGET   := nextendo-nx
BUILD    := build
SOURCES  := source source/core source/ui
DATA     := data
INCLUDES := include source lib/Aether/include
ROMFS    := romfs

APP_TITLE   := Nextendo
APP_AUTHOR  := Nextendo Network
APP_VERSION := 1.0.0
APP_ICON    := icon.jpg

#---------------------------------------------------------------------------------
ARCH := -march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE

# Version is derived from APP_VERSION and nothing else (single source of truth).
DEFINES := -DNEXTENDO_VERSION_MAJOR=$(word 1,$(subst ., ,$(APP_VERSION))) \
           -DNEXTENDO_VERSION_MINOR=$(word 2,$(subst ., ,$(APP_VERSION))) \
           -DNEXTENDO_VERSION_PATCH=$(word 3,$(subst ., ,$(APP_VERSION)))
# nx-mod: build for a nextendo-testing LAN stack (include/nextendo/config.hpp).
ifneq ($(LAN_HOST),)
DEFINES += -DNEXTENDO_LAN=1 -DNEXTENDO_SERVER_IP_DEFAULT=\"$(LAN_HOST)\" -DNEXTENDO_SERVER_IP_MK8=\"$(LAN_HOST)\" \
           -DNEXTENDO_SERVER_IP_NNCS2=\"$(if $(LAN_HOST2),$(LAN_HOST2),$(LAN_HOST))\"
endif

CFLAGS := -g -Wall -Wextra -O2 -ffunction-sections -fstack-protector-strong \
          $(ARCH) $(DEFINES) $(INCLUDE) -D__SWITCH__
CFLAGS += -I$(PORTLIBS)/include/freetype2 -I$(PORTLIBS)/include/SDL2

# C++17, exceptions + RTTI ON for Aether.
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-rtti -fno-exceptions

ASFLAGS := -g $(ARCH)
LDFLAGS  = -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

# Aether static lib first, then SDL2 stack + FreeType + codecs + libnx.
LIBS := -lAether \
        -lSDL2_ttf -lSDL2_gfx -lSDL2_image -lSDL2 \
        -lEGL -lglapi -ldrm_nouveau \
        -lfreetype -lharfbuzz -lfreetype \
        -lpng -ljpeg -lwebp -lbz2 -lz \
        -lnx -lm -lstdc++

LIBDIRS := $(PORTLIBS) $(LIBNX) $(TOPDIR)/lib/Aether

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
export OUTPUT   := $(CURDIR)/$(TARGET)
export TOPDIR   := $(CURDIR)
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                   $(foreach dir,$(DATA),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))

ifeq ($(strip $(CPPFILES)),)
    export LD := $(CC)
else
    export LD := $(CXX)
endif

export OFILES    := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)
export INCLUDE   := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                    $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                    -I$(CURDIR)/$(BUILD)
export LIBPATHS  := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export BUILD_EXEFS_SRC := $(TOPDIR)/$(EXEFS_SRC)

ifeq ($(strip $(ROMFS)),)
else
    export APP_ROMFS := $(TOPDIR)/$(ROMFS)
endif

# elf2nro asset flags (icon + nacp + romfs). Without these the .nro is built
# with no asset section, so the bundled font and the cert-trust patches under
# romfs/ never reach the console. switch_rules uses NROFLAGS but does not define
# it — the application Makefile must.
export NROFLAGS := --icon=$(CURDIR)/$(APP_ICON) --nacp=$(OUTPUT).nacp
ifneq ($(strip $(APP_ROMFS)),)
    export NROFLAGS += --romfsdir=$(APP_ROMFS)
endif

.PHONY: $(BUILD) clean all
all: $(BUILD)
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).nro $(TARGET).nacp $(TARGET).elf
#---------------------------------------------------------------------------------
else
.PHONY: all
DEPENDS := $(OFILES:.o=.d)
all: $(OUTPUT).nro
$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf: $(OFILES)
$(OFILES_SRC): $(HFILES_BIN)
-include $(DEPENDS)
endif
#---------------------------------------------------------------------------------
