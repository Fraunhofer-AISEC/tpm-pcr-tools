/** @file
  GUID for EFI (NVRAM) Variables.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

  @par Revision Reference:
  GUID defined in UEFI 2.1
**/

#ifndef __GLOBAL_VARIABLE_GUID_H__
#define __GLOBAL_VARIABLE_GUID_H__

#include "UefiBaseType.h"

#define EFI_GLOBAL_VARIABLE \
  { \
    0x8BE4DF61, 0x93CA, 0x11d2, {0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C } \
  }

extern EFI_GUID  gEfiGlobalVariableGuid;

//
// Follow UEFI 2.4 spec:
// To prevent name collisions with possible future globally defined variables,
// other internal firmware data variables that are not defined here must be
// saved with a unique VendorGuid other than EFI_GLOBAL_VARIABLE or
// any other GUID defined by the UEFI Specification. Implementations must
// only permit the creation of variables with a UEFI Specification-defined
// VendorGuid when these variables are documented in the UEFI Specification.
//
// Note: except the globally defined variables defined below, the spec also defines
// u"Boot####"      - A boot load option.
// u"Driver####"    - A driver load option.
// u"SysPrep####"   - A System Prep application load option.
// u"Key####"       - Describes hot key relationship with a Boot#### load option.
// The attribute for them is NV+BS+RT, #### is a printed hex value, and no 0x or h
// is included in the hex value. They can not be expressed as a #define like other globally
// defined variables, it is because we can not list the Boot0000, Boot0001, etc one by one.
//

///
/// The language codes that the firmware supports. This value is deprecated.
/// Its attribute is BS+RT.
///
#define EFI_LANG_CODES_VARIABLE_NAME  u"LangCodes"
///
/// The language code that the system is configured for. This value is deprecated.
/// Its attribute is NV+BS+RT.
///
#define EFI_LANG_VARIABLE_NAME  u"Lang"
///
/// The firmware's boot managers timeout, in seconds, before initiating the default boot selection.
/// Its attribute is NV+BS+RT.
///
#define EFI_TIME_OUT_VARIABLE_NAME  u"Timeout"
///
/// The language codes that the firmware supports.
/// Its attribute is BS+RT.
///
#define EFI_PLATFORM_LANG_CODES_VARIABLE_NAME  u"PlatformLangCodes"
///
/// The language code that the system is configured for.
/// Its attribute is NV+BS+RT.
///
#define EFI_PLATFORM_LANG_VARIABLE_NAME  u"PlatformLang"
///
/// The device path of the default input/output/error output console.
/// Its attribute is NV+BS+RT.
///
#define EFI_CON_IN_VARIABLE_NAME   u"ConIn"
#define EFI_CON_OUT_VARIABLE_NAME  u"ConOut"
#define EFI_ERR_OUT_VARIABLE_NAME  u"ErrOut"
///
/// The device path of all possible input/output/error output devices.
/// Its attribute is BS+RT.
///
#define EFI_CON_IN_DEV_VARIABLE_NAME   u"ConInDev"
#define EFI_CON_OUT_DEV_VARIABLE_NAME  u"ConOutDev"
#define EFI_ERR_OUT_DEV_VARIABLE_NAME  u"ErrOutDev"
///
/// The ordered boot option load list.
/// Its attribute is NV+BS+RT.
///
#define EFI_BOOT_ORDER_VARIABLE_NAME  u"BootOrder"
///
/// The boot option for the next boot only.
/// Its attribute is NV+BS+RT.
///
#define EFI_BOOT_NEXT_VARIABLE_NAME  u"BootNext"
///
/// The boot option that was selected for the current boot.
/// Its attribute is BS+RT.
///
#define EFI_BOOT_CURRENT_VARIABLE_NAME  u"BootCurrent"
///
/// The types of boot options supported by the boot manager. Should be treated as read-only.
/// Its attribute is BS+RT.
///
#define EFI_BOOT_OPTION_SUPPORT_VARIABLE_NAME  u"BootOptionSupport"
///
/// The ordered driver load option list.
/// Its attribute is NV+BS+RT.
///
#define EFI_DRIVER_ORDER_VARIABLE_NAME  u"DriverOrder"
///
/// The ordered System Prep Application load option list.
/// Its attribute is NV+BS+RT.
///
#define EFI_SYS_PREP_ORDER_VARIABLE_NAME  u"SysPrepOrder"
///
/// Identifies the level of hardware error record persistence
/// support implemented by the platform. This variable is
/// only modified by firmware and is read-only to the OS.
/// Its attribute is NV+BS+RT.
///
#define EFI_HW_ERR_REC_SUPPORT_VARIABLE_NAME  u"HwErrRecSupport"
///
/// Whether the system is operating in setup mode (1) or not (0).
/// All other values are reserved. Should be treated as read-only.
/// Its attribute is BS+RT.
///
#define EFI_SETUP_MODE_NAME  u"SetupMode"
///
/// The Key Exchange Key Signature Database.
/// Its attribute is NV+BS+RT+AT.
///
#define EFI_KEY_EXCHANGE_KEY_NAME  u"KEK"
///
/// The public Platform Key.
/// Its attribute is NV+BS+RT+AT.
///
#define EFI_PLATFORM_KEY_NAME  u"PK"
///
/// Array of GUIDs representing the type of signatures supported
/// by the platform firmware. Should be treated as read-only.
/// Its attribute is BS+RT.
///
#define EFI_SIGNATURE_SUPPORT_NAME  u"SignatureSupport"
///
/// Whether the platform firmware is operating in Secure boot mode (1) or not (0).
/// All other values are reserved. Should be treated as read-only.
/// Its attribute is BS+RT.
///
#define EFI_SECURE_BOOT_MODE_NAME  u"SecureBoot"
///
/// The OEM's default Key Exchange Key Signature Database. Should be treated as read-only.
/// Its attribute is BS+RT.
///
#define EFI_KEK_DEFAULT_VARIABLE_NAME  u"KEKDefault"
///
/// The OEM's default public Platform Key. Should be treated as read-only.
/// Its attribute is BS+RT.
///
#define EFI_PK_DEFAULT_VARIABLE_NAME  u"PKDefault"
///
/// The OEM's default secure boot signature store. Should be treated as read-only.
/// Its attribute is BS+RT.
///
#define EFI_DB_DEFAULT_VARIABLE_NAME  u"dbDefault"
///
/// The OEM's default secure boot blacklist signature store. Should be treated as read-only.
/// Its attribute is BS+RT.
///
#define EFI_DBX_DEFAULT_VARIABLE_NAME  u"dbxDefault"
///
/// The OEM's default secure boot timestamp signature store. Should be treated as read-only.
/// Its attribute is BS+RT.
///
#define EFI_DBT_DEFAULT_VARIABLE_NAME  u"dbtDefault"
///
/// Allows the firmware to indicate supported features and actions to the OS.
/// Its attribute is BS+RT.
///
#define EFI_OS_INDICATIONS_SUPPORT_VARIABLE_NAME  u"OsIndicationsSupported"
///
/// Allows the OS to request the firmware to enable certain features and to take certain actions.
/// Its attribute is NV+BS+RT.
///
#define EFI_OS_INDICATIONS_VARIABLE_NAME  u"OsIndications"
///
/// Whether the system is configured to use only vendor provided
/// keys or not. Should be treated as read-only.
/// Its attribute is BS+RT.
///
#define EFI_VENDOR_KEYS_VARIABLE_NAME  u"VendorKeys"

#endif
