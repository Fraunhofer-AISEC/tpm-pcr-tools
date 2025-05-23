# README

This tool calculates the values of the SRTM PCRs. It takes the software running on the platform as
input and outputs the PCR summary or the detailed eventlog in a text format or json format.
The tool is work in progress and currently only supports x86 systems started by QEMU with OVMF and a
Linux kernel as PE/COFF image.

- PCR0 UEFI is calculated based on the OVMF firmware
- PCR1 UEFI Configuration is calculated using the EFI boot variables
- PCR2 is calculated based on optionally specified 3rd party UEFI drivers
- PCR3 is calculated based on optionally specified 3rd party UEFI driver configurations
- PCR4 UEFI Boot Manager Code is calculated from the kernel PE/COFF image
- PCR5 UEFI Boot Manager Configuration currently only supports event EFI actions being present
- PCR6 Host platform Manufacturer specific, currently supports only EV_SEPARATOR present
- PCR7 Secure Boot Policy, is calculated from the Secure Boot variables, currently supports only secure boot disabled

## Prerequisites

```sh
sudo apt install -y build-essential
```

## Build

```sh
make
```

## Install

```sh
sudo make install
```

## Usage

```sh
calculate-srtm-pcrs --help
```

### Example Usage

```sh
./calculate-srtm-pcrs \
	--ovmf OVMF.fd \
	--kernel linux-amd64-virtio-systemd-debug.bzImage \
	--config configs/default.cfg \
	--acpirsdp configs/etc-acpi-rsdp \
    --acpitables configs/etc-acpi-tables \
    --tableloader configs/etc-table-loader \
	--tpmlog configs/etc-tpm-log \
	--pcrs 0,1,3,4,5,6,7 \
	--format json \
	--eventlog \
	--summary \
```

## Configuration

The kernel image contains a setup header for exchanging values with the bootloader [2]. A part of
these values is "modify" or "write", which means that the bootloader writes these values. As these
values are written before the header is measured as part of the kernel image into PCR4, they must
be set configured.

Therefore, for calculating PCR0 and PCR4, a configuration file path must be specified via the
`-c|--config` command line argument. Example configurations are provided in `configs/`.

- [2] https://www.kernel.org/doc/html/latest/x86/boot.html?highlight=boot

## Supported OVMF versions

- Versions prior to edk2-stable202202 are untested
- In versions between edk2-stable202205 and edk2-stable202302 the PEIFV measures the values of
various global variables, when it measures itself. They are only supported in older versions of this
software
- Versions edk2-stable202311 to edk2-stable202408.01 are supported
- Versions edk2-stable202411 and edk2-stable202502 introduce EFI delayed dispatch functionality.
This might cause the global variable mAcpiTimerIoAddr to be measured, hindering reproducibility.
This can be fixed through setting the ACPI timer IO address to a constant value or reverting
OVMF commits d64d1e195ceb003c824786bfd817227c7ae81f06 and
79598f34fa024ad2491b46b95c3189c5a7f159d2 if delayed dispatch functionality is not required.
