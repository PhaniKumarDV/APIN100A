;/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
;
;                      S T A R T U P   C O D E
;
; GENERAL DESCRIPTION
;   This module contains the entry point for our Flash Programming Plug-In 
;   for Lauterbach JTAG/ICD TRACE32. All it does is setups stack and calls
;   our main 'C' function "main_c".
;
;   The C function does return and expects a breakpoint there. This is the
;   case because the FLASH programming plug-in specification requires it.
;   The break-point is used to give control back to TRACE32 Debugger so 
;   that it may fill up the buffer and call our plug-in again. This cycle
;   continues until FLASH programming is complete.
;
; EXTERNALIZED FUNCTIONS
;   None.  
;
; INITIALIZATION AND SEQUENCING REQUIREMENTS
;
;   This module must be linked first, specifically the AREA "StartHere", 
;   since this is our entry point. This is done in our scatter load file.
;
;  Copyright (c) 1998-2011 Qualcomm Technologies, Incorporated.  All Rights Reserved.
;*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*======================================================================

                           EDIT HISTORY FOR FILE
  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.

$Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_start.s#1 $

when         who     what, where, why
----------   ---     -----------------------------------------------------
2012-02-09   ah      Simplified (removed redundant blatox macro), never using DDR
2011-06-29   ab      Do init_zi only if RPM startup is not being used
2010-07-15   rh      Remove some unused code, add non-entry point option
2010-01-12   vj      Added Support for 7x27 target
2008-06-15   VTW     Ported from JFLASH.
===========================================================================*/

#include "msmhwioreg.h"

;/*============================================================================
;
;                  REGISTER DEFINITIONS
;
;============================================================================*/

; CPSR Control Masks 
PSR_Fiq_Mask    EQU     0x40
PSR_Irq_Mask    EQU     0x80

; Processor mode definitions 
PSR_Supervisor  EQU     0x13


;/*============================================================================
;
;                  STACK DEFINITIONS
;
;============================================================================*/


; Stack sizes
;If this value has to be changed, please also change in emmcbld_main.c
SVC_Stack_Size  EQU     0x4000
    

;/*============================================================================
;
;                  STARTUP CODE
;
;============================================================================*/


        IMPORT  main_c
        ;IMPORT  svc_stack
        IMPORT  memory_init
        IMPORT  zero_init_needed
        IMPORT  emmcbld_bsp_hw_init
        IMPORT  |Image$$ZI_DATA$$ZI$$Base|
        IMPORT  |Image$$ZI_DATA$$ZI$$Limit|

        EXPORT  __main
        EXPORT  __emmcbld_entry

        PRESERVE8
        AREA    EMMCBLD_ENTRY_POINT, CODE, READONLY
        CODE32


;=======================================================================
; MACRO mmu_set_default_cfg
; ARGS
;   NONE
; DESCRIPTION
;   Sets a default value into the mmu control register
;   we do not really need to do this, but we do it to
;   know exactly the state of the processor
;=======================================================================
   MACRO
   mmu_set_default_cfg
   ldr    r0, = 0x00050078
   MCR    p15, 0, r0, c1, c0, 0
   MEND
;=======================================================================


;=======================================================================
; MACRO mmu_enable_i_cache
; ARGS
;  NONE
; DESCRIPTION
;  Enables the I Cache
;  Does a read modify write, so no other bits in the control register a
;  affected
;=======================================================================
   MACRO
   mmu_enable_i_cache
   mrc     p15, 0 , r0, c1, c0, 0
   orr     r0, r0, # (1 << 12 )
   mcr     p15, 0 , r0, c1, c0, 0
   MEND
;=======================================================================

;=======================================================================
; MACRO init_zi
; ARGS
;  NONE
; DESCRIPTION
;  Initializes the zero-init region (.bss) to zero
;  r0  = start of ZI
;  r1  = end of ZI
;  r2  = 0 (for initializing memory) 
;  r3  = watchdog register
;  r4  = 1 (for kicking the dog)
;=======================================================================
  MACRO
    init_zi
    ldr     r0, =|Image$$ZI_DATA$$ZI$$Base|
    ldr     r1, =|Image$$ZI_DATA$$ZI$$Limit|
#ifdef FEATURE_EMMCBLD_HAVE_WDOG
    ldr     r3, =HWIO_WDOG_RESET_ADDR
#endif
    mov     r4, #0x1                   
    mov     r2,#0
init_zi_label
    cmp     r0,r1          ; unsigned compare, clear c on borrow
    strcc   r2,[r0],#4     ; str 0 in [r0] if r0 < r1
#ifdef FEATURE_EMMCBLD_HAVE_WDOG
    str     r4, [r3]       ; kick the dog
#endif
    bcc     init_zi_label  ; loop while ro < r1
  MEND
;=======================================================================


;=======================================================================
; MACRO reloc_data
; ARGS
;  NONE
; DESCRIPTION
;  Relocates the RW data for eHOSTDL
;  r0  = start of dest mem
;  r1  = end of dest mem (length of data to be relocated)
;  r2  = start of src mem
;  r3  = watchdog register
;  r4  = 1 (for kicking the dog)
;  r5  = tmp data holder for data relocation
;=======================================================================
  MACRO
    reloc_data
    ldr     r0, =|Image$$APP_RAM$$Base|
    ldr     r1, =|Image$$APP_RAM$$ZI$$Base|
#ifdef FEATURE_EMMCBLD_HAVE_WDOG
    ldr     r3, =HWIO_WDOG_RESET_ADDR
#endif
    mov     r4, #0x1                   
    ldr     r2, =|Load$$APP_RAM$$Base|
reloc_data_label
    cmp     r0,r1            ; unsigned compare, clear c on borrow
    ldrcc   r5,[r2],#4       ; Load [r2] to r5, increment [r2] if r0 < r1
    strcc   r5,[r0],#4       ; str r5 at [r0],  increment [r0] if r0 < r1
#ifdef FEATURE_EMMCBLD_HAVE_WDOG
    str     r4, [r3]         ; kick the dog
#endif
    bcc     reloc_data_label ; loop while ro < r1
  MEND
;=======================================================================


;/*============================================================================
;
; FUNCTION STARTUP
;
; DESCRIPTION
;   This function just setup the stack and calls "c" routine main_c.
;
; FORMAL PARAMETERS
;   None.
;
; DEPENDENCIES
;   Any initialization required for normal CPU operation must have
;   been performed prior to calling this module. This can be achieved
;   in the startup script files using data.set command to write to
;   appropriate MSM/ASB registers.
;
; RETURN VALUE
;   This function never returns.
;
; SIDE EFFECTS
;
;============================================================================*/
__main
__emmcbld_entry

#ifndef FEATURE_EMMCBLD_USE_RPM_STARTUP
        ENTRY
#endif
; Supervisor Mode
; Set up the Supervisor stack pointer.
        msr     CPSR_c, #PSR_Supervisor:OR:PSR_Irq_Mask:OR:PSR_Fiq_Mask
        ldr     r13, =EMMCBLD_IMEM_STACK_BASE

#ifndef FEATURE_EMMCBLD_USE_RPM_STARTUP
        init_zi
#endif

#ifdef FEATURE_EMMCBLD_USE_DDR
        reloc_data
#endif  

; ======================================================================
; Enable the instruction cache
; ======================================================================
   ;mmu_set_default_cfg
   ;mmu_enable_i_cache

; ======================================================================
; Initialize memory for C only once
;   The test/set of the global variable must be done here in assembly
;   because if we access a global variable in the C function, the
;   compiler will construct a PUSH/POP of registers and since we will
;   have just zeroed the stack, we will pop zero into R14 and then
;   branch to zero.  With no use of globals in the C function,
;   the compiler will generate a bx r14 for the return and all will
;   work correctly.
; ======================================================================

; indicate zero init completed
        ldr   r2, =zero_init_needed
        mov   r0, #0x0
        str   r0, [r2]

; Enter C code execution
        ;ldr     r13, =svc_stack+SVC_Stack_Size
;;loop_forever
;;		b loop_forever
        ldr     a1, =main_c
        bx      a1

        END
