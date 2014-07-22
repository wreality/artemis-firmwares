;Simple AHK script to genereate button clicks for screen elements that are missing key board mappings
; IMPORTANT: The click positions should be edited for the given console.  They are likely not correct. (Read: guaranteed)

;CAM
F11::
MouseGetPos, xpos, ypos
SetDefaultMouseSpeed 0
Click 158, 570
MouseMove xpos, ypos
return

;Default View
^,::
MouseGetPos, xpos, ypos
SetDefaultMouseSpeed 0
Click 158, 570
MouseMove xpos, ypos
return

; LRS View
^.::
MouseGetPos, xpos, ypos
SetDefaultMouseSpeed 0
Click 158, 570
MouseMove xpos, ypos
return

; Visualization
^/::
MouseGetPos, xpos, ypos
SetDefaultMouseSpeed 0
Click 159, 695
MouseMove xpos, ypos
return
