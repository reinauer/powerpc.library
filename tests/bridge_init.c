/* Exercise the built library's actual 68k LibInit entry with mock library
 * vectors. Stop at a missing PPC/VGA card before any hardware access. */
#include <exec/execbase.h>
#include <exec/resident.h>
#include <libraries/configvars.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <stdio.h>
#include <string.h>

#define REG(type, name, reg) register type name __asm(#reg)
#define CHECK(c) do { ++checks; if (!(c)) { ++failures; \
    printf("FAIL line %u: %s\n", __LINE__, #c); } } while (0)
#define HOST_OFFSET 0x40000000UL
#define HOST_REGISTERS (HOST_OFFSET + 0x1fc08000UL)
#define FIRESTORM_ID 0x0e3b00c8UL
#define TAG_VENDOR 0x6eda0000UL
#define TAG_DEVICE 0x6eda0001UL
#define TAG_OFFSET 0x6eda000fUL

static struct { UBYTE vectors[768]; struct ExecBase base; } exec_fixture;
static struct { UBYTE vectors[1024]; struct Library base; } libs[5];
enum { DOS, EXPANSION, PCI, PROMETHEUS, INTUITION };
static struct ConfigDev boards[4];
static unsigned board_count, checks, failures, requests, bad_closes;
static unsigned opened[5], closed[5], pci_available, prom_available, hide_on_open;
static unsigned fail_dos, fail_expansion, selected_cards, expected_requests;
static char error_text[160];
struct card { ULONG vendor, device, offset, has_offset; };
static struct card cards[4];
static unsigned card_count;

static void vector(APTR base, unsigned lvo, APTR function)
{
    UBYTE *slot = (UBYTE *)base - lvo;
    *(UWORD *)slot = 0x4ef9;
    *(APTR *)(slot + 2) = function;
}

static struct Library *open_library(REG(const char *, name, a1))
{
    int id = -1;
    if (!strcmp(name, "dos.library") && !fail_dos) id = DOS;
    if (!strcmp(name, "utility.library")) return NULL;
    if (!strcmp(name, "expansion.library") && !fail_expansion) id = EXPANSION;
    if (!strcmp(name, "openpci.library") && pci_available) {
        id = PCI;
        if (hide_on_open) {
            boards[0].cd_Rom.er_Manufacturer = 1731;
            boards[0].cd_Rom.er_Product = 0;
            boards[0].cd_Rom.er_SerialNumber = FIRESTORM_ID;
            boards[0].cd_BoardAddr = (APTR)HOST_REGISTERS;
            boards[0].cd_BoardSize = 4;
        }
    }
    if (!strcmp(name, "prometheus.library") && prom_available) id = PROMETHEUS;
    if (!strcmp(name, "intuition.library")) id = INTUITION;
    if (id < 0) return NULL;
    ++opened[id];
    return &libs[id].base;
}

static void close_library(REG(struct Library *, base, a1))
{
    unsigned i;
    for (i = 0; i < 5; ++i) if (base == &libs[i].base) {
        ++closed[i];
        if (closed[i] > opened[i]) ++bad_closes;
        return;
    }
    ++bad_closes;
}

static APTR find_name(void) { return NULL; }
static LONG get_var(void) { return -1; }

static struct ConfigDev *find_config(REG(struct ConfigDev *, prev, a0),
                                    REG(ULONG, vendor, d0), REG(ULONG, product, d1))
{
    unsigned i = prev ? (unsigned)(prev - boards) + 1 : 0;
    for (; i < board_count; ++i)
        if (boards[i].cd_Rom.er_Manufacturer == vendor &&
            boards[i].cd_Rom.er_Product == product) return boards + i;
    return NULL;
}

static LONG requester(REG(struct EasyStruct *, request, a1))
{
    ++requests;
    snprintf(error_text, sizeof(error_text), "%s", request->es_TextFormat);
    return 1;
}

static struct card *find_card(REG(struct card *, prev, a0),
                              REG(struct TagItem *, tags, a1))
{
    ULONG vendor = ~0UL, device = ~0UL;
    unsigned i = prev ? (unsigned)(prev - cards) + 1 : 0;
    for (; tags->ti_Tag; ++tags) {
        if (tags->ti_Tag == TAG_VENDOR) vendor = tags->ti_Data;
        if (tags->ti_Tag == TAG_DEVICE) device = tags->ti_Data;
    }
    for (; i < card_count; ++i)
        if (cards[i].vendor == vendor && cards[i].device == device) {
            ++selected_cards;
            return cards + i;
        }
    return NULL;
}

static ULONG card_attrs(REG(struct card *, card, a0),
                        REG(struct TagItem *, tags, a1))
{
    ULONG count = 0;
    for (; tags->ti_Tag; ++tags)
        if (tags->ti_Tag == TAG_OFFSET && card->has_offset) {
            *(ULONG *)tags->ti_Data = card->offset;
            ++count;
        } else if ((tags->ti_Tag >= 0x6eda0010 && tags->ti_Tag <= 0x6eda0015) ||
                   (tags->ti_Tag >= 0x6eda0020 && tags->ti_Tag <= 0x6eda0025)) {
            *(ULONG *)tags->ti_Data = card->vendor == 0x121a ? 0x50000000 : 0;
            ++count;
        }
    return count;
}

static void reset(void)
{
    memset(&exec_fixture, 0, sizeof(exec_fixture));
    memset(libs, 0, sizeof(libs));
    memset(boards, 0, sizeof(boards));
    memset(cards, 0, sizeof(cards));
    memset(opened, 0, sizeof(opened));
    memset(closed, 0, sizeof(closed));
    requests = bad_closes = fail_dos = fail_expansion = selected_cards = 0;
    board_count = card_count = prom_available = hide_on_open = 0;
    pci_available = expected_requests = 1;
    error_text[0] = 0;
    exec_fixture.base.AttnFlags = AFF_68040;
    libs[PROMETHEUS].base.lib_Revision = 5;
    vector(&exec_fixture.base, 552, open_library);
    vector(&exec_fixture.base, 414, close_library);
    vector(&exec_fixture.base, 276, find_name);
    vector(&libs[EXPANSION].base, 72, find_config);
    vector(&libs[INTUITION].base, 588, requester);
    vector(&libs[DOS].base, 906, get_var);
    vector(&libs[PCI].base, 204, find_card);
    vector(&libs[PCI].base, 210, card_attrs);
    vector(&libs[PROMETHEUS].base, 30, find_card);
    CacheClearU();
}

static void physical(void)
{
    board_count = 1;
    boards[0].cd_Rom.er_Manufacturer = 3643;
    boards[0].cd_Rom.er_Product = 200;
    boards[0].cd_BoardAddr = (APTR)HOST_OFFSET;
    boards[0].cd_BoardSize = 0x20000000;
}

static void published(void)
{
    physical();
    boards[0].cd_Rom.er_Manufacturer = 1731;
    boards[0].cd_Rom.er_Product = 0;
    boards[0].cd_Rom.er_SerialNumber = FIRESTORM_ID;
    boards[0].cd_BoardAddr = (APTR)HOST_REGISTERS;
    boards[0].cd_BoardSize = 4;
}

typedef APTR (*init_entry)(REG(APTR, base, d0), REG(BPTR, segment, a0),
                          REG(struct ExecBase *, sys, a6), REG(APTR, foreign_data, a4));
static void run(init_entry entry, BPTR segment, const char *expected)
{
    /* Poison the caller stack to expose cleanup of uninitialized bases. */
    volatile ULONG poison[256];
    unsigned i;
    for (i = 0; i < 256; ++i) poison[i] = 0xdeadbeef;
    CHECK(entry(NULL, segment, &exec_fixture.base, NULL) == NULL);
    CHECK(requests == expected_requests);
    CHECK(!strcmp(error_text, expected));
    CHECK(bad_closes == 0);
    for (i = 0; i < 5; ++i) CHECK(opened[i] == closed[i]);
    CHECK(poison[0] == 0xdeadbeef);
}

int main(int argc, char **argv)
{
    BPTR segment;
    UBYTE *code;
    ULONG bytes, i;
    init_entry entry = NULL;
    const char *no_bridge = "Could not find a supported PCI bridge";
    const char *no_ppc = "No supported PPC PCI bridge detected";
    const char *no_vga = "No supported VGA card detected";
    if (argc != 2 || !(segment = LoadSeg(argv[1]))) return 20;
    code = (UBYTE *)BADDR(segment) + 4;
    bytes = *((ULONG *)BADDR(segment) - 1) - 8;
    for (i = 0; i + sizeof(struct Resident) <= bytes; i += 2) {
        struct Resident *resident = (struct Resident *)(code + i);
        if (resident->rt_MatchWord == RTC_MATCHWORD &&
            resident->rt_MatchTag == resident) {
            entry = (init_entry)resident->rt_Init;
            break;
        }
    }
    if (!entry) { puts("No resident found"); return 20; }

    reset(); run(entry, segment, no_bridge);
    reset(); exec_fixture.base.AttnFlags = 0;
    run(entry, segment, "This library requires a 68LC040 or better");
    reset(); fail_dos = 1; run(entry, segment, "Could not open dos.library V37+");
    reset(); fail_expansion = 1;
    run(entry, segment, "Could not open expansion.library V37+");
    reset(); physical(); pci_available = 0;
    run(entry, segment, "Could not open openpci.library or prometheus.library V4+");
    reset(); physical(); run(entry, segment, no_ppc);
    reset(); published(); run(entry, segment, no_ppc);
    reset(); physical(); hide_on_open = 1; run(entry, segment, no_ppc);
    reset(); published(); boards[0].cd_Rom.er_SerialNumber = 0xad470001;
    run(entry, segment, no_bridge);
    reset(); published(); boards[0].cd_Rom.er_Product = 3;
    run(entry, segment, no_bridge);
    reset(); published(); boards[0].cd_BoardSize = 0;
    run(entry, segment, no_bridge);
    reset(); published(); boards[0].cd_BoardAddr = (APTR)0x1000;
    run(entry, segment, no_bridge);
    reset(); published(); card_count = 1;
    cards[0] = (struct card){0x1057, 4, HOST_OFFSET, 1};
    run(entry, segment, no_vga);
    reset(); published(); card_count = 1;
    cards[0] = (struct card){0x1057, 4, 0x60000000, 1};
    run(entry, segment, no_ppc);
    reset(); published(); card_count = 1;
    cards[0] = (struct card){0x1057, 4, HOST_OFFSET, 0};
    run(entry, segment, no_ppc);
    reset(); published(); card_count = 2;
    cards[0] = (struct card){0x1057, 4, 0x60000000, 1};
    cards[1] = (struct card){0x1057, 4, HOST_OFFSET, 1};
    run(entry, segment, no_vga); CHECK(selected_cards == 2);
    reset(); physical(); pci_available = 0; prom_available = 1;
    run(entry, segment, no_ppc);
    reset(); physical(); pci_available = 0; prom_available = 1;
    libs[PROMETHEUS].base.lib_Revision = 4;
    run(entry, segment, "Could not open prometheus.library V4.5+");

    /* A graphics card on another host must not be paired with this PPC. */
    reset(); published(); card_count = 2;
    cards[0] = (struct card){0x1057, 4, HOST_OFFSET, 1};
    cards[1] = (struct card){0x121a, 9, 0x60000000, 1};
    run(entry, segment, no_vga);
    /* A missing Harrier BAR reservation reports one error and closes
     * each library once; LibInit must not report the failure again. */
    reset(); published(); card_count = 2;
    cards[0] = (struct card){0x1057, 0x480b, HOST_OFFSET, 1};
    cards[1] = (struct card){0x121a, 9, HOST_OFFSET, 1};
    run(entry, segment, "Could not allocate sufficient PCI memory");

    UnLoadSeg(segment);
    printf("Bridge init: %u checks, %u failures\n", checks, failures);
    return failures ? 20 : 0;
}
