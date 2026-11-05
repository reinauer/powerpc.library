#ifndef OPENPCI_INLINE_H
#define OPENPCI_INLINE_H

//vbcc inline calls for the part of openpci.library used here.

struct pci_dev;

/* OpenPCI extends the original Prometheus tag set. */
#ifndef PRM_PCIToHostOffset
#define PRM_PCIToHostOffset 0x6eda000fUL
#endif

UBYTE __pci_read_config_byte(__reg("a6") struct Library* base, __reg("d0") ULONG offset, __reg("a0") struct pci_dev* device) = "\tjsr\t-108(a6)\n";
UWORD __pci_read_config_word(__reg("a6") struct Library* base, __reg("d0") ULONG offset, __reg("a0") struct pci_dev* device) = "\tjsr\t-114(a6)\n";
ULONG __pci_read_config_long(__reg("a6") struct Library* base, __reg("d0") ULONG offset, __reg("a0") struct pci_dev* device) = "\tjsr\t-120(a6)\n";
VOID  __pci_write_config_byte(__reg("a6") struct Library* base, __reg("d0") ULONG offset, __reg("d1") ULONG value, __reg("a0") struct pci_dev* device) = "\tjsr\t-126(a6)\n";
VOID  __pci_write_config_word(__reg("a6") struct Library* base, __reg("d0") ULONG offset, __reg("d1") ULONG value, __reg("a0") struct pci_dev* device) = "\tjsr\t-132(a6)\n";
VOID  __pci_write_config_long(__reg("a6") struct Library* base, __reg("d0") ULONG offset, __reg("d1") ULONG value, __reg("a0") struct pci_dev* device) = "\tjsr\t-138(a6)\n";
ULONG __pci_add_intserver(__reg("a6") struct Library* base, __reg("a0") struct Interrupt* server, __reg("a1") struct pci_dev* device) = "\tjsr\t-150(a6)\n";
struct pci_dev* __FindBoardA(__reg("a6") struct Library* base, __reg("a0") struct pci_dev* previous, __reg("a1") struct TagItem* tags) = "\tjsr\t-204(a6)\n";
ULONG __GetBoardAttrsA(__reg("a6") struct Library* base, __reg("a0") struct pci_dev* device, __reg("a1") struct TagItem* tags) = "\tjsr\t-210(a6)\n";
ULONG __SetBoardAttrsA(__reg("a6") struct Library* base, __reg("a0") struct pci_dev* device, __reg("a1") struct TagItem* tags) = "\tjsr\t-216(a6)\n";

#define pci_read_config_byte(offset, device)         __pci_read_config_byte(OpenPCIBase, (offset), (device))
#define pci_read_config_word(offset, device)         __pci_read_config_word(OpenPCIBase, (offset), (device))
#define pci_read_config_long(offset, device)         __pci_read_config_long(OpenPCIBase, (offset), (device))
#define pci_write_config_byte(offset, value, device) __pci_write_config_byte(OpenPCIBase, (offset), (value), (device))
#define pci_write_config_word(offset, value, device) __pci_write_config_word(OpenPCIBase, (offset), (value), (device))
#define pci_write_config_long(offset, value, device) __pci_write_config_long(OpenPCIBase, (offset), (value), (device))
#define pci_add_intserver(server, device)            __pci_add_intserver(OpenPCIBase, (server), (device))
#define FindBoardA(previous, tags)                   __FindBoardA(OpenPCIBase, (previous), (tags))
#define GetBoardAttrsA(device, tags)                 __GetBoardAttrsA(OpenPCIBase, (device), (tags))
#define SetBoardAttrsA(device, tags)                 __SetBoardAttrsA(OpenPCIBase, (device), (tags))

#endif
