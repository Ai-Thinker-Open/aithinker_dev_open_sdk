;/**************************************************************************//**
; * @file     startup_ARMCM4.s
; * @brief    CMSIS Core Device Startup File for
; *           ARMCM4 Device Series
; * @version  V5.00
; * @date     02. March 2016
; ******************************************************************************/
;/*
; * Copyright (c) 2009-2016 ARM Limited. All rights reserved.
; *
; * SPDX-License-Identifier: Apache-2.0
; *
; * Licensed under the Apache License, Version 2.0 (the License); you may
; * not use this file except in compliance with the License.
; * You may obtain a copy of the License at
; *
; * www.apache.org/licenses/LICENSE-2.0
; *
; * Unless required by applicable law or agreed to in writing, software
; * distributed under the License is distributed on an AS IS BASIS, WITHOUT
; * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; * See the License for the specific language governing permissions and
; * limitations under the License.
; */

;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000600

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp               ;(0x00)Top of Stack
                DCD     Reset_Handler              ;(0x04)IRQ -15  Reset Handler
                DCD     NMI_Handler                ;(0x08)IRQ -14  NMI Handler
                DCD     HardFault_Handler          ;(0x0C)IRQ -13  Hard Fault Handler
                DCD     MemManage_Handler          ;(0x10)IRQ -12  MPU Fault Handler
                DCD     BusFault_Handler           ;(0x14)IRQ -11  Bus Fault Handler
                DCD     UsageFault_Handler         ;(0x18)IRQ -10  Usage Fault Handler
                DCD     0                          ;(0x1C)IRQ -9   Reserved
                DCD     0                          ;(0x20)IRQ -8   Reserved
                DCD     0                          ;(0x24)IRQ -7   Reserved
                DCD     0                          ;(0x28)IRQ -6   Reserved
                DCD     SVC_Handler                ;(0x2C)IRQ -5   SVCall Handler
                DCD     DebugMon_Handler           ;(0x30)IRQ -4   Debug Monitor Handler
                DCD     0                          ;(0x34)IRQ -3   Reserved
                DCD     PendSV_Handler             ;(0x38)IRQ -2   PendSV Handler
                DCD     SysTick_Handler            ;(0x3C)IRQ -1   SysTick Handler

                ; External Interrupts
                DCD     WDT_IRQHandler             ;(0x40)IRQ0 
                DCD     EXT_IRQHandler             ;(0x44)IRQ1
                DCD     RTC_IRQHandler             ;(0x48)IRQ2
                DCD     RFSLP_IRQHandler           ;(0x4C)IRQ3
                DCD     MAC_IRQHandler             ;(0x50)IRQ4
                DCD     BLE_WAKE_IRQHandler        ;(0x54)IRQ5
                DCD     BLE_ERR_IRQHandler         ;(0x58)IRQ6
                DCD     BLE_MAC_IRQHandler         ;(0x5C)IRQ7
                DCD     DMA_IRQHandler             ;(0x60)IRQ8
                DCD     QSPI_IRQHandler            ;(0x64)IRQ9
                DCD     SDIO_F1_IRQHandler         ;(0x68)IRQ10
                DCD     SDIO_F2_IRQHandler         ;(0x6C)IRQ11
                DCD     SDIO_F3_IRQHandler         ;(0x70)IRQ12
                DCD     CM4_FPIXC_IRQHandler       ;(0x74)IRQ13
                DCD     CM4_FPOFC_IRQHandler       ;(0x78)IRQ14
                DCD     CM4_FPUFC_IRQHandler       ;(0x7C)IRQ15
                DCD     CM4_FPIOC_IRQHandler       ;(0x80)IRQ16
                DCD     CM4_FPDZC_IRQHandler       ;(0x84)IRQ17
                DCD     CM4_FPIDC_IRQHandler       ;(0x88)IRQ18		
                DCD     I2C_IRQHandler             ;(0x8C)IRQ19
                DCD     SPI0_IRQHandler            ;(0x90)IRQ20
                DCD     SPI1_IRQHandler            ;(0x94)IRQ21
                DCD     UART0_IRQHandler           ;(0x98)IRQ22
                DCD     UART1_IRQHandler           ;(0x9C)IRQ23
                DCD     UART2_IRQHandler           ;(0xA0)IRQ24
                DCD     ADC_IRQHandler             ;(0xA4)IRQ25
                DCD     WS_IRQHandler              ;(0xA8)IRQ26
                DCD     I2S_IRQHandler             ;(0xAC)IRQ27
                DCD     GPIOA_IRQHandler           ;(0xB0)IRQ28
                DCD     GPIOB_IRQHandler           ;(0xB4)IRQ29
                DCD     TIMER0_IRQHandler          ;(0xB8)IRQ30
                DCD     TIMER1_IRQHandler          ;(0xBC)IRQ31
                DCD     TIMER2_IRQHandler          ;(0xC0)IRQ32
                DCD     TIMER3_IRQHandler          ;(0xC4)IRQ33
                DCD     ADV_TIMER_IRQHandler       ;(0xC8)IRQ34
                DCD     AES_IRQHandler             ;(0xCC)IRQ35
                DCD     TRNG_IRQHandler            ;(0xD0)IRQ36
                DCD     PAOTD_IRQHandler           ;(0xD4)IRQ37

__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors

                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main
                LDR     R4, =SystemInit
                BLX     R4
                LDR     R4, =__main
                BX      R4
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler         [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler          [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler        [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler          [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

Default_Handler PROC


                EXPORT     WDT_IRQHandler             [WEAK];(0x40)IRQ0 
                EXPORT     EXT_IRQHandler             [WEAK];(0x44)IRQ1
                EXPORT     RTC_IRQHandler             [WEAK];(0x48)IRQ2
                EXPORT	   RFSLP_IRQHandler           [WEAK];(0x4C)IRQ3
                EXPORT     MAC_IRQHandler             [WEAK];(0x50)IRQ4
                EXPORT	   BLE_WAKE_IRQHandler	      [WEAK];(0x54)IRQ5
                EXPORT	   BLE_ERR_IRQHandler         [WEAK];(0x58)IRQ6
                EXPORT	   BLE_MAC_IRQHandler         [WEAK];(0x5C)IRQ7
                EXPORT     DMA_IRQHandler             [WEAK];(0x60)IRQ8
                EXPORT     QSPI_IRQHandler            [WEAK];(0x64)IRQ9
                EXPORT     SDIO_F1_IRQHandler         [WEAK];(0x68)IRQ10
                EXPORT     SDIO_F2_IRQHandler         [WEAK];(0x6C)IRQ11
                EXPORT     SDIO_F3_IRQHandler         [WEAK];(0x70)IRQ12
                EXPORT     CM4_FPIXC_IRQHandler       [WEAK];(0x74)IRQ13
                EXPORT     CM4_FPOFC_IRQHandler       [WEAK];(0x78)IRQ14
                EXPORT     CM4_FPUFC_IRQHandler       [WEAK];(0x7C)IRQ15
                EXPORT     CM4_FPIOC_IRQHandler       [WEAK];(0x80)IRQ16
                EXPORT     CM4_FPDZC_IRQHandler       [WEAK];(0x84)IRQ17
                EXPORT     CM4_FPIDC_IRQHandler       [WEAK];(0x88)IRQ18		
                EXPORT     I2C_IRQHandler             [WEAK];(0x8C)IRQ19
                EXPORT     SPI0_IRQHandler            [WEAK];(0x90)IRQ20
                EXPORT     SPI1_IRQHandler            [WEAK];(0x94)IRQ21
                EXPORT     UART0_IRQHandler           [WEAK];(0x98)IRQ22
                EXPORT     UART1_IRQHandler           [WEAK];(0x9C)IRQ23
                EXPORT     UART2_IRQHandler           [WEAK];(0xA0)IRQ24
                EXPORT     ADC_IRQHandler             [WEAK];(0xA4)IRQ25
                EXPORT     WS_IRQHandler              [WEAK];(0xA8)IRQ26
                EXPORT     I2S_IRQHandler             [WEAK];(0xAC)IRQ27
                EXPORT     GPIOA_IRQHandler           [WEAK];(0xB0)IRQ28
                EXPORT     GPIOB_IRQHandler           [WEAK];(0xB4)IRQ29
                EXPORT     TIMER0_IRQHandler          [WEAK];(0xB8)IRQ30
                EXPORT     TIMER1_IRQHandler          [WEAK];(0xBC)IRQ31
                EXPORT     TIMER2_IRQHandler          [WEAK];(0xC0)IRQ32
                EXPORT     TIMER3_IRQHandler          [WEAK];(0xC4)IRQ33
                EXPORT     ADV_TIMER_IRQHandler       [WEAK];(0xC8)IRQ34
                EXPORT     AES_IRQHandler             [WEAK];(0xCC)IRQ35
                EXPORT     TRNG_IRQHandler            [WEAK];(0xD0)IRQ36
                EXPORT     PAOTD_IRQHandler           [WEAK];(0xD4)IRQ37
				

WDT_IRQHandler             ;(0x40)IRQ0 
EXT_IRQHandler             ;(0x44)IRQ1
RTC_IRQHandler             ;(0x48)IRQ2
RFSLP_IRQHandler           ;(0x4C)IRQ3
MAC_IRQHandler             ;(0x50)IRQ4
BLE_WAKE_IRQHandler        ;(0x54)IRQ5
BLE_ERR_IRQHandler         ;(0x58)IRQ6
BLE_MAC_IRQHandler         ;(0x5C)IRQ7
DMA_IRQHandler             ;(0x60)IRQ8
QSPI_IRQHandler            ;(0x64)IRQ9
SDIO_F1_IRQHandler         ;(0x68)IRQ10
SDIO_F2_IRQHandler         ;(0x6C)IRQ11
SDIO_F3_IRQHandler         ;(0x70)IRQ12
CM4_FPIXC_IRQHandler       ;(0x74)IRQ13
CM4_FPOFC_IRQHandler       ;(0x78)IRQ14
CM4_FPUFC_IRQHandler       ;(0x7C)IRQ15
CM4_FPIOC_IRQHandler       ;(0x80)IRQ16
CM4_FPDZC_IRQHandler       ;(0x84)IRQ17
CM4_FPIDC_IRQHandler       ;(0x88)IRQ18		
I2C_IRQHandler             ;(0x8C)IRQ19
SPI0_IRQHandler            ;(0x90)IRQ20
SPI1_IRQHandler            ;(0x94)IRQ21
UART0_IRQHandler           ;(0x98)IRQ22
UART1_IRQHandler           ;(0x9C)IRQ23
UART2_IRQHandler           ;(0xA0)IRQ24
ADC_IRQHandler             ;(0xA4)IRQ25
WS_IRQHandler              ;(0xA8)IRQ26
I2S_IRQHandler             ;(0xAC)IRQ27
GPIOA_IRQHandler           ;(0xB0)IRQ28
GPIOB_IRQHandler           ;(0xB4)IRQ29
TIMER0_IRQHandler          ;(0xB8)IRQ30
TIMER1_IRQHandler          ;(0xBC)IRQ31
TIMER2_IRQHandler          ;(0xC0)IRQ32
TIMER3_IRQHandler          ;(0xC4)IRQ33
ADV_TIMER_IRQHandler       ;(0xC8)IRQ34
AES_IRQHandler             ;(0xCC)IRQ35
TRNG_IRQHandler            ;(0xD0)IRQ36
PAOTD_IRQHandler           ;(0xD4)IRQ37
                B       .
                ENDP
                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, =(Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF


                END
