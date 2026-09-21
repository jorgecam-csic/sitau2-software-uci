# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\Platform\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\Platform\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {Platform}\
-hw {C:\PV\UCI_SITAU2_enclustra_repository\UCI_SITAU2.sdk\design_1_wrapper.xsa}\
-proc {ps7_cortexa9_0} -os {standalone} -out {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk}

platform write
platform generate -domains 
platform active {Platform}
platform generate
platform -make-local
domain create -name {ps7_cortex_a0_1} -os {standalone} -proc {ps7_cortexa9_1} -arch {32-bit} -display-name {ps7_cortex_a0_1} -desc {} -runtime {cpp}
platform generate -domains 
platform write
domain -report -json
domain active {standalone_domain}
bsp reload
bsp reload
bsp setlib -name lwip211 -ver 1.8
bsp setlib -name xilffs -ver 4.8
bsp setlib -name xilflash -ver 4.9
bsp setlib -name xilrsa -ver 1.6
bsp setlib -name xilskey -ver 7.3
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain,ps7_cortex_a0_1 
platform clean
platform generate
platform clean
platform generate
platform active {Platform}
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/HW_uci/hw/design_1_wrapper.xsa}
platform clean
platform clean
platform generate
platform generate -domains 
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/HW_uci/hw/design_1_wrapper.xsa}
platform clean
platform clean
platform clean
platform generate
platform clean
platform generate
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/HW_uci/hw/design_1_wrapper.xsa}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform clean
platform clean
platform clean
platform clean
platform generate
catch {platform remove HW_uci}
platform generate -domains 
platform -make-local
platform generate -domains 
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate
platform clean
platform clean
platform generate
platform generate -domains 
platform clean
platform generate
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform clean
platform clean
platform clean
platform generate
platform generate
platform generate
platform generate
platform generate
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate
platform generate -domains 
platform generate
platform generate
platform clean
platform generate
platform clean
platform generate
platform generate -domains 
platform generate -domains 
platform active {Platform}
bsp reload
platform generate -domains 
platform generate
platform generate -domains 
platform generate -domains 
platform generate
platform generate -domains 
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate
platform generate -domains 
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform clean
platform clean
platform clean
platform generate
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
bsp reload
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform clean
platform clean
platform clean
platform generate
platform clean
platform generate
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform clean
platform clean
platform generate
platform clean
platform generate
platform generate -domains 
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
domain active {zynq_fsbl}
bsp reload
platform generate -domains 
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository_prg_lento/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform clean
platform generate
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository_prg_lento/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository_prg_lento/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository_prg_lento/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository_prg_lento/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform generate -domains 
platform generate
platform generate
platform generate -domains 
platform generate
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository_prg_256/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform clean
platform generate
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform generate
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate
platform clean
platform generate
platform clean
platform generate
platform generate -domains standalone_domain,ps7_cortex_a0_1,zynq_fsbl 
platform generate -domains 
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate
domain active {ps7_cortex_a0_1}
bsp reload
platform generate -domains 
platform clean
platform clean
platform generate
domain active {standalone_domain}
bsp reload
bsp setlib -name xiltimer -ver 1.1
bsp write
bsp reload
catch {bsp regenerate}
bsp write
domain active {ps7_cortex_a0_1}
catch {bsp regenerate}
platform generate -domains standalone_domain,ps7_cortex_a0_1 
domain active {zynq_fsbl}
bsp reload
platform -make-local
domain active {ps7_cortex_a0_1}
platform generate -domains 
domain active {standalone_domain}
bsp reload
bsp reload
domain active {zynq_fsbl}
bsp reload
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/resources/design_1_wrapper.xsa}
domain active {zynq_fsbl}
domain active {standalone_domain}
bsp reload
bsp reload
platform generate -domains 
bsp setlib -name libmetal -ver 2.4
bsp write
bsp reload
catch {bsp regenerate}
bsp reload
platform generate -domains standalone_domain 
bsp removelib -name libmetal
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform generate -domains 
platform clean
platform generate
bsp reload
bsp removelib -name xiltimer
bsp write
bsp reload
catch {bsp regenerate}
bsp reload
platform generate -domains standalone_domain 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate -domains 
platform generate
platform generate -domains 
platform clean
platform active {Platform}
platform config -updatehw {C:/PV/UCI_SITAU2_enclustra_repository/UCI_SITAU2.sdk/Platform/export/design_1_wrapper.xsa}
platform clean
platform generate
platform generate -domains 
platform generate -domains 
platform generate
platform generate -domains 
platform generate
platform generate
platform generate -domains 
