#ifndef LIBRARIES_MEDIATORPCI_H
#define LIBRARIES_MEDIATORPCI_H

//Rebuilt from pci.i in the 17.13b sources. Only the fields used here are named.

#include <exec/libraries.h>
#include <exec/nodes.h>

#pragma pack(push,2)

struct PciBase {
    struct Library pb_LibNode;
    UBYTE          pb_Private[0x2a - sizeof(struct Library)];
    ULONG          pb_MemAddress;
};

struct PciDevice {
    struct Node pd_Node;
    UWORD       pd_VendorID;
    UWORD       pd_DeviceID;
    ULONG       pd_ClassCode;
    UBYTE       pd_RevisionID;
    UBYTE       pd_InterruptPin;
    UWORD       pd_SubVendorID;
    UWORD       pd_SubID;
    ULONG       pd_ABaseAddress0;
    ULONG       pd_Size0;
    ULONG       pd_ABaseAddress1;
    ULONG       pd_Size1;
    ULONG       pd_ABaseAddress2;
    ULONG       pd_Size2;
};

#pragma pack(pop)

#endif
