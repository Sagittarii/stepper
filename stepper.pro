#CONFIG -= qt
#TARGET = stepper

#QMAKE_CC = arm-none-eabi-gcc
#QMAKE_CXX = arm-none-eabi-g++

#QMAKE_CFLAGS  = -g -O2 -Wall -Tstm32_flash.ld
#QMAKE_CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
#QMAKE_CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

#QMAKE_LFLAGS  = -g -O2 -Wall -Tstm32_flash.ld
#QMAKE_LFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
#QMAKE_LFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

#arm-none-eabi-gcc 
#-g -O2 -Wall -Tstm32_flash.ld  
#-mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
#-mfloat-abi=hard -mfpu=fpv4-sp-d16 
# -o stepper_drive.elf

INCLUDEPATH += inc \
#        lib/STM32F4xx_StdPeriph_Driver/inc \
#        lib/STM32F4xx_StdPeriph_Driver/inc/device_support \
#        lib/STM32F4xx_StdPeriph_Driver/inc/core_support

DEFINES += USE_USB_OTG_FS=1

INCLUDEPATH += lib/STM32_USB_Device_Library/Class/cdc/inc \
        lib/STM32_USB_Device_Library/Core/inc \
        lib/STM32_USB_OTG_Driver/inc \
        usb

SOURCES += src/main.c \
        src/sys.c \
        src/leds.c \
        src/stepper.c \
        src/inputs.c \
        src/system_stm32f4xx.c \

SOURCES += lib/startup_stm32f4xx.s

SOURCES += lib/STM32_USB_OTG_Driver/src/usb_dcd_int.c \
        lib/STM32_USB_OTG_Driver/src/usb_dcd.c \
        lib/STM32_USB_OTG_Driver/src/usb_core.c \
        usb/usb_bsp.c

SOURCES += lib/STM32_USB_Device_Library/Core/src/usbd_core.c \
        lib/STM32_USB_Device_Library/Core/src/usbd_req.c \
        lib/STM32_USB_Device_Library/Core/src/usbd_ioreq.c \
        lib/STM32_USB_Device_Library/Class/cdc/src/usbd_cdc_core.c

SOURCES += usb/usbd_cdc_vcp.c \
        usb/usbd_desc.c \
        usb/usbd_usr.c \
        usb/stm32f4xx_it.c \
        usb/syscalls.c

#LIBS += -Llib/STM32F4xx_StdPeriph_Driver/build -lSTM32F4xx_StdPeriph_Driver
include(lib/STM32F4xx_StdPeriph_Driver/STM32F4xx_StdPeriph_Driver.pri)

#QMAKE_POST_LINK += arm-none-eabi-objcopy -O ihex stepper stepper.hex && arm-none-eabi-objcopy -O binary stepper stepper.bin