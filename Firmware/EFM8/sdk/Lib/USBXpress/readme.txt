USBXpress Firmware Library

Overview
----------------------------
USBXpress is pre-compiled firmware library. It simplifies USB firmware
development by providing a simple, bi-directional bulk data pipe as well as a
callback function with various USB events (device opened,  device closed, tx 
complete, rx complete, etc.). It communicates with the host via the WinUSB
driver and a DLL provided by Silicon Labs called SiUSBXp.dll (see 
https://www.silabs.com/products/mcu/Pages/USBXpress.aspx for the DLL download).
The firmware library supports Microsoft OS Descriptors, which allows the host
Windows machine to associate the device to the WinUSB driver without any .inf files.

Target Devices
----------------------------
  - EFM8UB1
  - EFM8UB2
  - C8051F380
  - C8051F340
  - C8051F326
  - C8051F320

Known Issues and Limitations
----------------------------
  - The C8051F32x and F34x devices will not stay in suspend mode when VBUS is
    removed in a self-powered configuration.

Version Information:
----------------------------

Version 5.0:
  - Modified API and file names.
  - Added support for multi-packet transfers.
  - Added support for Microsoft OS Descriptors.
  - Added USBX_suspend() function.
  - Added USBX_STRING_DESC macro for easy string descriptor definitions.

Version 4.0:
  - Ported to the WinUSB driver.
  - Library now distributed in Simplicity Studio.
  - Removed support for C8051T62x devices.

Version 3.5:
- Added support for C8051F38x, C8051T620_1_T320_3, and C8051T622_3_T326_7
  device families.

Version 3.4:
  - Added support for Raisonance; Keil, Raisonance, and SDCC libraries are
    now included.
  - Fixed Block_Write to correct issue in v3.3 library for writes greater than
    63 bytes.
  
Version 3.3:
  - Added fix to STALL bad feature requests
  - Added support for SDCC; Both Keil and SDCC libraries are now included.
  - Added support for vendor-specific request "GetPartNum"

Version 2.4:
  - USBX_F320_1.LIB firmware library is compatible with 'F320/1 and 'F34x
    devices.
  - Added FileTransfer and TestPanel examples for 'F34x devices.
    - Please note that these examples are the same as those for 'F32x device
      because those devices  are code-compatible.
  - Added new firmware library for C8051F326/7 devices (USBX_F326_7.LIB).
  - Added FileTransfer example for C8051F326/7 devices.
  - If you are porting an existing application from 'F320/1 to
    'F326/7, please note that these two families are not exactly
    code-compatible. See the device datasheets for more details.
  - Added function USB_Get_Library_Version to library that returns
    a 2-byte BCD value (0x0241=2.41); Previously, the only way 
    to determine the version was by the file date.
  - Fixed issue in v2.3 library that ignored a IN endpoint halt command.

Version 2.3:
  - Library renamed from USB_API.LIB to USBX_F320_1.LIB
  - USB_Clock_Start() function added to improve flexibility.
  - Fixed USB_Disable() hang issue.
  - Eliminated unnecessary delay in Block_Write() when sending a zero-length
    packet.
  - Corrected USB clock startup code to check if clock multiplier is already
    enabled.
  - Removed default values for USB_Init() parameters to save code space.
    NULL cannot be used any more to choose defaults.

Verison 2.1:
  - FlushBuffers IO Control Request fixed for F32x devices, make sure to update
    both the device driver and the dll from your previous version.

Version 2.0:
  - ResetDevice function removed.

Version 1.2:
  - Fixed Keil Linker incompatibility which would give the following linker
    error: Fatal error L251: Restricted module not supported.
    
Version 1.1:
  - Fixed Windows 98SE issue where the serial number string was not being
    returned.

Version 1.0
  - Initial Release