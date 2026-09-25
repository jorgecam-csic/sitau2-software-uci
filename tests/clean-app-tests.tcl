# Prueba aislada de la limpieza de objetos, sin crear un workspace Xilinx.
set repo [file normalize [file join [file dirname [info script]] ..]]
set f [open [file join $repo scripts create-workspace.tcl] r]
set recipe [read $f]; close $f
set command {}; set found 0
foreach line [split $recipe \n] {
    append command $line \n
    if {[info complete $command]} {
        if {[regexp {^\s*proc clean_app_objects } $command]} {
            eval $command
            set found 1
            break
        }
        set command {}
    }
}
if {!$found} {error "No se encontro clean_app_objects"}
set parent [file normalize $env(TEMP)]
set root [file join $parent sitau-clean-test-[pid]-[clock clicks]]
file mkdir [file join $root src nested] [file join $root _sdk bsp]
try {
    foreach name {top.o src/a.o src/a.d src/nested/b.o src/keep.c makefile _sdk/bsp/library.o} {
        set f [open [file join $root $name] w]; puts $f fixture; close $f
    }
    clean_app_objects $root
    foreach name {top.o src/a.o src/a.d src/nested/b.o} {
        if {[file exists [file join $root $name]]} {error "Objeto no retirado: $name"}
    }
    foreach name {src/keep.c makefile _sdk/bsp/library.o} {
        if {![file exists [file join $root $name]]} {error "Archivo ajeno a la limpieza retirado: $name"}
    }
    clean_app_objects [file join $root absent]
    puts CLEAN_APP_TESTS_OK
} finally {
    if {[file dirname [file normalize $root]] ne $parent || ![string match sitau-clean-test-* [file tail $root]]} {error "Ruta temporal inesperada"}
    file delete -force $root
}
