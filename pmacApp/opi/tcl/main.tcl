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
label .l$name  -text $name -fg SteelBlue -font -adobe-times-bold-r-normal--14-100-100-100-p-76-iso8859-1
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
#  proc menuLabel

proc menuLabel {widget label} {
  set fg [ ${widget} cget -foreground ]
  set bg [ ${widget} cget -background ]
  ${widget} add command -label $label -activeforeground $fg -activebackground $bg -state disabled
}
#proc menuLabel {widget name label} {
#  label ${widget}.l$name -text $label
#  pack ${widget}.l$name -side top -pady 1m
#}

#***********************************************************
#  proc menuSeparator

proc menuSeparator {widget} {
${widget} add separator
}

#***********************************************************
#  proc menuCollimatorPresetsBURT (backup/restore collimator presets)

proc menuCollimatorPresetsBURT {thisMenu} {
  global Beamline burtSave burtRestore snapdir sp burtGMCA SYSTEM

  if { ${SYSTEM} == "LINUX" }  {
    ${thisMenu} add command -label "Backup collimator presets" -command {\
      set dateStamp [clock format [clock seconds] -format {%Y-%m-%d-%H%M}]; \
      set initialFile "${Beamline}_collimator_${dateStamp}.snap"; \
      set snapFile [ \
        tk_getSaveFile -title {Backup Collimator Presets: Specify SNAP File} \
                       -initialdir "${snapdir}${sp}collimator_presets" \
		       -initialfile ${initialFile} \
                       -filetypes {{"SNAP files" {.snap}} {"ALL files" {*}}} \
                       -defaultextension {.snap}]; \
      if {$snapFile != ""} {\
        if {[file exists $snapFile] == 1} { \
          tk_messageBox -message "Overwriting is not allowed!" -type ok -icon error -title "File Exists"; \
        } else { \
          set requestFile "${burtGMCA}request${sp}COL_St_presets.req"; \
          if { [ catch {exec ${burtSave} -f ${requestFile} -o ${snapFile} -DBEAMLINE=${Beamline}} err ] } { \
### Ignore strings like: "error waiting for process to exit: child process lost
### (is SIGCHLD ignored or trapped?)"
            if { [ string match *SIGCHLD* $err ] == 0 } { \
              puts "exec: '$err'"; \
              tk_messageBox -message $err -type ok -icon error -title "BURTsave Application Error"; \
            } \
          } \
        } \
      } \
    }
    ${thisMenu} add command -label "Restore collimator presets" -command {\
      set snapFile [tk_getOpenFile -title {Choose Collimator Presets Snapshot} \
                       -initialdir "${snapdir}${sp}collimator_presets" \
		       -filetypes {{"SNAP files" {.snap}} {"ALL files" {*}}} \
                       -defaultextension {.snap}]; \
      if {$snapFile != ""} {\
        if {[file exists $snapFile] != 1} { \
          tk_messageBox -message "File does not exist!" -type ok -icon error -title "File Exists"; \
        } else { \
          if { [ catch {exec  ${burtRestore} -f ${snapFile}} err ] } { \
### Ignore strings like: "error waiting for process to exit: child process lost
### (is SIGCHLD ignored or trapped?)"
            if { [ string match *SIGCHLD* $err ] == 0 } { \
              puts "exec: '$err'"; \
              tk_messageBox -message $err -type ok -icon error -title "BURTrestore Application Error"; \
            } \
          } \
        } \
      } \
    }
  }
}
