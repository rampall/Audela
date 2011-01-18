$Id: pkgIndex.tcl,v 1.2 2011-01-18 20:07:25 jacquesmichelet Exp $

proc load_tkhtml { dir } {
    if { $::tcl_platform(os) == "Linux" } {
        if { $::tcl_platform(pointerSize) == 4 } {
            # Mode 32 bits
            return [ load [ file join $dir Tkhtml3_32[info sharedlibextension] ] ]
        } else {
            # Mode 64 bits
            return [ load [ file join $dir Tkhtml3_64[info sharedlibextension] ] ]
        }
    } else {
        return [ load [ file join $dir Tkhtml3[info sharedlibextension] ] ]
    }
}

package ifneeded Tkhtml 3.0 [ load_tkhtml $dir ]
