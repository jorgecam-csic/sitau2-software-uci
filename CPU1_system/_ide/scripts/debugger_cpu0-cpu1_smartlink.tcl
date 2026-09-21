# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\CPU1_system\_ide\scripts\debugger_cpu0-cpu1_smartlink.tcl
# 
# 
# Usage with xsct:
# In an external shell use the below command and launch symbol server.
# symbol_server.bat -S -s tcp::1534
# To debug using xsct, launch xsct and run below command
# source C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\CPU1_system\_ide\scripts\debugger_cpu0-cpu1_smartlink.tcl
# 
connect -path [list tcp::1534 tcp:10.0.0.2:3121]
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "JTAG Cable 2022.2.2 AAo1B+WJ0" && level==0 && jtag_device_ctx=="jsn-XSC0-AAo1B+WJ0-23731093-0"}
fpga -file C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU0/_ide/bitstream/design_1_wrapper.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/Platform/hw/design_1_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU0/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU0/Debug/CPU0.elf
targets -set -nocase -filter {name =~ "*A9*#1"}
dow C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU1/Debug/CPU1.elf
configparams force-mem-access 0
bpadd -addr &main
