;*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
;
;         STARTUP - EMERGENCY DOWNLOADER IN RPM PROCESSOR
;
; GENERAL DESCRIPTION
;   This file sets up scorpion processor via rpm. The RPM uses this Start-up
;   Emergency Downloader (ehostdl) file to configure scorpion, bring it out
;   of reset and then transfer control to it.
;   
;
; EXTERNALIZED SYMBOLS
;   __main
;   _main
;
; INITIALIZATION AND SEQUENCING REQUIREMENTS
;
;
; Copyright 2011    by Qualcomm Technologies, Incorporated.All Rights Reserved.
;*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

;*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
;
;                           EDIT HISTORY FOR FILE
;
; This section contains comments describing changes made to the module.
; Notice that changes are listed in reverse chronological order.
;
; $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_rpm_start.s#1 $
;
; YYYY-MM-DD   who     what, where, why
; ----------   ---     --------------------------------------------------------
; 2011-06-29   ab      Move 8660 scorpion start code to emmcbld_hw_init_8660.s
; 2010-10-29   rh      Splitting up the startup script
;*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*


;============================================================================
;
;                            MODULE INCLUDES
;
;============================================================================

#include "msmhwioreg.h"

; Include header for ssbi hwio defines.
#include "HALhwioSBI.h"

; Header inclusion for SCSS addresses like SCSS_START_ADDR
#include "msm.h"



;============================================================================
;
;                             MODULE DEFINES
;
;============================================================================
;
Mode_SVC                EQU    0x13
Mode_ABT                EQU    0x17
Mode_UND                EQU    0x1b
Mode_USR                EQU    0x10
Mode_FIQ                EQU    0x11
Mode_IRQ                EQU    0x12

I_Bit                   EQU    0x80
F_Bit                   EQU    0x40


;============================================================================
;
;                             MODULE IMPORTS
;
;============================================================================
    IMPORT __emmcbld_entry
    IMPORT scorpion_init_module
  
  	 IMPORT apps_ss_start_in_ram
    IMPORT apps_ss_start_in_ram_end

    IMPORT  |Image$$APP_RAM$$ZI$$Base|
    IMPORT  |Image$$APP_RAM$$ZI$$Limit|

  
;============================================================================
;
;                             MODULE EXPORTS
;
;============================================================================

    ; Export the external symbols that are referenced in this module.
    ;;EXPORT rpm_loop_here
    EXPORT return_from_scorpion_init

    ; Export the symbols __main and _main to prevent the linker from
    ; including the standard runtime library and startup routine.
    EXPORT   __rpm_main
    


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
    ldr     r0, =|Image$$APP_RAM$$ZI$$Base|
    ldr     r1, =|Image$$APP_RAM$$ZI$$Limit|
    mov     r4, #0x1                   
    mov     r2,#0
init_zi_label
    cmp     r0,r1          ; unsigned compare, clear c on borrow
    strcc   r2,[r0],#4     ; str 0 in [r0] if r0 < r1
    bcc     init_zi_label  ; loop while ro < r1
  MEND

;============================================================================
;
;                             MODULE DATA AREA
;
;============================================================================

    ;-------------------------------------------------------------------------
    ; Area RPM_ENTRY_POINT resides in IMEM and is the start of ehostdl code.
    ; The location is defined in scatter load file.
    ;-------------------------------------------------------------------------
    PRESERVE8
    AREA    RPM_ENTRY_POINT, CODE, READONLY
    CODE32
    ENTRY

__rpm_main
    ;--------------------------------------------------------------------------
    ; Exception vectors table located at the top of IMEM.  This is initialized
    ; to the Ehostdl exception handlers.  
    ;--------------------------------------------------------------------------

;============================================================================
; The aARM exception vector table is located where the aARM starts running
; when released from reset.  The ehostdl vector table will contain long
; branches that allow branching anywhere within the 32 bit address space.
; Each long branch consists of a 32 bit LDR instruction using the PC
; relative 12 bit immediate offset address mode (ARM address mode 2).
;============================================================================
    b       rpm_reset_handler                          ; reset vector

;============================ IMPORTANT =====================================
;
;  The area EMMCBLD_INDIRECT_VECTOR_TABLE follows area RPM_ENTRY_POINT.
;  EMMCBLD_INDIRECT_VECTOR_TABLE area ** MUST ALWAYS ** be located at an
;   offset "CODE_START_ADDR +0x4" which normally evaluates to 
;   address 0x2a000054.  Initially upon reset scorpion reset vector table
;   is not set. RPM prepares it.
;    1. It first configures pmic and clock for scorpion.
;    2. Prepares scorpion exception vector table. it does this by copying the 
;       predetermined opcodes to scorpion reset address. The hard-coded opcodes
;       EXPECT the indirect table to start at above offset [eg 0x2a000054] .
;    3. The rpm then copies code that releases scorpion, to coderam and jumps
;        to it from imem. This is to fix hardware issue, that rpm and scorpion
;        cannot access imem at same time.
;    4. Rpm then releases scorpion. RPM loops forver in Coderam, Scorpion
;        starts in IMEM
;
;   * The ehostdl code is compiled for scorpion environment. but a part of it 
;     is rpm compatible and is executed by rpm for taking scorpion out of
;     rest.
;
;   Image Layout
;   +-----------+ 0x2a000000
;   |           |     ehostdl hex file header
;   +-----------+ 0x2a000050;
;   |           |   (actual code start) rpm_reset_handler
;   +-----------+ 0x2a000054
;   |           |     EMMCBLD_INDIRECT_VECTOR_TABLE start
;   |           |
;   |           |
;   +-----------+
;   |           |rest of the ehostdl image code
;   +-----------+
;   ehostdl header region (0x2a000000) later gets replaced with scopion vector table.
;   
;============================================================================
vector_table
    DCD     __emmcbld_entry
    DCD     __emmcbld_entry ; spbl_undefined_instruction_nested_handler
    DCD     __emmcbld_entry ; spbl_swi_c_handler
    DCD     __emmcbld_entry ; spbl_prefetch_abort_nested_handler
    DCD     __emmcbld_entry ; spbl_data_abort_nested_handler
    DCD     __emmcbld_entry ; spbl_reserved_c_handler
    DCD     __emmcbld_entry ; spbl_irq_c_handler
    DCD     __emmcbld_entry ; spbl_fiq_c_handler


;============================================================================
; Qualcomm EHOSTDL ENTRY POINT
;============================================================================

    AREA  RPM_CODE_REGION, CODE, READONLY, ALIGN=4
    CODE32

rpm_reset_handler
    ; Zero out memory...
    init_zi
    ;-----------------------------------------------
    ; Do scorpion init without taking it out of reset
    ;-----------------------------------------------
    b scorpion_init_module
return_from_scorpion_init
code_for_rpm
    ;-------------------------------------------------
    ; rpm moves code between "src_in_code_ram_start" and
    ; "src_in_code_ram_end" to CODE RAM
    ;-------------------------------------------------
    ldr r1, =apps_ss_start_in_ram
    ldr r0, =CODE_RAM_JUMP_ADDR
    ldr r2, [r0]
    ldr r3, =apps_ss_start_in_ram_end
copy_code
    cmp r1, r3 ; compare end address
    beq rpm_do_remaining_task
    ldr r0, [r1]
    str r0, [r2]
    add r1, r1, #4
    add r2, r2, #4
    b copy_code

rpm_do_remaining_task
    ;---------- Prepare Scorpion Exception Handler Vector Table ----------------------
    ; replace the imem address 0x2a000000 to have opcode for branch to scorpion
    ; reset handler. By default, the address has ehostdl hex image header
    
   ; For loop to move the opcodes for 8 exception handlers
   ldr r0, =SCORPION_SCSS_START_ADDR
   ldr r1, [r0]
   ldr r2, =SCORPION_ENTRY_HANDLER
   mov r3, #08   ; move count
move_opcode
   ldr r0, [r2]  ;load the opcode
   str r0, [r1]  ; store it at reset vect table offset
   add r1, r1, #04 ; point to next location
   add r2, r2, #04
   subs r3, r3, #01
   bne move_opcode
    
   ; now make rpm jump to code ram. When in Code_ram rpm brings scorpion
   ; out of reset and itself loops forever as its no longer needed.
   ldr r1, =CODE_RAM_JUMP_ADDR
   ldr r0, [r1]
   bx r0 ; jump to coderam
   
loophere
   b loophere
;----------------------------------------------------------------------------
;Appropriate ehostdl scorpion handlers would later replace this handler
dummy_exception_handler
   b  loophere

;----------------------------------------------------------------------------
   ; offset of 56 bytes. Indirect Vector table start address is 0x2a000038.
   ; rpmsbl entry point at 0x2a000034
   ; exception table bridge starts at 0x2a000038
   ; below are the opcodes for "ldr pc, 0x2a000038" instruction compiled
   ; for 0x2a000000 addr
   ; ie similar to "d.a 0x2a000000 ldr pc, 0x2a000034" opcode
SCORPION_ENTRY_HANDLER
   dcd 0xE59FF04C  ; opcode for instn "ldr pc, 0x2a000054" at 0x2a000000
   dcd 0xE59FF04C  ; opcode for instn "ldr pc, 0x2a000058" at 0x2a000004
   dcd 0xE59FF04C  ; opcode for instn "ldr pc, 0x2a00005c" at 0x2a000008
   dcd 0xE59FF04C  ; opcode for instn "ldr pc, 0x2a000060" at 0x2a00000c
   dcd 0xE59FF04C  ; opcode for instn "ldr pc, 0x2a000064" at 0x2a000010
   dcd 0xE59FF04C  ; opcode for instn "ldr pc, 0x2a000068" at 0x2a000014
   dcd 0xE59FF04C  ; opcode for instn "ldr pc, 0x2a00006c" at 0x2a000018
   dcd 0xE59FF04C  ; opcode for instn "ldr pc, 0x2a000070" at 0x2a00001c


;============= Constant declarations =================
   ; the #define approach cannot be used. Though the assembler compiles the code,
   ; the opcode results in undefined instruction for rpm (for some values) 
   ; and it crashes.
   ;eg #define RPM_CLK_BRANCH_ENA_VOTE_0x3FFF 0x3FFF 
   ;ldr r0, =RPM_CLK_BRANCH_ENA_VOTE_0x3FFF
   ;ie ldr r0, =0x3FFF -> undefined opcode for rpm
   ; Hence DCD approach needed

   
   ;Address in code_ram where rpm copies code from IMEM.
   ;This code brings scorpion out of reset.This needs to be
   ; done from code ram due to hw limitation. rpm and scorpion
   ;cannot access imem simultaneously.
CODE_RAM_JUMP_ADDR
   DCD                 0x2e000
SCORPION_SCSS_START_ADDR
   DCD                 0x2A000000

   END
    
