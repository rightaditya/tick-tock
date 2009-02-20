//*--------------------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*--------------------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*--------------------------------------------------------------------------------------
//* File Name           : ebi.h
//* Object              : External Bus Interface Definition File
//*
//* 1.0 01/04/00 JCZ    : Creation
//* 1.1 10/01/02 PFi    : Conditionnal compilation added in StructEBI for
//*                     : AT91M42800's extra Abort Status registers and definition
//*--------------------------------------------------------------------------------------

#ifndef ebi_h
#define ebi_h

#include    "periph/stdc/std_c.h"

/*-------------------------------------------------------*/
/* Memory Controller User Interface Structure Definition */
/*-------------------------------------------------------*/

typedef struct
{
    at91_reg        EBI_CSR[8] ;        /* Chip Select Register */
    at91_reg        EBI_RCR ;           /* Remap Control Register */
    at91_reg        EBI_MCR ;           /* Memory Control Register */
#ifdef AT91M42800
    at91_reg        Reserved0 ;         /* Reserved */
    at91_reg        Reserved1 ;         /* Reserved */
    at91_reg        EBI_ASR ;           /* Abort Status Register */
    at91_reg        EBI_AASR ;          /* Address Abort Status Register */
#endif
} StructEBI ;

/*--------------------------------*/
/* EBI_CSR: Chip Select Registers */
/*--------------------------------*/

#define EBI_DBW             0x3         /* Data Bus Width */
#define EBI_DBW_16          0x1         /* 16-bit Data Bus */
#define EBI_DBW_8           0x2         /* 8-bit Data Bus */

#define EBI_NWS             0x1C        /* Number of Wait States */
#define EBI_NWS_1           0x0         /* 1 wait state */
#define EBI_NWS_2           0x4         /* 2 wait state */
#define EBI_NWS_3           0x8         /* 3 wait state */
#define EBI_NWS_4           0xC         /* 4 wait state */
#define EBI_NWS_5           0x10        /* 5 wait state */
#define EBI_NWS_6           0x14        /* 6 wait state */
#define EBI_NWS_7           0x18        /* 7 wait state */
#define EBI_NWS_8           0x1C        /* 8 wait state */

#define EBI_WSE             0x20        /* Wait State Enable */

#define EBI_PAGES           0x180       /* Page size */
#define EBI_PAGES_1M        0x0         /* 1 Mbytes */
#define EBI_PAGES_4M        0x80        /* 4 Mbytes */
#define EBI_PAGES_16M       0x100       /* 16 Mbytes */
#define EBI_PAGES_64M       0x180       /* 64 Mbytes */

#define EBI_TDF             0xE00       /* Number of Data Float Output Time Clock Cycle */
#define EBI_TDF_0           0x0         /* 0 cycles added */
#define EBI_TDF_1           0x200       /* 1 cycles added */
#define EBI_TDF_2           0x400       /* 2 cycles added */
#define EBI_TDF_3           0x600       /* 3 cycles added */
#define EBI_TDF_4           0x800       /* 4 cycles added */
#define EBI_TDF_5           0xA00       /* 5 cycles added */
#define EBI_TDF_6           0xC00       /* 6 cycles added */
#define EBI_TDF_7           0xE00       /* 7 cycles added */

#define EBI_BAT             0x1000      /* Byte Access Type */
#define EBI_BAT_BYTE_WRITE  0x0         /* Byte Write Access Type */
#define EBI_BAT_BYTE_SELECT 0x1000      /* Byte Select Access Type */

#define EBI_CSEN            0x2000      /* Chip Select Enable */

#define EBI_BA              0xFFF00000

/*---------------------------------*/
/* EBI_RCR: Remap Control Register */
/*---------------------------------*/

#define EBI_RCB             0x1         /* Remap Command Bit */

/*----------------------------------*/
/* EBI_MCR: Memory Control Register */
/*----------------------------------*/

#define EBI_ALE             0x7         /* Address Line Enable */
#define EBI_ALE_16M         0x0         /* Bank Size 16Mbytes */
#define EBI_ALE_8M          0x4         /* Bank Size 16Mbytes */
#define EBI_ALE_4M          0x5         /* Bank Size 16Mbytes */
#define EBI_ALE_2M          0x6         /* Bank Size 16Mbytes */
#define EBI_ALE_1M          0x7         /* Bank Size 16Mbytes */

#define EBI_DRP             0x10        /* Data Read Protocol */
#define EBI_DRP_STANDARD    0x0         /* Standard Read Protocol */
#define EBI_DRP_EARLY       0x10        /* Early Read Protocol */

/*-------------------------------------------------------------------*/
/* EBI_MCR: Abort Status Register - Defined only for the AT91M42800  */
/*-------------------------------------------------------------------*/
#ifdef AT91M42800

#define EBI_UNDADD              0x1          /* Undefined Address Abort Status */
#define EBI_MISADD              0x2          /* Misaligned Address Abort Status */

#define EBI_ABTSZ_IN_BYTE       (0x0<<8)     /* Abort size in Byte */
#define EBI_ABTSZ_IN_HALF_WORD  0x100        /* Abort size in Half-word */
#define EBI_ABTSZ_IN_WORD       0x200        /* Abort size in Word */
#define EBI_ABTSZ_RESERVED      0x300        /* Abort size in Reserved */

#define EBI_ABTTYP_READ         (0x0<<10)    /* Abort Type = Data Read */
#define EBI_ABTTYP_WRITE        0x400        /* Abort Type = Data Write */
#define EBI_ABTTYP_CODE_FETCH   0x800        /* Abort Type = Code Fetch */
#define EBI_ABTTYP_RESERVED     0xC00        /* Abort Type = Reserved */

#define EBI_ARM                 0x1000       /* Abort Induced by the ARM Core */
#define EBI_PDC                 0x2000       /* Abort Induced by the Peripheral Data Controller */

#endif //AT91M42800

/*------------------*/
/* EBI Base Address */
/*------------------*/
#define EBI_BASE            ((StructEBI *)0xFFE00000)

#endif /* ebi_h */
