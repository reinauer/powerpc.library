/* PPCInit: reserve PCI memory for a PowerPC card from PCI-Configuration.
 *
 * pci.library and prometheus.library give drivers a call that claims PCI
 * address space at run time. OpenPCI assigns all of it during library
 * initialization, so a driver arriving later has nothing to claim from.
 * This tool runs before the library sizes the BARs and presets them through
 * SetBoardAttrs(); the library then places each reservation like any other
 * BAR. Its address appears in the pci_dev and through PRM_MemoryAddr<n>.
 *
 * The slots and sizes match the BARs powerpc.library sets up, which it reads
 * back with PRM_MemoryAddr<n> and PRM_MemorySize<n>. A preset overrides an
 * Ignore for the same BAR, so this line must follow any Ignore line for the
 * card.
 *
 * A device whose BARs cannot all be placed is dropped, so a card whose
 * reservations exceed the host's memory window is left as it was and the
 * tool reports a warning.
 *
 * Install as LIBS:PCI/PPCInit and add a line "PPCInit" to the configuration
 * file. RAM=<MiB> sets the RAM window, a power of two; the default is 256. */
/* LoadSeg()ed, not run: the library expects moveq #-1,d0; rts first,
 * ahead of everything the headers define. */
asm(".text\n"
    "\tmoveq #-1,d0\n"
    "\trts\n");

#include <exec/types.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <utility/tagitem.h>
#include <libraries/openpci.h>
#include <libraries/pcitags.h>
#define __NOLIBBASE__
#include <proto/dos.h>
#include <proto/openpci.h>

#define RAM_WINDOW       0
#define DEFAULT_RAM_MIB  256
#define MAXIMUM_RAM_MIB  1024
#define WARNING          10
#define FAILURE          20

struct reservation {
    UWORD vendor, device;
    ULONG bar;
    ULONG size;
};

/* Table order groups a card's slots; a card with no match costs nothing. */
static const struct reservation reservations[] = {
    {0x1057, 0x0004, 0, RAM_WINDOW},    /* MPC107 (Sonnet): LMBAR */
    {0x1057, 0x480b, 0, 0x1000},        /* Harrier: PMEP */
    {0x1057, 0x480b, 1, 0x1000},        /* Harrier: XCSR */
    {0x1057, 0x480b, 2, RAM_WINDOW},    /* Harrier: PowerPC RAM */
    {0x1057, 0x480b, 3, 0x40000},       /* Harrier: MPIC */
    {0x1957, 0x0086, 1, 0x4000000}      /* MPC8343E (Killer NIC): RAM */
};
#define RESERVATION_COUNT (sizeof(reservations) / sizeof(reservations[0]))

static LONG ppcinit_init(struct RDArgs *arguments __asm("a0"),
                         struct DosLibrary *dos __asm("a2"),
                         struct Library *base __asm("a6"));

static const struct Resident romtag = {
    RTC_MATCHWORD, (struct Resident *)&romtag, (APTR)(&romtag + 1),
    0, 1, 0, 0,
    "PPCInit", "PPCInit 1.0 (31.10.2026)\r\n",
    (APTR)ppcinit_init
};

static ULONG reservation_size(const struct reservation *entry, ULONG ram)
{
    return entry->size == RAM_WINDOW ? ram : entry->size;
}

static BOOL same_card(const struct reservation *entry,
                      const struct reservation *card)
{
    return entry < reservations + RESERVATION_COUNT &&
           entry->vendor == card->vendor && entry->device == card->device;
}

static BOOL card_fits(struct pci_dev *board, const struct reservation *card,
                      ULONG ram, struct Library *OpenPciBase)
{
    const struct reservation *entry;
    struct TagItem window[3];
    ULONG low = 0, high = 0, total = 0;
    window[0].ti_Tag = PRM_PCIMemWindowLow;
    window[0].ti_Data = (ULONG)&low;
    window[1].ti_Tag = PRM_PCIMemWindowHigh;
    window[1].ti_Data = (ULONG)&high;
    window[2].ti_Tag = TAG_DONE;
    if (GetBoardAttrsA(board, window) != 2 || high <= low) return FALSE;
    for (entry = card; same_card(entry, card); ++entry)
        total += reservation_size(entry, ram);
    return total <= high - low;
}

static BOOL reserve_card(struct pci_dev *board, const struct reservation *card,
                         ULONG ram, struct Library *OpenPciBase)
{
    const struct reservation *entry;
    for (entry = card; same_card(entry, card); ++entry) {
        struct TagItem preset[3];
        preset[0].ti_Tag = PRM_MemorySize0 + entry->bar;
        preset[0].ti_Data = reservation_size(entry, ram);
        /* A memory reservation, whatever the unused BAR would have decoded. */
        preset[1].ti_Tag = PRM_MemoryFlags0 + entry->bar;
        preset[1].ti_Data = 0;
        preset[2].ti_Tag = TAG_DONE;
        if (SetBoardAttrsA(board, preset) != 2) return FALSE;
    }
    return TRUE;
}

static LONG ppcinit_init(struct RDArgs *arguments __asm("a0"),
                         struct DosLibrary *dos __asm("a2"),
                         struct Library *base __asm("a6"))
{
    struct Library *OpenPciBase = base;
    struct DosLibrary *DOSBase = dos;
    const struct reservation *entry = reservations;
    LONG *options[1];
    ULONG mib = DEFAULT_RAM_MIB;
    LONG result = 0;

    /* Filled in by hand: there is no C library for memset(). */
    options[0] = NULL;
    if (!ReadArgs((STRPTR)"RAM/N", (LONG *)options, arguments)) return FAILURE;
    if (options[0]) mib = *options[0];
    if (!mib || mib > MAXIMUM_RAM_MIB || (mib & (mib - 1))) return FAILURE;

    while (entry < reservations + RESERVATION_COUNT) {
        const struct reservation *card = entry;
        struct pci_dev *board = pci_find_device(card->vendor, card->device, NULL);
        if (board && (!card_fits(board, card, mib << 20, OpenPciBase) ||
                      !reserve_card(board, card, mib << 20, OpenPciBase)))
            result = WARNING;
        while (same_card(entry, card)) ++entry;
    }
    return result;
}
