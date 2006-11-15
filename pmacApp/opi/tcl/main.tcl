# main.tcl

wm title . $Title
if { $subTitle == "(staff)" } {
  wm geometry .  +20+0
} else {
  wm geometry .  +0+0
}
# . config -bg Black

#frame .fLOGO
#label .lLOGO -image [image create photo -file $Logo]
#pack .lLOGO -in .fLOGO   -side top
#pack .fLOGO -side top

frame .fTitle
label .lTitle -text $Title    -fg White  -bg SteelBlue -font -adobe-times-bold-r-normal--24-240-75-75-p-132-iso8859-1
if { $subTitle == "(staff)" } {
  label .sTitle -text $subTitle -fg Green  -bg SteelBlue -font -adobe-times-bold-r-normal--18-180-75-75-p-99-iso8859-1
} else {
  label .sTitle -text $subTitle -fg Yellow -bg SteelBlue -font -adobe-times-bold-r-normal--14-140-75-75-p-77-iso8859-1
}
pack .lTitle -in .fTitle  -side top -fill x
pack .sTitle -in .fTitle  -side top -fill x
pack .fTitle -side top -pady 1m -fill x

#***********************************************************
#  proc menuMain

proc menuMain {name icon} {
global icondir sp
frame .f$name
label .l$name  -text $name -fg White -font -adobe-times-bold-r-normal--14-100-100-100-p-76-iso8859-1
menubutton .mb$name -relief raised -bitmap @${icondir}${icon} -fg PowderBlue -bg SteelBlue -menu .mb$name.m
pack .mb$name .l$name -in .f$name -side top -padx 10
pack .f$name -side top -pady 1m
}

#***********************************************************
#  proc menuMedm

proc menuMedm {widget label adl {macro {}} } {
global medm varyFont errorFile sp
if { $macro == {} } {
        ${widget} add command -label "${label}" \
                -command "exec $medm -attach $varyFont -x ${adl} $errorFile & "
} else {
        ${widget} add command -label "${label}" \
                -command "exec $medm -attach $varyFont -macro \"${macro}\" -x ${adl} $errorFile & "
}
}

#***********************************************************
#  proc menuCascade

proc menuCascade {widget submenu label} {
${widget} add cascade -label $label -menu ${widget}.${submenu}
}

#***********************************************************
#  proc menuSeparator

proc menuSeparator {widget} {
${widget} add separator
}

