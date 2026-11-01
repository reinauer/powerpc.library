#ifndef PROTO_PROMETHEUS_H
#define PROTO_PROMETHEUS_H

//vbcc inline calls for prometheus.library. The registers of the two address
//space calls come from the compiled 17.15 library.

#include <exec/types.h>
#include <utility/tagitem.h>

typedef struct PCIBoard PCIBoard;

#define PRM_Vendor          0x6EDA0000
#define PRM_Device          0x6EDA0001
#define PRM_Revision        0x6EDA0002
#define PRM_BoardOwner      0x6EDA0005
#define PRM_MemoryAddr0     0x6EDA0010
#define PRM_MemoryAddr1     0x6EDA0011
#define PRM_MemoryAddr2     0x6EDA0012
#define PRM_MemoryAddr3     0x6EDA0013
#define PRM_MemorySize0     0x6EDA0020
#define PRM_MemorySize1     0x6EDA0021

PCIBoard* __Prm_FindBoardTagList(__reg("a6") struct Library* base, __reg("a0") PCIBoard* previous, __reg("a1") struct TagItem* tags) = "\tjsr\t-30(a6)\n";
ULONG __Prm_GetBoardAttrsTagList(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, __reg("a1") struct TagItem* tags) = "\tjsr\t-36(a6)\n";
ULONG __Prm_ReadConfigLong(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, __reg("d0") ULONG offset) = "\tjsr\t-42(a6)\n";
UWORD __Prm_ReadConfigWord(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, __reg("d0") ULONG offset) = "\tjsr\t-48(a6)\n";
UBYTE __Prm_ReadConfigByte(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, __reg("d0") ULONG offset) = "\tjsr\t-54(a6)\n";
VOID  __Prm_WriteConfigLong(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, __reg("d0") ULONG value, __reg("d1") ULONG offset) = "\tjsr\t-60(a6)\n";
VOID  __Prm_WriteConfigWord(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, __reg("d0") ULONG value, __reg("d1") ULONG offset) = "\tjsr\t-66(a6)\n";
VOID  __Prm_WriteConfigByte(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, __reg("d0") ULONG value, __reg("d1") ULONG offset) = "\tjsr\t-72(a6)\n";
ULONG __Prm_SetBoardAttrsTagList(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, __reg("a1") struct TagItem* tags) = "\tjsr\t-78(a6)\n";
ULONG __Prm_AddIntServer(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, __reg("a1") struct Interrupt* server) = "\tjsr\t-84(a6)\n";
APTR  __Prm_AllocDMABuffer(__reg("a6") struct Library* base, __reg("d0") ULONG size) = "\tjsr\t-96(a6)\n";
APTR  __Prm_AllocPCIAddressSpace(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, __reg("d0") ULONG size, __reg("d1") ULONG bar) = "\tjsr\t-120(a6)\n";
VOID  __Prm_FreePCIAddressSpace(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, __reg("d0") ULONG bar) = "\tjsr\t-126(a6)\n";

PCIBoard* __Prm_FindBoardTags(__reg("a6") struct Library* base, __reg("a0") PCIBoard* previous, ULONG tag, ...) = "\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-30(a6)\n\tmovea.l\t(a7)+,a1\n";
ULONG __Prm_GetBoardAttrsTags(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, ULONG tag, ...) = "\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-36(a6)\n\tmovea.l\t(a7)+,a1\n";
ULONG __Prm_SetBoardAttrsTags(__reg("a6") struct Library* base, __reg("a0") PCIBoard* board, ULONG tag, ...) = "\tmove.l\ta1,-(a7)\n\tlea\t4(a7),a1\n\tjsr\t-78(a6)\n\tmovea.l\t(a7)+,a1\n";

#define Prm_FindBoardTags(previous, ...)            __Prm_FindBoardTags(PrometheusBase, (previous), __VA_ARGS__)
#define Prm_GetBoardAttrsTags(board, ...)           __Prm_GetBoardAttrsTags(PrometheusBase, (board), __VA_ARGS__)
#define Prm_SetBoardAttrsTags(board, ...)           __Prm_SetBoardAttrsTags(PrometheusBase, (board), __VA_ARGS__)
#define Prm_FindBoardTagList(previous, tags)        __Prm_FindBoardTagList(PrometheusBase, (previous), (tags))
#define Prm_GetBoardAttrsTagList(board, tags)       __Prm_GetBoardAttrsTagList(PrometheusBase, (board), (tags))
#define Prm_ReadConfigLong(board, offset)           __Prm_ReadConfigLong(PrometheusBase, (board), (offset))
#define Prm_ReadConfigWord(board, offset)           __Prm_ReadConfigWord(PrometheusBase, (board), (offset))
#define Prm_ReadConfigByte(board, offset)           __Prm_ReadConfigByte(PrometheusBase, (board), (offset))
#define Prm_WriteConfigLong(board, value, offset)   __Prm_WriteConfigLong(PrometheusBase, (board), (value), (offset))
#define Prm_WriteConfigWord(board, value, offset)   __Prm_WriteConfigWord(PrometheusBase, (board), (value), (offset))
#define Prm_WriteConfigByte(board, value, offset)   __Prm_WriteConfigByte(PrometheusBase, (board), (value), (offset))
#define Prm_AddIntServer(board, server)             __Prm_AddIntServer(PrometheusBase, (board), (server))
#define Prm_AllocDMABuffer(size)                    __Prm_AllocDMABuffer(PrometheusBase, (size))
#define Prm_AllocPCIAddressSpace(board, size, bar)  __Prm_AllocPCIAddressSpace(PrometheusBase, (board), (size), (bar))
#define Prm_FreePCIAddressSpace(board, bar)         __Prm_FreePCIAddressSpace(PrometheusBase, (board), (bar))

#endif
