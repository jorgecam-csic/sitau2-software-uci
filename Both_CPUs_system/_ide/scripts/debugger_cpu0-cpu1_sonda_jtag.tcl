# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\Both_CPUs_system\_ide\scripts\debugger_cpu0-cpu1_sonda_jtag.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\Both_CPUs_system\_ide\scripts\debugger_cpu0-cpu1_sonda_jtag.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Platform Cable USB II 00001322b59d01" && jtag_device_ctx=="jsn-DLC10-00001322b59d01-4ba00477-0"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Platform Cable USB II 00001322b59d01" && level==0 && jtag_device_ctx=="jsn-DLC10-00001322b59d01-23731093-0"}
fpga -file C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU0/_ide/bitstream/design_1_wrapper.bit
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Platform Cable USB II 00001322b59d01" && jtag_device_ctx=="jsn-DLC10-00001322b59d01-4ba00477-0"}
loadhw -hw C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/HW_uci/export/HW_uci/hw/design_1_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Platform Cable USB II 00001322b59d01" && jtag_device_ctx=="jsn-DLC10-00001322b59d01-4ba00477-0"}
source C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU0/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0" && jtag_cable_name =~ "Platform Cable USB II 00001322b59d01" && jtag_device_ctx=="jsn-DLC10-00001322b59d01-4ba00477-0"}
dow C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU0/Debug/CPU0.elf
targets -set -nocase -filter {name =~ "*A9*#1" && jtag_cable_name =~ "Platform Cable USB II 00001322b59d01" && jtag_device_ctx=="jsn-DLC10-00001322b59d01-4ba00477-0"}
dow C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU1/Debug/CPU1.elf
configparams force-mem-access 0
bpadd -addr &main
