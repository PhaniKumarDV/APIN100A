;*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
;
;        HW init/PMIC support - EMERGENCY DOWNLOADER IN RPM PROCESSOR
;
; GENERAL DESCRIPTION
;   This file contain PMIC related function that is used by the startup 
;   script to enable Scorpion processor
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
; $Header: //components/rel/boot.bf/3.1.1/boot_images/core/storage/tools/emmcbld/src/emmcbld_hw_init_8660.s#1 $
;
; when         who     what, where, why
; ----------   ---     --------------------------------------------------------
; 2011-10-20   ah      Initialize SSBI to enable forwarding clock
; 2011-08-18   ah      For Rev2.1 HW - changed MASK value to follow sbl1
; 2010-12-10   kpa     Update config for pmic 2.1.
; 2010-11-18   kpa     Update config for rev2.
; 2010-10-28   rh      Initial creation.
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
    IMPORT return_from_scorpion_init
  
;============================================================================
;
;                             MODULE EXPORTS
;
;============================================================================

    ; Export the external symbols that are referenced in this module.
    EXPORT scorpion_init_module
	 EXPORT apps_ss_start_in_ram
    EXPORT apps_ss_start_in_ram_end

;============================================================================
;
;                             MODULE DATA AREA
;
;============================================================================

    ;-------------------------------------------------------------------------
    ; Area code
    ;-------------------------------------------------------------------------
    PRESERVE8
    AREA    PMIC_CODE, CODE, READONLY
    CODE32

;------------------ Code to power up scorpion ----------------------------
;  This code is copied into a different memory region and run there
apps_ss_start_in_ram

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
           
apps_ss_start_in_ram_end

;------------------ Code to initialize scorpion ----------------------------
scorpion_init_module
   ;------------------------------------------------
   ; SETUP clocks
   ;-------------------------------------------------
   b ehostdl_setup_clocks
   ;Dont use link register to store address, here. To keep it
   ; simple use another label instead.
return_from_clocks
   b scorpion_REV2_MSM_pre_pmic_init
return_scorpion_REV2_MSM_pre_pmic_init
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
   ldr r3, =PA1_SSBI2_CTL_0x45005
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
   cmp r4, #0x1f
   ble else_case1
   
   ; Switch PMIC to SSBI 2.0 mode
   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x0450
   ldr r0, [r3]
   str r0, [r1]   
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status  
   
else_case1
   ; Switch MSM to SSBI 2.0 mode
   ldr r1, =HWIO_PA2_SSBI2_CTL_ADDR
   ldr r3, =PA2_SSBI2_CTL_0x45285
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
   
   cmp r5, #0x1f
   ble else_case2
   
   ; Switch PMIC to SSBI 2.0 mode
   ldr r1, =HWIO_PA1_SSBI2_CMD_ADDR
   ldr r3, =PA1_SSBI2_CMD_VAL_0x04f50
   ldr r0, [r3]
   str r0, [r1]
   ldr r1, =HWIO_PA1_SSBI2_RD_STATUS_ADDR
   ldr r2, =PA1_SSBI2_RD_STATUS_0x8000000
   ; store return address
   mov r14, pc 
   b chk_ready_flag_status    
   
else_case2
   ; Switch MSM to SSBI 2.0 mode
   ldr r1, =HWIO_PA1_SSBI2_CTL_ADDR
   ldr r3, =PA2_SSBI2_CTL_0x45285
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
   
   ;  if ((ready != 0xf1)&&(ready != 0xf2)&&(ready_8508!=0xe3))
   cmp r4, #0xf1
   beq not_an_error
   cmp r4, #0xf2
   beq not_an_error
   cmp r4, #0xf3     ; Pmic rev 8901-2.0
   beq not_an_error
   cmp r4, #0xf4     ; Pmic rev 8901-2.1
   beq not_an_error
   cmp r5, #0xe3
   beq not_an_error
   ; reached here means error.
   ; ERROR Do Nothing
   ;loop forever
loophere
   b loophere
   
not_an_error
   ; check if this is pm8901 (rev 1) pmic
   cmp r4, #0xf2
   bne else_case3
   b rpmsbl_hw_pm8901_setup
   ;  return from function. if we reach here then return directly
   ;  to "code_for_rpm" from "rpmsbl_hw_pm8901_setup". Rest of  
   ; code "else_case3" onwards not needed
   
else_case3
   ; check if this is pm8901 Rev 2 pmic
   cmp r4, #0xf3     ; config for pmic rev2.0
   beq rpmsbl_hw_pm8901_rev2_setup
   cmp r4, #0xf4    ; config for pmic rev2.1 
   bne else_case4
   b rpmsbl_hw_pm8901_rev2_setup
   ;  return from function. if we reach here then return directly
   ;  to "code_for_rpm" from "rpmsbl_hw_pm8901_rev2_setup". Rest of  
   ; code "else_case4" onwards not needed   

else_case4
   ; if we come here, this is 8058 pmic
   mov r14, pc 
   b dumb_busy_wait
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
   mov r14, pc 
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
   mov r14, pc 
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
   
   ; Check if its a Revision 2 MSM. If so, do addiditional configs
   b scorpion_init_REV2_MSM_post_pmic_init
    
; #### end of scorpion_init_module ###  

;-------------------------------------------------------------
    ; Setup for PMIC 8901
;-------------------------------------------------------------
rpmsbl_hw_pm8901_setup
   mov r14, pc 
   b dumb_busy_wait
   
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
   ;  mov pc, r14 ; jump back to caller
   ;  if we enter this function, rest of "scorpion_init_module" [caller]
   ;  is not needed. Go to "scorpion_init_REV2_MSM_post_pmic_init" to check 
   ;  for revision 2 MSM configs before returning back to code_for_rpm
   b scorpion_init_REV2_MSM_post_pmic_init
   
   ;   ### end of rpmsbl_hw_pm8901_setup ### 
   

;-------------------------------------------------------------
;   Setup for PMIC 8901  REVISION 2
;-------------------------------------------------------------
rpmsbl_hw_pm8901_rev2_setup
   mov r14, pc 
   b dumb_busy_wait

   ; --NEW for pmic 8091 -- 
   ; pmic cmd for 1.6MHz switching

    
   ;Max Soft Start 
   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_VAL3_0x6481
   ldr r0, [r3]
   str r0, [r1]    
   
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status

   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_VAL3_0x60FC
   ldr r0, [r3]
   str r0, [r1]    
   
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status  
   ; -- END of NEW for pmic 8091 --

    ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x5BA0
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
   ; pmic cmd for 1.6MHz switching   

   ;Max Soft Start 
   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x7381
   ldr r0, [r3]
   str r0, [r1]      
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status
   
   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x6FFC
   ldr r0, [r3]
   str r0, [r1]      
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status
   
   ; --- END of NEW for pmic 8091 ---

   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0x6AA0
   ldr r0, [r3]
   str r0, [r1]  
   ; check for status
   mov r14, pc 
   b chk_pa2_ssbi2_rd_status   

   ldr r1, =HWIO_PA2_SSBI2_CMD_ADDR
   ldr r3, =PA2_SSBI2_CMD_0xA7FF
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
   ;  mov pc, r14 ; jump back to caller
   ;  if we enter this function, rest of "scorpion_init_module" [caller]
   ;  is not needed. Go to "scorpion_init_REV2_MSM_post_pmic_init" to check 
   ;  for revision 2 MSM configs before returning back to code_for_rpm
   b scorpion_init_REV2_MSM_post_pmic_init
   
   ;   ### end of rpmsbl_hw_pm8901_rev2_setup ###  


   ;-------------------------------------------------------------
   ;   Check to see if its a REV2 MSM. If so do additional
   ;   required configurations for SCPU regs.
   ;-------------------------------------------------------------
scorpion_init_REV2_MSM_post_pmic_init
    ; Check  msm revision number
    ldr r1, =HWIO_HW_REVISION_NUMBER_ADDR
    ldr r0, [r1]
    ldr r3, =HW_REVISION_NUMBER_0xF0000000
    ldr r2, [r3]
    and r1, r0, r2 
    cmp r1, #0x0
    beq skip_msm_rev2_config     
    
    ;HWIO_SCPU0_SCSS_CPU_PWR_CTL_OUTM(
    ;           HWIO_SCPU0_SCSS_CPU_PWR_CTL_L2DT_SLP_BMSK,0x0); 
    ldr r1, =HWIO_SCPU0_SCSS_CPU_PWR_CTL_ADDR
    ldr r0, [r1]
    mov r2, #HWIO_SCPU0_SCSS_CPU_PWR_CTL_L2DT_SLP_BMSK
    ;invert bits of HWIO_SCPU0_SCSS_CPU_PWR_CTL_L2DT_SLP_BMSK
    ldr r3, =HWIO_SCPU0_SCSS_CPU_PWR_CTL_RMSK
    sub r2, r3, r2
    and r0, r0, r2
    str r0, [r1]
    
    ;HWIO_SCPU1_SCSS_CPU_PWR_CTL_OUTM(
    ;           HWIO_SCPU1_SCSS_CPU_PWR_CTL_L2DT_SLP_BMSK,0x0);   
    ldr r1, =HWIO_SCPU1_SCSS_CPU_PWR_CTL_ADDR
    ldr r0, [r1]
    mov r2, #HWIO_SCPU1_SCSS_CPU_PWR_CTL_L2DT_SLP_BMSK
    ;invert bits of HWIO_SCPU1_SCSS_CPU_PWR_CTL_L2DT_SLP_BMSK
    ldr r3, =HWIO_SCPU1_SCSS_CPU_PWR_CTL_RMSK
    sub r2, r3, r2
    and r0, r0, r2
    str r0, [r1] 
      
skip_msm_rev2_config 
    ; Return back to module that called scorpion_init_module
    b return_from_scorpion_init

   ;-------------------------------------------------------------
   ;   Check to see if its a REV2 MSM. If so do additional
   ;   required configurations for M2VMT regs.
   ;-------------------------------------------------------------
scorpion_REV2_MSM_pre_pmic_init
    ; Check  msm revision number
    ldr r1, =HWIO_HW_REVISION_NUMBER_ADDR
    ldr r0, [r1]
    ldr r3, =HW_REVISION_NUMBER_0xF0000000
    ldr r2, [r3]
    and r1, r0, r2 
    cmp r1, #0x0
    beq skip_msm_rev2_config_pre_pmic     
    
    
    ; Do additional M2VMR Rregister writes    
    mov r0, #0x0
    
    ;HWIO_SFPB_2x1M2VMT_M2VMR_ n_OUTI(0,0x00000000);
    ldr r1, =HWIO_SFPB_2x1M2VMT_M2VMR_ADDR(0)
    str r0, [r1]
    
    ;HWIO_SFPB_2x1M2VMT_M2VMR_ n_OUTI(1,0x00000000); 
    ldr r1, =HWIO_SFPB_2x1M2VMT_M2VMR_ADDR(1)
    str r0, [r1]

skip_msm_rev2_config_pre_pmic
    b return_scorpion_REV2_MSM_pre_pmic_init
    
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

        ;HWIO_OUT(SFAB_AHB_S1_FCLK_CTL, 0x10);
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
    DCD                 0x2e000

    ; Loop count for busy wait, set to 250000
loop_count
    DCD                 0x03D090

    ; Pmic register values
PA1_SSBI2_CMD_VAL_0x04f50
    DCD                 0x04f50    
PA1_SSBI2_RD_STATUS_0x8000000
    DCD                 0x8000000    
PA1_SSBI2_CTL_0x45285    
    DCD                 0x45285
PA2_SSBI2_CMD_0x0450 
    DCD                 0x0450
PA2_SSBI2_RD_STATUS_0x8000000
    DCD                 0x8000000
PA2_SSBI2_CTL_0x45285
    DCD                 0x45285
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
PA1_SSBI2_CTL_0x45005
    DCD                 0x045005
PA2_SSBI2_CMD_0x01000200
    DCD                 0x01000200     
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
PA2_SSBI2_CMD_0x0001CA85
    DCD                 0x0001CA85
PA2_SSBI2_CMD_0x00012F80
    DCD                 0x00012F80
PA2_SSBI2_CMD_0x0001CA84
    DCD                 0x0001CA84
PA2_SSBI2_CMD_0x00012F00
    DCD                 0x00012F00    
PA2_SSBI2_CMD_VAL3_0x6481
    DCD                 0x00006481
PA2_SSBI2_CMD_VAL3_0x60FC
    DCD                 0x000060FC
PA2_SSBI2_CMD_0x5BA0
    DCD                 0x00005BA0
PA2_SSBI2_CMD_0x7381
    DCD                 0x00007381
PA2_SSBI2_CMD_0x6FFC
    DCD                 0x00006FFC
PA2_SSBI2_CMD_0x6AA0
    DCD                 0x00006AA0
PA2_SSBI2_CMD_0xA7FF
    DCD                 0x0000A7FF
PA2_SSBI2_CMD_0x01013000
    DCD                 0x01013000
HW_REVISION_NUMBER_0xF0000000
    DCD                 0xF0000000
    END
    
