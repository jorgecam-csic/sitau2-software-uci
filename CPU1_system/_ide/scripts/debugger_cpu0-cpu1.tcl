# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\CPU1_system\_ide\scripts\debugger_cpu0-cpu1.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\CPU1_system\_ide\scripts\debugger_cpu0-cpu1.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Digilent JTAG-SMT2 210251B73D17" && jtag_device_ctx=="jsn-JTAG-SMT2-210251B73D17-4ba00477-0"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Digilent JTAG-SMT2 210251B73D17" && level==0 && jtag_device_ctx=="jsn-JTAG-SMT2-210251B73D17-23731093-0"}
fpga -file C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/hw/design_1_wrapper.bit
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Digilent JTAG-SMT2 210251B73D17" && jtag_device_ctx=="jsn-JTAG-SMT2-210251B73D17-4ba00477-0"}
loadhw -hw C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/hw/design_1_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Digilent JTAG-SMT2 210251B73D17" && jtag_device_ctx=="jsn-JTAG-SMT2-210251B73D17-4ba00477-0"}
source C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/hw/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0" && jtag_cable_name =~ "Digilent JTAG-SMT2 210251B73D17" && jtag_device_ctx=="jsn-JTAG-SMT2-210251B73D17-4ba00477-0"}
dow C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU0/Debug/CPU0.elf
targets -set -nocase -filter {name =~ "*A9*#1" && jtag_cable_name =~ "Digilent JTAG-SMT2 210251B73D17" && jtag_device_ctx=="jsn-JTAG-SMT2-210251B73D17-4ba00477-0"}
dow C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/CPU1/Debug/CPU1.elf
configparams force-mem-access 0
bpadd -addr &main
