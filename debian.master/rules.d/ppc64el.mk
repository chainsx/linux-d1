human_arch	= PowerPC 64el
build_arch	= powerpc
header_arch	= $(build_arch)
defconfig	= pseries_le_defconfig
flavours	= generic
build_image	= vmlinux.strip
kernel_file     = arch/powerpc/boot/vmlinux.strip
install_file	= vmlinux
no_dumpfile	= true
do_extras_package = true
opal_signed       = false
do_tools_usbip    = true
do_tools_cpupower = true
do_tools_perf	  = true
do_tools_perf_jvmti = true
do_tools_bpftool  = true

#do_flavour_image_package = false
