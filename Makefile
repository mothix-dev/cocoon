CC ?= clang
LD ?= ld.lld
MKDEP ?= bmkdep

CFLAGS += -O2 -Isrc -Iprintf -nostdlib -fno-builtin -ffreestanding -fno-stack-protector -static -Wall -mregparm=3 -DPRINTF_DISABLE_SUPPORT_LONG_LONG -DPLATFORM=$(PLATFORM)

PLATFORM_PATH = src/platform/$(PLATFORM)

MULTIBOOT_X86_CFLAGS != [ "$(PLATFORM)" = multiboot-x86 ] && echo "-target i586-unknown-none -march=i586 -DPAGE_SIZE=4096 -DBITS=32" || echo ""
OPENFIRMWARE_POWERPC_CFLAGS != [ "$(PLATFORM)" = openfirmware-powerpc ] && echo "-target ppc32-unknown-none -mcpu=601 -DPAGE_SIZE=4096 -DBITS=32" || echo ""
OPENFIRMWARE_X86_CFLAGS != [ "$(PLATFORM)" = openfirmware-x86 ] && echo "-target i586-unknown-none -march=i586 -DPAGE_SIZE=4096 -DBITS=32" || echo ""
CFLAGS += $(MULTIBOOT_X86_CFLAGS) $(OPENFIRMWARE_POWERPC_CFLAGS) $(OPENFIRMWARE_X86_CFLAGS)

MULTIBOOT_X86_LDFLAGS != [ "$(PLATFORM)" = multiboot-x86 ] && echo "-T$(PLATFORM_PATH)/kernel.ld" || echo ""
LDFLAGS += $(MULTIBOOT_X86_LDFLAGS)

PLATFORM_SOURCES != find $(PLATFORM_PATH) -maxdepth 1 -name "*.c" -o -name "*.S"
COMMON_SOURCES != find src -maxdepth 1 -name "*.c"

OPENFIRMWARE_SOURCES != [ -n "`echo $(PLATFORM) | grep -e 'openfirmware-.*'`" ] && echo "src/platform/openfirmware.c" || echo ""
PLATFORM_SOURCES += $(OPENFIRMWARE_SOURCES)

X86_SOURCES != [ -n "`echo $(PLATFORM) | grep -e '.*-x86'`" ] && echo "src/platform/x86.c" || echo ""
POWERPC_SOURCES != [ -n "`echo $(PLATFORM) | grep -e '.*-powerpc'`" ] && echo "src/platform/powerpc.c" || echo ""
PLATFORM_SOURCES += $(X86_SOURCES) $(POWERPC_SOURCES)

SOURCE_FILES = $(COMMON_SOURCES) $(PLATFORM_SOURCES) printf/printf.c
C_OBJECTS = $(SOURCE_FILES:.c=.o)
OBJECTS = $(C_OBJECTS:.S=.o)

BINARY = cocoon

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(BINARY)

depend:
	$(MKDEP) $(CFLAGS) $(SOURCE_FILES)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.S.o:
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-find . -name "*.o" | xargs -n1 rm
	-rm $(BINARY)
	-rm .depend
