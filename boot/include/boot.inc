; ------------- loader and kernel ------------
LOADER_BASE_ADDR    equ 0x900
LOADER_START_SECTOR equ 0x2

; ------------- attributes of GDT ------------
DESC_G_4K           equ 10000000_00000000_00000000b

DESC_D_32           equ 1000000_00000000_00000000b

DESC_L              equ 000000_00000000_00000000b

DESC_AVL            equ 00000_00000000_00000000b

DESC_LIMIT_CODE2    equ 1111_00000000_00000000b
DESC_LIMIT_DATA2    equ DESC_LIMIT_CODE2
DESC_LIMIT_VIDEO2   equ 0000_00000000_00000000b

DESC_P              equ 10000000_00000000b

DESC_DPL_0          equ 0000000_00000000b
DESC_DPL_1          equ 0100000_00000000b
DESC_DPL_2          equ 1000000_00000000b
DESC_DPL_3          equ 1100000_00000000b

DESC_S_CODE         equ 10000_00000000b
DESC_S_DATA         equ DESC_S_CODE
DESC_S_SYS          equ 00000_00000000b

DESC_TYPE_CODE      equ 1000_00000000b
DESC_TYPE_DATA      equ 0010_00000000b

DESC_CODE_HIGH4     equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
                        DESC_L + DESC_AVL + DESC_LIMIT_CODE2 + \
                        DESC_P + DESC_DPL_0 + DESC_S_CODE + \
                        DESC_TYPE_CODE + 0x00

DESC_DATA_HIGH4     equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
                        DESC_L + DESC_AVL + DESC_LIMIT_DATA2 + \
                        DESC_P + DESC_DPL_0 + DESC_S_DATA + \
                        DESC_TYPE_DATA + 0x00

DESC_VIDEO_HIGH4    equ (0x00 << 24) + DESC_G_4K + DESC_D_32 + \
                        DESC_L + DESC_AVL + DESC_LIMIT_VIDEO2 + \
                        DESC_P + DESC_DPL_0 + DESC_S_DATA + \
                        DESC_TYPE_DATA + 0x0b

; ------------- attributes of selector -----------
RPL0    equ 00b
RPL1    equ 01b
RPL2    equ 10b
RPL3    equ 11b

TI_GDT  equ 000b
TI_LDT  equ 100b

; ------------- attributes of paging -------------
PAGE_DIR_TABLE_ADDR equ 0x100000

PG_P                equ 1b
PG_RW_R             equ 00b
PG_RW_W             equ 10b
PG_US_S             equ 000b
PG_US_U             equ 100b
