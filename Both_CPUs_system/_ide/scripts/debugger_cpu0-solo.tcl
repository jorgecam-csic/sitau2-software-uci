# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\Both_CPUs_system\_ide\scripts\debugger_cpu0-solo.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\Both_CPUs_system\_ide\scripts\debugger_cpu0-solo.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent JTAG-SMT2 210251B45391" && level==0 && jtag_device_ctx=="jsn-JTAG-SMT2-210251B45391-23731093-0"}
fpga -file C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU0/_ide/bitstream/design_1_wrapper.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/HW_uci/export/HW_uci/hw/design_1_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU0/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU0/Debug/CPU0.elf
configparams force-mem-access 0
bpadd -addr &main
