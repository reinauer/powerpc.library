#ifndef PROTO_MEDIATORPCI_H
#define PROTO_MEDIATORPCI_H

//vbcc inline calls for pci.library, from pci_lib.fd and pci.i in the 17.13b
//sources. pci.i gives PCIFreeMem as -154; vectors are 6 bytes apart, so -156 is
//assumed here.

#include <libraries/mediatorpci.h>

struct PciDevice* __FindPciDevice(__reg("a6") struct PciBase* base, __reg("d0") ULONG vendor, __reg("d1") ULONG device, __reg("d2") ULONG number) = "\tjsr\t-30(a6)\n";
struct PciDevice* __FindPciDevFunc(__reg("a6") struct PciBase* base, __reg("d0") ULONG number) = "\tjsr\t-42(a6)\n";
VOID  __SetInterrupt(__reg("a6") struct PciBase* base, __reg("a0") struct PciDevice* device) = "\tjsr\t-48(a6)\n";
VOID  __AddInterrupt(__reg("a6") struct PciBase* base, __reg("a0") struct PciDevice* device, __reg("a1") struct Interrupt* server) = "\tjsr\t-78(a6)\n";
UBYTE __ReadConfigurationByte(__reg("a6") struct PciBase* base, __reg("d0") ULONG devfn, __reg("d1") ULONG offset) = "\tjsr\t-90(a6)\n";
UWORD __ReadConfigurationWord(__reg("a6") struct PciBase* base, __reg("d0") ULONG devfn, __reg("d1") ULONG offset) = "\tjsr\t-96(a6)\n";
ULONG __ReadConfigurationLong(__reg("a6") struct PciBase* base, __reg("d0") ULONG devfn, __reg("d1") ULONG offset) = "\tjsr\t-102(a6)\n";
VOID  __WriteConfigurationByte(__reg("a6") struct PciBase* base, __reg("d0") ULONG devfn, __reg("d1") ULONG offset, __reg("d2") ULONG value) = "\tjsr\t-108(a6)\n";
VOID  __WriteConfigurationWord(__reg("a6") struct PciBase* base, __reg("d0") ULONG devfn, __reg("d1") ULONG offset, __reg("d2") ULONG value) = "\tjsr\t-114(a6)\n";
VOID  __WriteConfigurationLong(__reg("a6") struct PciBase* base, __reg("d0") ULONG devfn, __reg("d1") ULONG offset, __reg("d2") ULONG value) = "\tjsr\t-120(a6)\n";
ULONG __AllocPCIBAR(__reg("a6") struct PciBase* base, __reg("d0") ULONG size, __reg("d1") ULONG bar, __reg("a0") struct PciDevice* device) = "\tjsr\t-150(a6)\n";
VOID  __FreePCIBAR(__reg("a6") struct PciBase* base, __reg("d0") ULONG bar, __reg("a0") struct PciDevice* device) = "\tjsr\t-156(a6)\n";

#define FindPciDevice(vendor, device, number)       __FindPciDevice(MediatorPCIBase, (vendor), (device), (number))
#define FindPciDevFunc(number)                      __FindPciDevFunc(MediatorPCIBase, (number))
#define SetInterrupt(device)                        __SetInterrupt(MediatorPCIBase, (device))
#define AddInterrupt(device, server)                __AddInterrupt(MediatorPCIBase, (device), (server))
#define ReadConfigurationByte(devfn, offset)        __ReadConfigurationByte(MediatorPCIBase, (devfn), (offset))
#define ReadConfigurationWord(devfn, offset)        __ReadConfigurationWord(MediatorPCIBase, (devfn), (offset))
#define ReadConfigurationLong(devfn, offset)        __ReadConfigurationLong(MediatorPCIBase, (devfn), (offset))
#define WriteConfigurationByte(devfn, offset, value) __WriteConfigurationByte(MediatorPCIBase, (devfn), (offset), (value))
#define WriteConfigurationWord(devfn, offset, value) __WriteConfigurationWord(MediatorPCIBase, (devfn), (offset), (value))
#define WriteConfigurationLong(devfn, offset, value) __WriteConfigurationLong(MediatorPCIBase, (devfn), (offset), (value))
#define AllocPCIBAR(size, bar, device)              __AllocPCIBAR(MediatorPCIBase, (size), (bar), (device))
#define FreePCIBAR(bar, device)                     __FreePCIBAR(MediatorPCIBase, (bar), (device))

#endif
