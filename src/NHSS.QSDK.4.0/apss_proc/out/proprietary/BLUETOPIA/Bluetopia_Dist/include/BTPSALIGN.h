/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#ifndef __BTPSPATCH__
#define __BTPSPATCH__

#include <stddef.h>
#include "BKRNLAPI.h"           /* BTPS Kernel Prototypes/Constants.          */

   /* Utility macros to get rid of VS warning about undefined struct.   */
#define BTPS_COMBINE_TOKENS(x, y)                  x ## y
#define BTPS_COMBINE_TOKENS2(x, y)                 BTPS_COMBINE_TOKENS(x, y)
#define BTPS_CREATE_UNIQUE_TYPE(_x)                BTPS_COMBINE_TOKENS2(_x, __LINE__)

   /* The following MACRO is a MACRO that determines the alignment      */
   /* offset for the specified type, based on an input pointer value.   */
   /* This MACRO returns the offset that is required to be added to the */
   /* original pointer (to make the new type alignment safe), it does   */
   /* not return an actual pointer.  For instance, this MACRO will      */
   /* return the following (for the given inputs):                      */
   /*          Alignment      _Ptr     _Type    Result                  */
   /*          ---------------------------------------                  */
   /*              1      0x10000000   (int)      0                     */
   /*              1      0x10000001   (int)      0                     */
   /*              1      0x10000002   (int)      0                     */
   /*              1      0x10000003   (int)      0                     */
   /*              1      0x10000004   (int)      0                     */
   /*              2      0x10000000   (int)      0                     */
   /*              2      0x10000001   (int)      1                     */
   /*              2      0x10000002   (int)      0                     */
   /*              2      0x10000003   (int)      1                     */
   /*              2      0x10000004   (int)      0                     */
   /*              4      0x10000000   (int)      0                     */
   /*              4      0x10000001   (int)      3                     */
   /*              4      0x10000002   (int)      2                     */
   /*              4      0x10000003   (int)      1                     */
   /*              4      0x10000004   (int)      0                     */
   /* * NOTE * This MACRO is safe for any size pointer (e.g. 32 and     */
   /*          64 bit pointers can be passed to this function).         */
#define BTPS_ALIGNMENT_OFFSET(_Ptr, _Type)         (((BTPS_STRUCTURE_OFFSET(BTPS_CREATE_UNIQUE_TYPE(struct _Type) {Byte_t _C; _Type _Member;}, _Member) - (size_t)(_Ptr)) % BTPS_STRUCTURE_OFFSET(BTPS_CREATE_UNIQUE_TYPE(struct _Type), _Member)))

   /* The following defines the alignment size for the system in bytes. */
   /* It exists solely for allocating headroom for suspending the stack,*/
   /* to allow for proper alignment.                                    */
#define BTPS_ALIGNMENT_SIZE                        (sizeof(size_t))

#endif
