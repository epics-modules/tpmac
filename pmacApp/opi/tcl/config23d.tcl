# config23d.tcl

#***********************************************************
  set Title    "23-devel"
  set Facility "23-devel"
  set XX        23
  set ID        "none"

# set ComponentList { $name $cmp $units{$name $asy $calib $dpram $axes $drives $motors} }
# set ComponentList { $name $cmp $units{$name $asy $calib $pmac $dpram $axes $drives $motors{$mtrNo $mtr} } }
  set ComponentList {
        { "CCD Support" CCD:
            {
                { "CCD Lateral pos."    LpB: XPs      pmac20: 05X3       {}       {DL: }           {{13  ml:}}                        }
                { "CCD Vert.Support"    StB: CCDSt_BM pmac20: 11XYZ3  {D: 2Q:}    {X: VUS: VDS:}   {{14  mx:}  {15  mus:}  {16  mds:}}}
            }
 	}
        { "Goniometer" GO:
            {
                { "Goniometer Omega"    Om:  XPs   pmac20: 05X3       {}       {OM:}            {{5  mom:}}                         }
                { "Goniometer Kappa"    Kp:  XYZPs pmac20: 11XYZ3     {}       {KP: PH:  CZ: }  {{6  mkp:}  {7  mph:}  {8  mcz:}}   }
                { "Crystal XY Head"     Cr:  XYPs  pmac20: 08XY3      {}       {CX: CY:}        {{9  mcx:}  {10 mcy:}}              }
                { "Goniometer support"  StB: XYPs  pmac20: 08XY3      {}       {SH: SV:}        {{11 msh:}  {12 msv:}}              }
            }
 	}
        { "Cryo Jet" JET:
            {
                { "Cryo Jet position"   Ps:  XPs   pmac20: 05X3       {}       {P: }            {{1  mp:}}                          }
            }
 	}
        { "Beam Delivery" BD:
            {
                { "Horizontal delivery" DhB: XPs   pmac20: 05X3       {}       {HD: }           {{2 mhd:}}              }
                { "Vertical delivery"   Dv:  BDSt  pmac20: 08XY3   {VD: VA:}   {VDS: VUS: }     {{3 mvd:}  {4 mvu:}}              }
	    }
 	}
        { "Guard Slits" GS:
            {
                { "Guard Slits Support" StB: XYPs  pmac20: 08XY3     {}        {X:  PT:}        {{17 mx: }  {18 mpt:}}              }
                { "Vertical aperture"   Av:  HSAp  pmac20: 08XY3   {ZC: ZS:}   {ZT: ZB:}        {{19 mt: }  {20 mb: }}              }
                { "Horizontal aperture" Ah:  HSAp  pmac20: 08XY3   {YC: YS:}   {YO: YI:}        {{21 mo: }  {22 mi: }}              }
            }
 	}
        { "Beam Stop"   BS:
            {
                { "Beam Stop Position"  Ps:  XYZPs pmac20: 11XYZ3     {}       {D:  PT: YW:}    {{23 md:}   {24 mpt:}  {25 myw:}}   }
            }
 	}
        { "Video cameras" TV:
            {
                { "HiRes.Camera"        V1:  XPs   pmac20: 05X3       {}       {ZM1:}           {{29 mz1:}}                         }
                { "LoRes.Camera"        V2:  XPs   pmac20: 05X3       {}       {ZM2:}           {{30 mz2:}}                         }
            }
 	}
        { "Shutter"     SH:
            {
                { "Shutter Position"    Ps:  SHPs  pmac20: 05X3       {}       {P:}             {{31 mp:}}                          }
            }
 	}
 	{ "XY Positioners PMAC-21" XY:
 	    {
 		{ "Positioner-1"        P1:  XYPs pmac21: 08XY3       {}       {X1:  Y1: }      {{ 1 mx1: } { 2 my1: }}             }
 		{ "Positioner-2"        P2:  XYPs pmac21: 08XY3       {}       {X2:  Y2: }      {{ 3 mx2: } { 4 my2: }}             }
 		{ "Positioner-3"        P3:  XYPs pmac21: 08XY3       {}       {X3:  Y3: }      {{ 5 mx3: } { 6 my3: }}             }
 		{ "Positioner-4"        P4:  XYPs pmac21: 08XY3       {}       {X4:  Y4: }      {{ 7 mx4: } { 8 my4: }}             }
 		{ "Positioner-5"        P5:  XYPs pmac21: 08XY3       {}       {X5:  Y5: }      {{ 9 mx5: } {10 my5: }}             }
 		{ "Positioner-6"        P6:  XYPs pmac21: 08XY3       {}       {X6:  Y6: }      {{11 mx6: } {12 my6: }}             }
 		{ "Positioner-7"        P7:  XYPs pmac21: 08XY3       {}       {X7:  Y7: }      {{13 mx7: } {14 my7: }}             }
 		{ "Positioner-8"        P8:  XYPs pmac21: 08XY3       {}       {X8:  Y8: }      {{15 mx8: } {16 my8: }}             }
 		{ "Positioner-9"        P9:  XYPs pmac21: 08XY3       {}       {X9:  Y9: }      {{17 mx9: } {18 my9: }}             }
 		{ "Positioner-10"       P10: XYPs pmac21: 08XY3       {}       {X10: Y10:}      {{19 mx10:} {20 my10:}}             }
 		{ "Positioner-11"       P11: XYPs pmac21: 08XY3       {}       {X11: Y11:}      {{21 mx11:} {22 my11:}}             }
 		{ "Positioner-12"       P12: XYPs pmac21: 08XY3       {}       {X12: Y12:}      {{23 mx12:} {24 my12:}}             }
 		{ "Positioner-13"       P13: XYPs pmac21: 08XY3       {}       {X13: Y13:}      {{25 mx13:} {26 my13:}}             }
 		{ "Positioner-14"       P14: XYPs pmac21: 08XY3       {}       {X14: Y14:}      {{27 mx14:} {28 my14:}}             }
 		{ "Positioner-15"       P15: XYPs pmac21: 08XY3       {}       {X15: Y15:}      {{29 mx15:} {30 my15:}}             }
 		{ "Positioner-16"       P16: XYPs pmac21: 08XY3       {}       {X16: Y16:}      {{31 mx16:} {32 my16:}}             }
 	    }
 	}
  }


  set IocPmacList {
    {
	iocgmca1: {
	    {
                pmac10:
                { { 1 WS:Av:}
                  { 2 WS:Ah:}
                  { 3 MO:Ps:}
                  { 4 MO:En:}
                  { 5 MO:Tn1:}
                  { 6 MO:Tn2:}
                  { 7 BP:Mo:}
                  { 8 HD:St:}
                  { 9 HD:Up:}
                  {10 HD:Dn:}
                  {11 BP:Hd1:}
		}
	    }
	}
    }
    {
 	iocgmca2: {
            {
 		pmac20:
                { { 1 CCD:LpB:}
                  { 2 CCD:StB:}
                  { 3 GO:Om:  }
                  { 4 GO:Kp:  }
                  { 5 GO:Cr:  }
                  { 6 GO:StB: }
                  { 7 JET:Ps: }
                  { 8 BD:DhB: }
                  { 9 BD:Dv:  }
                  {10 GS:StB: }
                  {11 GS:Av:  }
                  {12 GS:Ah:  }
                  {13 BS:Ps:  }
                  {14 TV:V1:  }
                  {15 TV:V2:  }
 		  {16 SH:Ps:  }
                }
 	    }
 	    {
 		pmac21:
 		{ { 1 XY:P1:}
                  { 2 XY:P2:}
                  { 3 XY:P3:}
                  { 4 XY:P4:}
                  { 5 XY:P5:}
                  { 6 XY:P6:}
                  { 7 XY:P7:}
                  { 8 XY:P8:}
                  { 9 XY:P9:}
                  {10 XY:P10:}
                  {11 XY:P11:}
                  {12 XY:P12:}
                  {13 XY:P13:}
                  {14 XY:P14:}
                  {15 XY:P15:}
                  {16 XY:P16:}
                }
 	    }
 	}
    }
  }

## Additional list for brushless motors:
## (Sergey, 2005/03/29)
  set brushlessList { XY:mx9: XY:my9:}

## Additional list for geodrives:
## The format is: {motor_name pmac_node}
  set geodrivesList [ ]

## Additional list for Acc_65E:
  set acc65eList {pmac21:}

## Additional list for Acc_59E:
  set acc59eList {pmac20: pmac21:}

## Additional settings for the shutter program executed by gonio during frame collection:
## (see 2_m3300-endstation-pmacXX_BLN.pmc)
  set shutterOpenedMvar M3425
  set shutterClosedMvar M3426


####################################################################################
# For users only:
  set ComponentUsers {
        { "Monochromator" MO:
            {
                { "Mono Energy"         En:  {E:  L: } }
                { "Crystal-2 Tuning"    Tn2: {P2:    } }
            }
	}
        { "KB Mirrors" KB:
            {
                { "Horiz.Mirror Z-pos." Hz:  {HL: } }
                { "Vert. Mirror Y-pos." Vy:  {VL: } }
            }
 	}
        { "Beam Delivery" BD:
            {
                { "Horizontal delivery" Dh:  {HD: } }
                { "Vertical delivery"   Dv:  {VD: } }
            }
 	}
        { "Guard Slits" GS:
            {
                { "Guard Slits Support" St:  {X:      } }
                { "Vertical aperture"   Av:  {ZC: ZS: } }
                { "Horizontal aperture" Ah:  {YC: YS: } }
            }
 	}
        { "Shutter"     SH:
            {
                { "Shutter Position"    Ps:  {P: } }
            }
 	}
        { "Video cameras" TV:
            {
                { "HiRes.Camera"        V1:  {ZM1: } }
                { "LoRes.Camera"        V2:  {ZM2: } }
            }
 	}
 	{ "Collimator" COL:
 	    {
 		{ "Collimator Position" St:  {H: V:} }
 	    }
 	}
        { "Beam Stop"   BS:
            {
                { "Beam Stop Position"  Ps:  {D: PT: YW:} }
            }
 	}
 	{ "Pin Diode" PIN:
 	    {
 		{ "Support"             St:  {P: } }
 	    }
 	}
        { "Goniometer" GO:
            {
                { "Goniometer Omega"    Om:  {OM:}     }
                { "Crystal XY Head"     Cr:  {CX: CY:} }
                { "Goniometer support"  St:  {SH: SV:} }
            }
 	}
        { "Cryo Jet" JET:
            {
                { "Cryo Jet position"   Ps:  {P: } }
            }
 	}
        { "Fluorescent Detector" FLU:
            {
                { "Flu.Detector dist."  Di:  {D: } }
            }
 	}
        { "CCD Support" CCD:
            {
                { "CCD Lateral pos."    Lp:  {DL:   } }
                { "CCD Vert.Support"    St:  {D: 2Q:} }
            }
 	}
  }

