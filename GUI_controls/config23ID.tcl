# config23o.tcl

#***********************************************************
  set Title    "23ID"
  set subTitle "(staff)"

  set Facility "23ID"
  set Beamline 23o
  set bln      ${Beamline}:

# set ComponentList { $name $cmp $units{$name $asy $calib dpram $axes $drives $motors} }
# set ComponentList { $name $cmp $units{$name $asy $calib $pmac $dpram $axes $drives $motors{$mtrNo $mtr} } }
  set ComponentList {
        { "White Beam Slits" WS:
            {
                { "Vertical   Aperture" Av:  HSAp pmac10: 08XY2  {ZC: ZS:} {ZT: ZB:} {{ 1 mt: } { 2 mb: }}   }
                { "Horizontal Aperture" Ah:  HSAp pmac10: 08XY2  {YC: YS:} {YO: YI:} {{ 3 mo: } { 4 mi: }}   }
            }
	}
        { "Monochromator" MO:
            {
                { "Mono Positioning"    Ps:  XYPs pmac10: 08XY2    {}      {Z:  Y: } {{ 5 mz: } { 6 my: }}   }
                { "Mono Energy"         En:  MOEn pmac10: 08XY2  {E: L:}   {QB: T2:} {{ 7 mr: } { 8 mt2:}}   }
                { "Crystal-1 Tuning"    Tn1: MDCs pmac10: 06X2     {}      {R1:    } {{ 9 mr1:}}             }
                { "Crystal-2 Tuning"    Tn2: XYPs pmac10: 08XY2    {}      {P2: R2:} {{10 mp2:} {11 mr2:}}   }
            }
	}
        { "Beam Position Monitors" BP:
            {
                { "Mono BMP"            Mo:  MDCs pmac10: 06X2     {}      {MOZ:}    {{12 mmo:}}             }
                { "KB mirrors BMP"      Kb:  MDCs pmac10: 06X2     {}      {KBZ:}    {{31 mkb:}}             }
            }
	}
        { "Monochromatic Beam Slits" CS:
            {
                { "Vertical   Aperture" Av:  CSAp pmac10: 08XY2  {ZC: ZS:} {ZT: ZB:} {{17 mz1:} {18 mz2:}}   }
                { "Horizontal Aperture" Ah:  CSAp pmac10: 08XY2  {YC: YS:} {YO: YI:} {{19 my1:} {20 my2:}}   }
            }
	}
        { "KB Mirrors" KB:
            {
                { "Horiz.Mirror Z-pos." Hz:  TBSt pmac10: 10XYZ2 {HL: HW: HR:} {HZ1: HZ2: HZ3:} {{21 mhz1:} {22 mhz2:} {23 mhz3:}}  }
                { "Horiz.Mirror Y-pos." Hy:  MISt pmac10: 08XY2  {HY: HA:}     {HY1: HY2:}      {{24 mhy1:} {25 mhy2:}}             }
                { "Vert. Mirror Z-pos." Vz:  TBSt pmac10: 10XYZ2 {VZ: VA: VR:} {VZ1: VZ2: VZ3:} {{26 mvz1:} {27 mvz2:} {28 mvz3:}}  }
                { "Vert. Mirror Y-pos." Vy:  MISt pmac10: 08XY2  {VL: VW:}     {VY1: VY2:}      {{29 mvy1:} {30 mvy2:}}             }
            }
 	}
   	{ "XZ Positioners PMAC-2" XZ:
 	    {
 		{ "Positioner-1"        P1:  XYPs pmac20: 08XY2       {}       {X1:  Y1: }      {{ 1 mx1: } { 2 my1: }}             }
 		{ "Positioner-2"        P2:  XYPs pmac20: 08XY2       {}       {X2:  Y2: }      {{ 3 mx2: } { 4 my2: }}             }
 		{ "Positioner-3"        P3:  XYPs pmac20: 08XY2       {}       {X3:  Y3: }      {{ 5 mx3: } { 6 my3: }}             }
 		{ "Positioner-4"        P4:  XYPs pmac20: 08XY2       {}       {X4:  Y4: }      {{ 7 mx4: } { 8 my4: }}             }
 		{ "Positioner-5"        P5:  XYPs pmac20: 08XY2       {}       {X5:  Y5: }      {{ 9 mx5: } {10 my5: }}             }
 		{ "Positioner-6"        P6:  XYPs pmac20: 08XY2       {}       {X6:  Y6: }      {{11 mx6: } {12 my6: }}             }
 		{ "Positioner-7"        P7:  XYPs pmac20: 08XY2       {}       {X7:  Y7: }      {{13 mx7: } {14 my7: }}             }
 		{ "Positioner-8"        P8:  XYPs pmac20: 08XY2       {}       {X8:  Y8: }      {{15 mx8: } {16 my8: }}             }
 		{ "Positioner-9"        P9:  XYPs pmac20: 08XY2       {}       {X9:  Y9: }      {{17 mx9: } {18 my9: }}             }
 		{ "Positioner-10"       P10: XYPs pmac20: 08XY2       {}       {X10: Y10:}      {{19 mx10:} {20 my10:}}             }
 		{ "Positioner-11"       P11: XYPs pmac20: 08XY2       {}       {X11: Y11:}      {{21 mx11:} {22 my11:}}             }
 		{ "Positioner-12"       P12: XYPs pmac20: 08XY2       {}       {X12: Y12:}      {{23 mx12:} {24 my12:}}             }
 		{ "Positioner-13"       P13: XYPs pmac20: 08XY2       {}       {X13: Y13:}      {{25 mx13:} {26 my13:}}             }
 		{ "Positioner-14"       P14: XYPs pmac20: 08XY2       {}       {X14: Y14:}      {{27 mx14:} {28 my14:}}             }
 		{ "Positioner-15"       P15: XYPs pmac20: 08XY2       {}       {X15: Y15:}      {{29 mx15:} {30 my15:}}             }
 		{ "Positioner-16"       P16: XYPs pmac20: 08XY2       {}       {X16: Y16:}      {{31 mx16:} {32 my16:}}             }
 	    }
 	}
}

  set IocPmacList {
    {
	iocgmca1: {
	    {
                pmac10:
                { {1 WS:Av:}  {2 WS:Ah:}  {3 MO:Ps:}  {4 MO:En:}  {5 MO:Tn1:}  {6 MO:Tn2:}  {7 BP:Mo:}  {8 CS:Av:}  {9 CS:Ah:}  {10 KB:Hz:}  {11 KB:Hy:}  {12 KB:Vz:}  {13 KB:Vy:}  {14 BP:Kb:} }
                { {1 WS:mt:}  {2 WS:mb:}  {3 WS:mo:}  {4 WS:mi:}  {5 MO:mz:}   {6 MO:my:}   {7 MO:mr:}  {8 MO:mt2:} {9 MO:mr1:} {10 MO:mp2:} {11 MO:mr2:} {12 BP:mmo:} {17 CS:mz1:} {18 CS:mz2:} {19 CS:my1:} {20 CS:my2:} {21 KB:mhz1:} {22 KB:mhz2:} {23 KB:mhz3:} {24 KB:mhy1:} {25 KB:mhy2:} {26 KB:mvz1:} {27 KB:mvz2:} {28 KB:mvz3:} {29 KB:mvy1:} {30 KB:mvy2:} {31 BP:mkb:} }
	    }
	}
    }
    {
 	iocgmca2: {
 	    {
 		pmac20:
 		{ {1 XZ:P1:}  {2 XZ:P2:}  {3 XZ:P3:}  {4 XZ:P4:}  {5 XZ:P5:}  {6 XZ:P6:}  {7 XZ:P7:}  {8 XZ:P8:}  {9 XZ:P9:}  {10 XZ:P10:} {11 XZ:P11:} {12 XZ:P12:} {13 XZ:P13:} {14 XZ:P14:} {15 XZ:P15:} {16 XZ:P16:} }
 		{ {1 XZ:mx1:} {2 XZ:my1:} {3 XZ:mx2:} {4 XZ:my2:} {5 XZ:mx3:} {6 XZ:my3:} {7 XZ:mx4:} {8 XZ:my4:} {9 XZ:my5:} {10 XZ:my5:} {11 XZ:mx6:} {12 XZ:my6:} {13 XZ:mx7:} {14 XZ:my7:} {15 XZ:mx8:} {16 XZ:my8:} {17 XZ:mx9:} {18 XZ:my9:} {19 XZ:mx10:} {20 XZ:my10:} {21 XZ:mx11:} {22 XZ:my11:} {23 XZ:mx12:} {24 XZ:my12:} {25 XZ:mx13:} {26 XZ:my13:} {27 XZ:mx14:} {28 XZ:my14:} {29 XZ:mx15:} {30 XZ:my15:} {31 XZ:mx16:} {32 XZ:my16:} }
 	    }
 	}
    }
  }

