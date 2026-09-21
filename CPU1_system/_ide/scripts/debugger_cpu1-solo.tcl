# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\CPU1_system\_ide\scripts\debugger_cpu1-solo.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\CPU1_system\_ide\scripts\debugger_cpu1-solo.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/Platform/hw/design_1_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
stop
targets -set -nocase -filter {name =~ "*A9*#1"}
rst -processor
targets -set -nocase -filter {name =~ "*A9*#1"}
dow C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU1/Debug/CPU1.elf
configparams force-mem-access 0
bpadd -addr &main
