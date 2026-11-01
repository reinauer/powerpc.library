#include "compatibility.h"
#include "internal.h"
#include "resource_allocator.h"
#include <proto/dos.h>
#include <proto/exec.h>
#include <stdio.h>
#include <string.h>

/* Loads the built PPCInit through the library's own tool loader, against a
 * library base whose vectors reach the real board functions, then takes the
 * reservations through BAR probing and allocation. */
#define MIB                 0x100000UL
#define VECTOR_BYTES        516
#define LVO_FIND_DEVICE     90
#define LVO_GET_ATTRIBUTES  210
#define LVO_SET_ATTRIBUTES  216
#define MEMORY_LOW          0x100000UL
#define MEMORY_HIGH         0x20000000UL
#define IO_LOW              0x6000UL
#define IO_HIGH             0xff00UL

#define CHECK(actual, expected) do { ULONG a_ = (ULONG)(actual), e_ = (ULONG)(expected); \
    ++checks; if (a_ != e_) { ++failures; \
    printf("FAIL line %d: %s = %08lx, expected %08lx\n", __LINE__, #actual, a_, e_); } } while (0)

static UBYTE base_storage[VECTOR_BYTES + sizeof(struct cp_base)] __attribute__((aligned(4)));
static struct cp_base *const library =
    (struct cp_base *)(base_storage + VECTOR_BYTES);
static struct cp_bridge host;
static struct cp_node card, other;
static ULONG card_config[64], other_config[64];
static unsigned checks, failures;

static void initialize_list(struct MinList *list)
{
    list->mlh_Head = (struct MinNode *)&list->mlh_Tail;
    list->mlh_Tail = NULL;
    list->mlh_TailPred = (struct MinNode *)&list->mlh_Head;
}

/* Vamos rejects FreeArgs(NULL) and empty RDArgs, although AmigaDOS accepts
 * both. Forward populated arguments and retain the native no-op cases. */
typedef void (*free_arguments_entry)(CP_REG(struct RDArgs *,arguments,d1),
                                     CP_REG(struct Library *,dos,a6));
static free_arguments_entry original_free_arguments;
static void free_arguments_compatible(CP_REG(struct RDArgs *,arguments,d1),
                                      CP_REG(struct Library *,dos,a6))
{
    if (arguments && arguments->RDA_Buffer) {
        original_free_arguments(arguments,dos);
        arguments->RDA_Buffer = NULL; arguments->RDA_BufSiz = 0;
    }
}

static void prepare_configuration(void) { }
static void finalize_device(CP_REG(struct cp_node *,device,a0),CP_BASE)
{
    (void)device; (void)base;
}
static ULONG execute_configuration(CP_REG(struct cp_node *,device,a0),
    CP_REG(cp_operation,operation,a1),CP_REG(ULONG,offset,d4),
    CP_REG(ULONG,value,d5),CP_BASE)
{
    return operation(device,offset,value,base);
}

static void set_vector(unsigned offset, APTR function)
{
    UBYTE *vector = (UBYTE *)library - offset;
    *(UWORD *)vector = 0x4ef9;
    *(APTR *)(vector + 2) = function;
}

static void add_device(struct cp_node *device, ULONG *config, UWORD vendor,
                       UWORD device_id, UBYTE slot)
{
    memset(device,0,sizeof(*device));
    device->type = 25; device->config = config;
    device->parent = device->host = (struct cp_node *)&host;
    device->dev.vendor = vendor; device->dev.device = device_id;
    device->dev.devfn = slot << 3; device->slot = slot;
    device->public_bus = 1; device->flags = 1;
    device->memory_low = host.memory_min; device->memory_high = host.allocation_high;
    initialize_list(&device->children);
    AddTail((struct List *)&host.children,(struct Node *)device);
}

static void initialize(UWORD vendor, UWORD device_id, ULONG memory_high)
{
    memset(base_storage + VECTOR_BYTES,0,sizeof(struct cp_base));
    memset(&host,0,sizeof(host));
    memset(card_config,0,sizeof(card_config));
    memset(other_config,0,sizeof(other_config));
    library->sys = SysBase; library->dos = (struct Library *)DOSBase;
    library->page_size = 4096;
    set_vector(LVO_FIND_DEVICE,cp_pci_find_device);
    set_vector(LVO_GET_ATTRIBUTES,cp_GetBoardAttrsA);
    set_vector(LVO_SET_ATTRIBUTES,cp_SetBoardAttrsA);
    initialize_list(&library->bridges); initialize_list(&host.children);
    AddTail((struct List *)&library->bridges,(struct Node *)&host);
    host.type = 24; host.name = (STRPTR)"Fixture";
    host.memory_min = host.allocation_low = MEMORY_LOW;
    host.allocation_high = memory_high;
    host.io_low = IO_LOW; host.io_high = IO_HIGH;
    host.prepare_read = host.prepare_write = prepare_configuration;
    host.config_access = execute_configuration;
    host.finalize_device = (APTR)finalize_device;
    add_device(&card,card_config,vendor,device_id,1);
    add_device(&other,other_config,0x1234,0x5678,2);
}

static LONG run_line(const char *line)
{
    char text[516];
    struct RDArgs *arguments = AllocDosObject(DOS_RDARGS,NULL);
    LONG result;
    strcpy(text,line);
    result = pci_execute_configuration_line(text,arguments,1,library);
    FreeDosObject(DOS_RDARGS,arguments);
    return result;
}

static void check_sizes(ULONG size0, ULONG size1, ULONG size2, ULONG size3,
                        ULONG size4)
{
    CHECK(card.sizes[0],size0); CHECK(card.sizes[1],size1);
    CHECK(card.sizes[2],size2); CHECK(card.sizes[3],size3);
    CHECK(card.sizes[4],size4); CHECK(card.sizes[5],0);
    CHECK(other.sizes[0],0);
}

static void test_reservations(void)
{
    static const char *const harrier_ignores[] = {
        "Ignore DEVICE=1057:480b BAR=0\n", "Ignore DEVICE=1057:480b BAR=1\n",
        "Ignore DEVICE=1057:480b BAR=2\n", "Ignore DEVICE=1057:480b BAR=3\n",
        "Ignore DEVICE=1057:480b BAR=4\n"
    };
    unsigned i;
    initialize(0x1057,0x480b,MEMORY_HIGH);
    for (i = 0; i < 5; ++i) CHECK(run_line(harrier_ignores[i]),0);
    CHECK(run_line("PPCInit\n"),0);
    check_sizes(0x1000,0x1000,256*MIB,0x40000,CP_RESOURCE_IGNORED);
    CHECK(card.dev.master >> CP_DEVICE_BAR_NO_READBACK_SHIFT,0x0f);
    CHECK(card.dev.base_size[2],0UL - 256*MIB);

    initialize(0x1057,0x0004,MEMORY_HIGH);
    CHECK(run_line("PPCInit RAM=128\n"),0);
    check_sizes(128*MIB,0,0,0,0);

    /* The Killer NIC's RAM window is fixed, whatever RAM asks for. */
    initialize(0x1957,0x0086,MEMORY_HIGH);
    CHECK(run_line("PPCInit RAM=128\n"),0);
    check_sizes(0,64*MIB,0,0,0);
}

static void test_refusals(void)
{
    initialize(0x1234,0x9999,MEMORY_HIGH);
    CHECK(run_line("PPCInit\n"),0);
    check_sizes(0,0,0,0,0);

    initialize(0x1057,0x0004,MEMORY_HIGH);
    CHECK(run_line("PPCInit RAM=100\n"),20);
    CHECK(run_line("PPCInit RAM=0\n"),20);
    CHECK(run_line("PPCInit RAM=2048\n"),20);
    CHECK(run_line("PPCInit SIZE=64\n"),20);
    check_sizes(0,0,0,0,0);

    /* A card that could not be placed would be dropped: leave it alone. */
    initialize(0x1057,0x480b,MEMORY_LOW + 128*MIB);
    CHECK(run_line("Ignore DEVICE=1057:480b BAR=2\n"),0);
    CHECK(run_line("PPCInit\n"),10);
    check_sizes(0,0,CP_RESOURCE_IGNORED,0,0);
}

static void test_allocation(void)
{
    initialize(0x1057,0x480b,MEMORY_HIGH);
    CHECK(run_line("Ignore DEVICE=1057:480b BAR=4\n"),0);
    CHECK(run_line("PPCInit\n"),0);
    CHECK(pci_probe_device_resources(&card,library) != 0,1);
    CHECK(pci_probe_device_resources(&other,library) != 0,1);
    /* Fixture configuration space is plain memory, so only presets are sized. */
    CHECK(card.sizes[0],0x1000); CHECK(card.sizes[1],0x1000);
    CHECK(card.sizes[2],256*MIB); CHECK(card.sizes[3],0x40000);
    CHECK(card.sizes[4],0);
    pci_allocate_host_resources((struct cp_node *)&host,library);
    CHECK(card.dev.master & CP_DEVICE_ALLOCATION_FAILED,0);
    CHECK(card.dev.base_address[2] & (256*MIB - 1),0);
    CHECK(card.dev.base_address[2] != 0,1);
    CHECK(card.dev.base_address[3] & 0x3ffff,0);
    CHECK(card.dev.base_address[3] != 0,1);
    CHECK(card.dev.base_address[0] != 0,1);
    CHECK(card.dev.base_address[1] != 0,1);
    CHECK(card.dev.base_address[4],0);
    CHECK(__builtin_bswap32(card_config[6]) == card.dev.base_address[2] ||
          card_config[6] == card.dev.base_address[2],1);
}

int main(void)
{
    BPTR directory = Lock((STRPTR)"build/Libs/PCI",SHARED_LOCK);
    BPTR previous;
    if (!directory) { printf("FAIL: no build/Libs/PCI\n"); return 20; }
    previous = CurrentDir(directory);
    original_free_arguments = (free_arguments_entry)
        SetFunction((struct Library *)DOSBase,-858,(APTR)free_arguments_compatible);
    test_reservations();
    test_refusals();
    test_allocation();
    SetFunction((struct Library *)DOSBase,-858,(APTR)original_free_arguments);
    CurrentDir(previous); UnLock(directory);
    printf("PPCInit: %u checks, %u failures\n",checks,failures);
    return failures ? 20 : 0;
}
