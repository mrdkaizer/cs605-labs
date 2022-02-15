#!/bin/bash

# Disable intel_pstate driver to gain control over P-states
kernel_boot_params+=" intel_pstate=disable"

# Disable intel_idle driver to gain control over C-states (this driver will
# most ignore any other BIOS setting and kernel parameters). Then limit
# available C-states to C1 by "idle=halt".
#kernel_boot_params+=" intel_idle.max_cstate=0 idle=halt"

# Update GRUB with our kernel boot parameters
sed -i "s/GRUB_CMDLINE_LINUX_DEFAULT=\"/GRUB_CMDLINE_LINUX_DEFAULT=\"$kernel_boot_params /" /etc/default/grub
update-grub
