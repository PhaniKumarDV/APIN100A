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
; Copyright 2010    by Qualcomm Technologies, Incorporated.All Rights Reserved.
;*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

;*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
;
;                           EDIT HISTORY FOR FILE
;
; This section contains comments describing changes made to the module.
; Notice that changes are listed in reverse chronological order.
;
; $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_rpm_8660.s#1 $
;
; when       who     what, where, why
; --------   ---     --------------------------------------------------------
; 04/22/10   kpa     Initial creation.
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
  
;============================================================================
;
;                             MODULE EXPORTS
;
;============================================================================

    ; Export the external symbols that are referenced in this module.
    EXPORT rpm_loop_here

    ; Export the symbols __main and _main to prevent the linker from
    ; including the standard runtime library and startup routine.
    EXPORT   __rpm_main

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
    DCD     __emmcbld_entry ;  spbl_prefetch_abort_nested_handler
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
    ;-----------------------------------------------
    ; Do scorpion init without taking it out of reset
    ;-----------------------------------------------
    b scorpion_init_module
code_for_rpm
    ;-------------------------------------------------
    ; rpm moves code between "src_in_code_ram_start" and
    ; "src_in_code_ram_end" to CODE RAM
    ;-------------------------------------------------
    ldr r1, =src_in_code_ram_start
    ldr r0, =CODE_RAM_JUMP_ADDR
    ldr r2, [r0]
    ldr r3, =src_in_code_ram_end
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
   
src_in_code_ram_start

    ; *** This code runs in CODE RAM ***. 
    ;First, take Scorpion core 0 out of reset
    ;similar to HWIO_OUT( SCSS_CPU0CORE_RESET, 0x00000003 )

    ; Cannot rely on assembler to allocate value (DCD) for the address.
    ;  It does that in imem address space and we dont want that.
    ; Load r1 to have address 0x902d60 ie SCSS_CPU0CORE_RESET address.
    mov r1, #0x90
    lsl r1, r1, #0x8
    add r1,r1, #0x2d
    lsl r1, r1, #0x8
    add r1,r1, #0x60
    mov r0, #0x0003
    str r0, [r1]
   
    ; similar to HWIO_OUT( SCSS_CPU0CORE_RESET, 0x00000000 )
    mov r0, #0x0000
    ; r1 already has the SCSS_CPU0CORE_RESET value
    str r0, [r1]
    
    ; Now power up scorpion
   ; similar to HWIO_OUT( SCSS_DBG_STATUS_CORE_PWRDUP, 0x00000001 ); 
    ; Load r1 to have address 0x902e64 ie SCSS_DBG_STATUS_CORE_PWRDUP addr
    mov r1, #0x90
    lsl r1, r1, #0x8
    add r1,r1, #0x2e
    lsl r1, r1, #0x8
    add r1,r1, #0x64
    mov r0, #0x0001
    str r0, [r1]
    
    ; Rpm stays in code ram forever after releasing scorpion out of reset
rpm_loop_here
    b rpm_loop_here
           
src_in_code_ram_end

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
   

;------------------ Code to initialize scorpion ----------------------------
scorpion_init_module
    ;------------------------------------------------
    ; SETUP clocks
    ;-------------------------------------------------
   b ehostdl_setup_clocks
    ;Dont use link register to store address, here. To keep it
    ; simple use another label instead.
return_from_clocks
    ;------------------------------------------------------
    ; below code similar is to rpmsbl_hw_pre_boot_scorpion() 
    ;-------------------------------------------------------
    ;  HWIO_OUTI( SFPB_M2VMT_M2VMRn, 0, 0x0);
    ldr r1, =HWIO_SFPB_M2VMT_M2VMRn_ADDR(0)
   mov r0, #0x0
   str r0, [r1]
    ;  HWIO_OUTI( SFPB_M2VMT_M2VMRn, 1, 0x0);
    ldr r1, =HWIO_SFPB_M2VMT_M2VMRn_ADDR(1)
   mov r0, #0x0
   str r0, [r1]
    ;  HWIO_OUTI( SFPB_M2VMT_M2VMRn, 2, 0x0); 
    ldr r1, =HWIO_SFPB_M2VMT_M2VMRn_ADDR(2)
   mov r0, #0x0
   str r0, [r1]

    ldr r1, =HWIO_MPM_PMIC_SSBI_SEL_CFG_ADDR
   mov r0, #0xc
   str r0, [r1]
    ldr r1, =HWIO_PMIC_HDRV_PULL_CTL_ADDR
   ldr r3, =PMIC_HDRV_PULL_CTL_VAL
   ldr r0, [r3]
   str r0, [r1]
       
   ;New version sets both of these ctl regs before determining PMIC version 
    ldr r1, =HWIO_PA1_SSBI2_CTL_ADDR
   ldr r3, =PA1_SSBI2_CTL_0x41001
   ldr r0, [r3]
   str r0, [r1]
    ldr r1, =HWIO_PA2_SSBI2_CTL_ADDR
   str r0, [r1]
   
    ; LAUNCH A MODE1 read of the revision reg on PM8901
   ;Determine which version of 8901 PMIC chip
   
    ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x01000200
   ldr r0, [r3]
   str r0, [r1]   
   
   ; check for status
    ldr r1, =HWIO_PA2_SSBI2_RD_STATUS_ADDR
    ldr r2, =PA2_SSBI2_RD_STATUS_0x8000000
   ; store return address
   mov r14, pc 
   b chk_ready_flag_status    
    ;  r0 has read value
    ;-----------------------------------------+
   ;  store it to r4. It is the "ready" flag |
   ;-----------------------------------------+
   mov r4, r0;    
   and r4, r4, #0xff
   
   ;Detect PMIC 1.0/2.0 mode mismatch 
    ; if ((ready != 0xf1) && (ready != 0xf2))
    cmp r4, #0xf1
   beq else_case1
   cmp r4, #0xf2
   beq else_case1
   
   ; Need to enforce that MSM is in SSBI2.0 mode if PMIC is in 2.0 mode
   ldr r1, =HWIO_PA2_SSBI2_CTL_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x41281
   ldr r0, [r3]
   str r0, [r1]   
   
   ;Determine which version of PMIC chip
   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x01000200
   ldr r0, [r3]
   str r0, [r1]
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status  
   ; r0 contains the read value
   ; update r4 to hold this new value
   mov r4, r0
   and r4, r4, #0xff
   
else_case1  
    ;  Now determine which version of 8508 chip, should only be one currently,
    ;  but if unable to read version, that means we're in wrong mode.   
    ldr r1, =HWIO_PA1_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x01000200
   ldr r0, [r3]
   str r0, [r1]   
   
   ;  check for status
    ldr r1, =HWIO_PA1_SSBI2_RD_STATUS_ADDR
    ldr r2, =PA1_SSBI2_RD_STATUS_0x8000000
   ; store return address
   mov r14, pc 
   b chk_ready_flag_status 
   ; r0 contains the read value
   ; update r5 to hold this new value
   ; r5 is similar to "ready_8508" flag
   mov r5, r0
   and r5, r5, #0xff
   
    cmp r5, #0xe3
   beq else_case2
   
   ;  Need to enforce that MSM is in SSBI2.0 mode if PMIC8508 is in 2.0 mode 
    ldr r1, =HWIO_PA1_SSBI2_CTL_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x41281
   ldr r0, [r3]
   str r0, [r1]   
   
   ;   Determine which version of PMIC chip
   ldr r1, =HWIO_PA1_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x01000200
   ldr r0, [r3]
   str r0, [r1]
   ;   check for status
    ldr r1, =HWIO_PA1_SSBI2_RD_STATUS_ADDR
    ldr r2, =PA1_SSBI2_RD_STATUS_0x8000000
   ;  store return address
   mov r14, pc 
   b chk_ready_flag_status 
   ; r0 contains the read value
   ; update r5 to hold this new value
   ; r5 is similar to "ready_8508" flag
   mov r5, r0
   and r5, r5, #0xff
   
else_case2  
   ;  if ((ready != 0xf1)&&(ready != 0xf2)&&(ready_8508!=0xe3))
    cmp r4, #0xf1
   beq not_an_error
   cmp r4, #0xf2
   beq not_an_error
   cmp r5, #0xe3
    beq not_an_error
   ; reached here means error.
   ; ERROR Do Nothing
   ;loop forever
   b loophere
   
not_an_error
    cmp r4, #0xf2
   bne else_case3
   b rpmsbl_hw_pm8901_setup
   ;  return from function. if we reach here then return directly
    ;  to "code_for_rpm" from "rpmsbl_hw_pm8901_setup". Rest of  
   ; code "else_case3" onwards not needed
   
else_case3
    b dumb_busy_wait
   ldr r1, =HWIO_PA1_SSBI2_CMD_ADDR
   ldr r3, =PA1_SSBI2_CMD_VAL_0x04f50
   ldr r0, [r3]
   str r0, [r1]
    ldr r1, =HWIO_PA1_SSBI2_RD_STATUS_ADDR
    ldr r2, =PA1_SSBI2_RD_STATUS_0x8000000
   ; store return address
   mov r14, pc 
   b chk_ready_flag_status    
   ldr r1, =HWIO_PA1_SSBI2_CTL_ADDR
   ldr r3, =PA1_SSBI2_CTL_0x41281
   ldr r0, [r3]
   str r0, [r1]   
   ldr r1, =HWIO_PA2_SSBI2_CTL_ADDR
   ldr r3, =PA1_SSBI2_CTL_0x41001
   ldr r0, [r3]
   str r0, [r1]   
   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x0450
   ldr r0, [r3]
   str r0, [r1]   
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status  
   ldr r1, =HWIO_PA2_SSBI2_CTL_ADDR
   ldr r3, =PA2_SSBI2_CTL_0x41281
   ldr r0, [r3]
   str r0, [r1]   
   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x5B98
   ldr r0, [r3]
   str r0, [r1]      
   ;  check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status  
   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0xA67F
   ldr r0, [r3]
   str r0, [r1]      
   ;  check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status
    b dumb_busy_wait
   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x6A98
   ldr r0, [r3]
   str r0, [r1]      
   ;  check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status
   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0xA77F
   ldr r0, [r3]
   str r0, [r1]      
   ;  check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status  
   b dumb_busy_wait
   ;  The last PMIC wirte needs to be to bank0
   ;  Else write to AVS CTL does not work
   ;  Tristate PMIC MPP01 tied to ASIC GPIO48 (USIM_DATA) so GPIO48 can be
   ;  tested. This PMIC MPP only needs to be tristated on PM8058.

   ;  -- Setup Clamps and Clocks ---
   ldr r1, =HWIO_VDD_SC0_ARRAY_CLAMP_GFS_CTL_ADDR
   mov r0, #0x0
   str r0, [r1]   
   ldr r1, =HWIO_VDD_SC1_ARRAY_CLAMP_GFS_CTL_ADDR
   mov r0, #0x0
   str r0, [r1]   
   ;  The reset to Scorpion is ASSERTED BY DEFAULT ON POR
   ldr r1, =HWIO_SCSS_RESET_ADDR
   ldr r3, =HWIO_SCSS_RESET_SCSS_SYS_BMSK_VAL
   ldr r0, [r3]
   str r0, [r1]   
    ldr r1, =HWIO_SCSS_RESET_ADDR
   mov r0, #0x0
   str r0, [r1]
   ;Put a branch to self at the start of IMEM 
    ldr r1, =HWIO_SYS_IMEM_IMEM_RAM_CONFIG_ADDR
   mov r0, #0x0002
   str r0, [r1]   
   
   ;set Scorpion reset vector (should be 0x2a000000 by default)
    ldr r1, =HWIO_TCSR_RPM_VMID_NS_CTL_ADDR
   ldr r3, =TCSR_RPM_VMID_NS_CTL_VAL
   ldr r0, [r3]
   str r0, [r1]   
    ldr r1, =HWIO_SCSS_START_ADDR_ADDR
   ldr r3, =SCORPION_SCSS_START_ADDR
   ldr r0, [r3]
   str r0, [r1]   
    ldr r1, =HWIO_SCSS_L2_PWR_CTL_ADDR
   ldr r3, =SCSS_L2_PWR_CTL_0x00001F00
   ldr r0, [r3]
   str r0, [r1]   
   ldr r3, =SCSS_L2_PWR_CTL_0x00000200
   ldr r0, [r3]
   str r0, [r1]   
   
     b code_for_rpm ; jump back to code_for_rpm
    
; #### end of scorpion_init_module ###  

;-------------------------------------------------------------
    ; Setup for PMIC 8901
;-------------------------------------------------------------
rpmsbl_hw_pm8901_setup
   mov r14, pc 
   b dumb_busy_wait
   ldr r3, =PA1_SSBI2_CMD_VAL_0x04f50
   ldr r0, [r3]
   ldr r1, =HWIO_PA1_SSBI2_CMD_ADDR
   str r0, [r1]   
    ldr r1, =HWIO_PA1_SSBI2_RD_STATUS_ADDR
    ldr r2, =PA1_SSBI2_RD_STATUS_0x8000000
   ; store return address
   mov r14, pc 
   b chk_ready_flag_status   
   ldr   r1, =HWIO_PA1_SSBI2_CTL_ADDR
   ldr r3, =PA1_SSBI2_CTL_0x41281
   ldr r0, [r3]
   str r0, [r1]
    ldr  r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x0450
   ldr r0, [r3]
   str r0, [r1]

   mov r14, pc 
   b chk_pa2_ssbi2_rd_status
    ldr r1, =HWIO_PA2_SSBI2_CTL_ADDR
   ldr r3, =PA2_SSBI2_CTL_0x41281
   ldr r0, [r3]
   str r0, [r1]
   
    ; --NEW for pmic 8091 -- 
    ; pmic cmd for 4.8MHz switching
    ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_VAL2_0x5d31
   ldr r0, [r3]
   str r0, [r1]
   
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status
    
   ;Max Soft Start 
    ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_VAL3_0x64b1
   ldr r0, [r3]
   str r0, [r1]    
   
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status

    ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_VAL3_0x648F
   ldr r0, [r3]
   str r0, [r1]    
   
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status  
    ; -- END of NEW for pmic 8091 --

    ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x5B94
   ldr r0, [r3]
   str r0, [r1]   
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status
   
    ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0xA67F
   ldr r0, [r3]
   str r0, [r1]      
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status 
    ; wait for some time
   mov r14, pc
   b dumb_busy_wait

   ; --- NEW for pmic 8091 ---
     ; pmic cmd for 4.8MHz switching
    ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x6c31
   ldr r0, [r3]
   str r0, [r1]   
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status    

   ;Max Soft Start 
    ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x73b1
   ldr r0, [r3]
   str r0, [r1]      
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status
   
   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x738f
   ldr r0, [r3]
   str r0, [r1]      
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status
   
   ; --- END of NEW for pmic 8091 ---

    ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x6A94
   ldr r0, [r3]
   str r0, [r1]  
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status   

    ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0xA77F
   ldr r0, [r3]
   str r0, [r1]  
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status  
    ; wait for some time
   mov r14, pc
   b dumb_busy_wait

    ;The last PMIC wirte needs to be to bank0. (0x0?? Address)
    ;Else write to AVS CTL does not work.
    ;Tristate PMIC MPP01 tied to ASIC GPIO48 (USIM_DATA) so GPIO48 can be
    ;tested. This PMIC MPP only needs to be tristated on PM8058.

    
    ; --- Above  PMIC SETUP code -- 
    ; --- Setup CLAMS and clock below ----
   ldr r1, =HWIO_VDD_SC0_ARRAY_CLAMP_GFS_CTL_ADDR
   mov r0, #0x0
   str r0, [r1] 
    ldr r1, =HWIO_VDD_SC1_ARRAY_CLAMP_GFS_CTL_ADDR
   mov r0, #0x0
   str r0, [r1]   
   
   ;The reset to Scorpion is ASSERTED BY DEFAULT ON POR
    ldr r1, =HWIO_SCSS_RESET_ADDR
   ldr r3, =HWIO_SCSS_RESET_SCSS_SYS_BMSK_VAL
   ldr r0, [r3]
   str r0, [r1]   
    ldr r1, =HWIO_SCSS_RESET_ADDR
   mov r0, #0x0
   str r0, [r1]
   ;Put a branch to self at the start of IMEM 
    ldr r1, =HWIO_SYS_IMEM_IMEM_RAM_CONFIG_ADDR
   mov r0, #0x0002
   str r0, [r1]   
   
   ;set Scorpion reset vector (should be 0x2a000000 by default)
    ldr r1, =HWIO_TCSR_RPM_VMID_NS_CTL_ADDR
   ldr r3, =TCSR_RPM_VMID_NS_CTL_VAL
   ldr r0, [r3]
   str r0, [r1]   
    ldr r1, =HWIO_SCSS_START_ADDR_ADDR
   ldr r3, =SCORPION_SCSS_START_ADDR
   ldr r0, [r3]
   str r0, [r1]   
    ldr r1, =HWIO_SCSS_L2_PWR_CTL_ADDR
   ldr r3, =SCSS_L2_PWR_CTL_0x00001F00
   ldr r0, [r3]
   str r0, [r1]   
   ldr r3, =SCSS_L2_PWR_CTL_0x00000200
   ldr r0, [r3]
   str r0, [r1]         
   ;  using link reg this way is unsafe. supports only 1 level
   ;  of function recursion. else need to have stack
   ;mov pc, r14 ; jump back to caller
   ;  if we enter this function, rest of "scorpion_init_module" [caller]
   ;  is not needed. exit directly to "code_for_rpm" which 
   ;  calls "scorpion_init_module"
   b code_for_rpm
   
    ;   ### end of rpmsbl_hw_pm8901_setup ### 
    
    ;-------------------------------------------------------------
    ;                       Setup Clocks
    ;    similar to api clk_regime_rpm_init_boot_rpmsbl()
    ;-------------------------------------------------------------
ehostdl_setup_clocks

    ; ---- Enable AFAB [Apps Fabric] clocks  ----
    mov r0, #0x10;
    
        ;HWIO_OUT(AFAB_CORE_CLK_CTL, 0x10);
    ldr r1, =HWIO_AFAB_CORE_CLK_CTL_ADDR
    str r0, [r1]
    
        ;HWIO_OUT(AFAB_EBI1_S_ACLK_CTL, 0x10);
    ldr r1, =HWIO_AFAB_EBI1_S_ACLK_CTL_ADDR
    str r0, [r1]
    
        ;HWIO_OUT(AFAB_AXI_S0_FCLK_CTL, 0x10);
    ldr r1, =HWIO_AFAB_AXI_S0_FCLK_CTL_ADDR
    str r0, [r1]  
    
        ;HWIO_OUT(AFAB_AXI_S1_FCLK_CTL, 0x10);
    ldr r1, =HWIO_AFAB_AXI_S1_FCLK_CTL_ADDR
    str r0, [r1]
    
        ;HWIO_OUT(AFAB_AXI_S2_FCLK_CTL, 0x10);
    ldr r1, =HWIO_AFAB_AXI_S2_FCLK_CTL_ADDR
    str r0, [r1] 
    
        ;HWIO_OUT(AFAB_AXI_S3_FCLK_CTL, 0x10);
    ldr r1, =HWIO_AFAB_AXI_S3_FCLK_CTL_ADDR
    str r0, [r1]
    
        ;HWIO_OUT(AFAB_AXI_S4_FCLK_CTL, 0x10);
    ldr r1, =HWIO_AFAB_AXI_S4_FCLK_CTL_ADDR
    str r0, [r1]
    
    ; ---- Enable SFAB [System Fabric] clocks ----
        ;HWIO_OUT(SFAB_CORE_CLK_CTL, 0x10);
    mov r0, #0x10;
    ldr r1, =HWIO_SFAB_CORE_CLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AXI_S0_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AXI_S0_FCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AXI_S1_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AXI_S1_FCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AXI_S2_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AXI_S2_FCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AXI_S3_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AXI_S3_FCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AXI_S4_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AXI_S4_FCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AHB_S0_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AHB_S0_FCLK_CTL_ADDR
    str r0, [r1]

        ; HWIO_OUT(SFAB_AHB_S1_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AHB_S1_FCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AHB_S2_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AHB_S2_FCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AHB_S3_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AHB_S3_FCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AHB_S4_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AHB_S4_FCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AHB_S5_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AHB_S5_FCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AHB_S6_FCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AHB_S6_FCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_AFAB_M_ACLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_AFAB_M_ACLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(AFAB_SFAB_M0_ACLK_CTL, 0x10);
    ldr r1, =HWIO_AFAB_SFAB_M0_ACLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(AFAB_SFAB_M1_ACLK_CTL, 0x10);
    ldr r1, =HWIO_AFAB_SFAB_M1_ACLK_CTL_ADDR
    str r0, [r1]
   
  ;======================== CFPB clks ====================
     ;  Enable CFPB clks for SSBI PMIC
     
        ;HWIO_OUT(CFPB0_HCLK_CTL, 0x10);
    ldr r1, =HWIO_CFPB0_HCLK_CTL_ADDR      
    str r0, [r1]
    
        ;HWIO_OUT(CFPB1_HCLK_CTL, 0x10);
    ldr r1, =HWIO_CFPB1_HCLK_CTL_ADDR
    str r0, [r1]    
    
        ;HWIO_OUT(CFPB2_HCLK_CTL, 0x10);
    ldr r1, =HWIO_CFPB2_HCLK_CTL_ADDR
    str r0, [r1]
  
        ;HWIO_OUT(SFAB_CFPB_M_HCLK_CTL, 0x10);
     ldr r1, =HWIO_SFAB_CFPB_M_HCLK_CTL_ADDR
     str r0, [r1]
    
        ;HWIO_OUT(CFPB_MASTER_HCLK_CTL, 0x10);
    ldr r1, =HWIO_CFPB_MASTER_HCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFAB_CFPB_S_HCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_CFPB_S_HCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(CFPB_SPLITTER_HCLK_CTL, 0x10);
    ldr r1, =HWIO_CFPB_SPLITTER_HCLK_CTL_ADDR
    str r0, [r1]

        ;HWIO_OUT(SFPB_HCLK_CTL, 0x10);
    ldr r1, =HWIO_SFPB_HCLK_CTL_ADDR
    str r0, [r1]
    
        ;HWIO_OUT(SFAB_SFPB_M_HCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_SFPB_M_HCLK_CTL_ADDR
    str r0, [r1]
    
        ;HWIO_OUT(SFAB_SFPB_S_HCLK_CTL, 0x10);   
    ldr r1, =HWIO_SFAB_SFPB_S_HCLK_CTL_ADDR
    str r0, [r1]
    
     ; ---- Enable Scorpion clocks -----
     
     mov r0, #0x10
        ;HWIO_OUT(SCSS_DBG_CLK_CTL, 0x10);
    ldr r1, =HWIO_SCSS_DBG_CLK_CTL_ADDR
   str r0, [r1] 
    
        ;HWIO_OUT(SCSS_XO_SRC_CLK_CTL, 0x10);
    ldr r1, =HWIO_SCSS_XO_SRC_CLK_CTL_ADDR
   str r0, [r1]    
    
        ;HWIO_OUT(SFAB_SMPSS_S_HCLK_CTL, 0x10);
    ldr r1, =HWIO_SFAB_SMPSS_S_HCLK_CTL_ADDR
   str r0, [r1]

   mov r0, #0xA00    
        ;HWIO_OUTI(GPn_NS,0, 0xA00);
    ldr r1, =HWIO_GPn_NS_ADDR(0)
   str r0, [r1]    

        ;HWIO_OUTI(GPn_NS,1, 0xA00);
    ldr r1, =HWIO_GPn_NS_ADDR(1)
   str r0, [r1]    

        ;HWIO_OUTI(GPn_NS,2, 0xA00);
    ldr r1, =HWIO_GPn_NS_ADDR(2)
   str r0, [r1]


     ;----Vote RPM For all votable clks----
          
        ;HWIO_OUT(RPM_CLK_BRANCH_ENA_VOTE, 0x3FFF);
    ldr r1, =HWIO_RPM_CLK_BRANCH_ENA_VOTE_ADDR
    ldr r2, =RPM_CLK_BRANCH_ENA_VOTE_0x3FFF
    ldr r0, [r2]
    str r0, [r1]
    
        ;HWIO_OUT(MPM_CLK_CTL, 0x110);
    ldr r1, =HWIO_MPM_CLK_CTL_ADDR
    ldr r2, =MPM_CLK_CTL_0x110
    ldr r0, [r2]
    str r0, [r1]

        ;HWIO_OUT(DFAB_ARB0_HCLK_CTL, 0x10);
    ldr r1, =HWIO_DFAB_ARB0_HCLK_CTL_ADDR
    str r0, [r1]

    b return_from_clocks
    
;-------------------------------
dumb_busy_wait
    ; arbitrary delay [loop 1000000 times]
    ldr r0, =loop_count
   ldr r10, [r0]
dumbloop
    sub r10,r10, #0x1
    cmp r10, #0x0
   bne dumbloop
   mov pc, r14 ; jump back
   
;============== Function declarations =================
;---------------------------------
; name: CHK_READY_FLAG_STATUS
; args:   r1 = address to read from
;         r2 = bit mask value
; r3 is temporary register.
; return: r0 = r0 has read value
; the routine loops till the bit is cleared
;---------------------------------

chk_ready_flag_status
    ; loop till status reflects correct val.
    ldr r7, [r2]
status_loop_check
   ldr r0, [r1]
   ands r3, r0, r7
    ; loop back till r0 value is 0.
   beq status_loop_check 
    mov pc, r14

;---------------------------------
;name: CHK_PA2_SSBI2_RD_STATUS
; the routine loops till the bit is cleared
; its a special case of CHK_READY_FLAG_STATUS
; return: r0 = r0 has read value
;---------------------------------

chk_pa2_ssbi2_rd_status
    ldr r1, =HWIO_PA2_SSBI2_RD_STATUS_ADDR
   ldr r2, =PA2_SSBI2_RD_STATUS_0x8000000 
    ldr r7, [r2]
    ; loop till status reflects correct val.
status_loop_check2
   ldr r0, [r1]
   ands r3, r0, r7
    ; loop back till r0 value is 0.
   beq status_loop_check2 
    mov pc, r14

    
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
      DCD               0x2e000

    ; Loop count for busy wait, set to 250000
loop_count
      DCD               0x03D090

    ; Pmic register values
PA1_SSBI2_CMD_VAL_0x04f50
      DCD               0x04f50    
PA1_SSBI2_RD_STATUS_0x8000000
    DCD                 0x8000000    
PA1_SSBI2_CTL_0x41281    
    DCD                 0x41281
PA2_SSBI2_CMD_0x0450 
    DCD                 0x0450
PA2_SSBI2_RD_STATUS_0x8000000
    DCD                 0x8000000
PA2_SSBI2_CTL_0x41281
    DCD                 0x41281
PA2_SSBI2_CMD_VAL2_0x5d31
    DCD                 0x5d31
PA2_SSBI2_CMD_VAL3_0x64b1
    DCD                 0x64b1
PA2_SSBI2_CMD_0x5B98
    DCD                 0x5B98
PA2_SSBI2_CMD_0xA67F
    DCD                 0xA67F
PA2_SSBI2_CMD_0x6c31
    DCD                 0x6c31
PA2_SSBI2_CMD_0x73b1
    DCD                 0x73b1
PA2_SSBI2_CMD_0x6A98 
    DCD                 0x6A98
PA2_SSBI2_CMD_0xA77F
    DCD                 0xA77F
PA1_SSBI2_CMD_0x5000
    DCD                 0x5000
HWIO_SCSS_RESET_SCSS_SYS_BMSK_VAL
    DCD                 0x02
TCSR_RPM_VMID_NS_CTL_VAL
    DCD                 0x00800000  

    ;Release address (Reset vector) for scorpion
SCORPION_SCSS_START_ADDR
    DCD                 0x2A000000
SCSS_L2_PWR_CTL_0x00001F00
    DCD                 0x00001F00
SCSS_L2_PWR_CTL_0x00000200
    DCD                 0x00000200     
PMIC_HDRV_PULL_CTL_VAL
    DCD                 0x05000
PA1_SSBI2_CTL_0x41001
    DCD                 0x041001
PA2_SSBI2_CMD_0x01000200
    DCD                 0x01000200     
PA2_SSBI2_CMD_0x41281
    DCD                 0x041281
PA2_SSBI2_CMD_VAL3_0x648F
    DCD                 0x0648F     
PA2_SSBI2_CMD_0x5B94
    DCD                 0x05B94
PA2_SSBI2_CMD_0x738f
    DCD                 0x0738f
PA2_SSBI2_CMD_0x6A94
    DCD                 0x06A94
RPM_CLK_BRANCH_ENA_VOTE_0x3FFF
    DCD                 0x3fff
MPM_CLK_CTL_0x110
    DCD                 0x110
    
    END
    
