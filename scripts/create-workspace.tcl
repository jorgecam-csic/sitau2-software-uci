# Receta mantenida para Vitis Classic 2022.2. Invocar mediante setup.ps1.
proc read_text {path} {
    set f [open $path r]; set data [read $f]; close $f; return $data
}
proc write_text {path text} {
    file mkdir [file dirname $path]
    set f [open $path w]; fconfigure $f -encoding utf-8 -translation lf
    puts -nonewline $f $text; close $f
}
proc configure_bsp {domain mss} {
    domain active $domain
    set kind {}; set values [dict create]
    foreach line [split [read_text $mss] \n] {
        if {[regexp {^BEGIN (\w+)} $line -> block]} {set kind $block; set values [dict create]}
        if {[regexp {^\s*PARAMETER (\w+)\s*=\s*(.*)} $line -> key value]} {dict set values $key [string trim $value]}
        if {[string trim $line] eq "END"} {
            if {$kind eq "LIBRARY"} {
                bsp setlib -name [dict get $values LIBRARY_NAME] -ver [dict get $values LIBRARY_VER]
            }
            if {$kind eq "OS" || $kind eq "LIBRARY"} {
                dict for {key value} $values {
                    if {$key ni {OS_NAME OS_VER PROC_INSTANCE LIBRARY_NAME LIBRARY_VER}} {bsp config $key $value}
                }
            }
        }
    }
}
proc copy_tree {source target} {
    file mkdir $target
    foreach p [glob -nocomplain -directory $source *] {
        set dest [file join $target [file tail $p]]
        if {[file isdirectory $p]} {copy_tree $p $dest} else {file copy -force $p $dest}
    }
}
proc main {mode repo workspace xsa vitis_home} {
    setws $workspace
    set software_repo [file join $workspace software-repository]
    if {$mode eq "setup"} {
        set lwip [file join $software_repo sw_services lwip211_v1_08_s]
        copy_tree [file join $vitis_home data embeddedsw ThirdParty sw_services lwip211_v1_8] $lwip
        set mld [file join $lwip data lwip211.mld]
        set metadata [read_text $mld]
        if {[regsub {OPTION VERSION = 1\.8;} $metadata {OPTION VERSION = 1.08.s;} metadata] != 1} {
            error "Metadatos lwIP de origen inesperados: $mld"
        }
        set metadata [string map {{lwip211 library:} {SITAU2 revision 1 (based on Xilinx 1.8):}} $metadata]
        write_text $mld $metadata
        foreach name {xadapter.c xemacpsif_physpeed.c} {
            file copy -force [file join $repo config lwip211 $name] [file join $lwip src contrib ports xilinx netif $name]
        }
        repo -set $software_repo
        platform create -name Platform -hw $xsa -proc ps7_cortexa9_0 -os standalone
        platform active Platform
        domain create -name ps7_cortex_a0_1 -os standalone -proc ps7_cortexa9_1 -runtime cpp
        foreach {name config} {standalone_domain cpu0 ps7_cortex_a0_1 cpu1 zynq_fsbl fsbl} {
            configure_bsp $name [file join $repo config bsp $config.mss]
        }
        platform write
        platform generate
        # Configuracion de aplicaciones exportada como Tcl declarativo.
        source [file join $repo config applications.tcl]
        foreach item $sitau_apps {
            lassign $item name domain system relative includes libraries
            app create -name $name -platform Platform -domain $domain -sysproj $system -template {Empty Application(C)}
            importsources -name $name -path [file join $repo $relative] -soft-link -linker-script
            app config -name $name build-config Debug
            app config -name $name -set compiler-optimization {None (-O0)}
            app config -name $name -set linker-script [file join $repo $relative lscript.ld]
            foreach inc $includes {app config -name $name -add include-path [file join $repo $relative $inc]}
            foreach lib $libraries {app config -name $name -add libraries $lib}
            write_text [file join $workspace $name .settings org.eclipse.core.resources.prefs] "eclipse.preferences.version=1\nencoding/<project>=Cp1252\n"
        }
    } elseif {$mode eq "build"} {
        repo -set $software_repo
        platform active Platform
        # La plataforma se genera durante setup. Una configuracion distinta
        # exige un workspace nuevo; no reconstruir todos los BSP en cada build.
        foreach name {CPU0 CPU1} {
            set output [file join $workspace $name Debug $name.elf]
            # XSCT puede devolver exito aunque falle el builder de Eclipse.
            # Retirar solo esta salida generada obliga a comprobar el enlace.
            file delete -force $output
            puts [app build -name $name]
            if {![file exists $output] || [file size $output] < 4} {error "No se genero $output. Consultar .metadata/.log e IDE.log."}
            set f [open $output rb]; set magic [read $f 4]; close $f
            binary scan $magic H* signature
            if {$signature ne "7f454c46"} {error "Salida ELF no valida: $output"}
        }
        # Los productos de distribucion se generan con package.ps1 y BIF propios.
        # No usar el BOOT automatico de CPU1_system para la carga por red.
    } else {error "Accion desconocida: $mode"}
    puts "SITAU_OK:$mode"
}
if {[catch {main {*}$argv} message options]} {
    puts stderr $message
    puts stderr [dict get $options -errorinfo]
    exit 1
}
exit 0
