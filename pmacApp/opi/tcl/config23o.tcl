# config23ID.tcl

#***********************************************************
  set Title    "23-ID-out"
  set Facility "23ID-out"
  set XX        23
  set ID       "${XX}us"

# set ComponentList { $name $cmp $units{$name $asy $calib $dpram $axes $drives $motors} }
# set ComponentList { $name $cmp $units{$name $asy $calib $pmac $dpram $axes $drives $motors{$mtrNo $mtr} } }
  set ComponentList {
        { "White Beam Slits" WS:
            {
                { "Vertical   Aperture" Av:  HSAp pmac10: 08XY3  {ZC: ZS:} {ZT: ZB:} {{ 1 mt: } { 2 mb: }}   }
                { "Horizontal Aperture" Ah:  HSAp pmac10: 08XY3  {YC: YS:} {YO: YI:} {{ 3 mo: } { 4 mi: }}   }
            }
	}
        { "Monochromator" MO:
            {
                { "Mono Positioning"    Ps:  XYPs pmac10: 08XY3    {}      {Z:  Y: } {{ 5 mz: } { 6 my: }}   }
                { "Mono Energy"         En:  MOEn pmac10: 08XY3  {E: L:}   {QB: T2:} {{ 7 mr: } { 8 mt2:}}   }
                { "Crystal-1 Tuning"    Tn1: MDCs pmac10: 05X3     {}      {R1:    } {{17 mr1:}}             }
                { "Crystal-2 Tuning"    Tn2: XYPs pmac10: 08XY3    {}      {P2: R2:} {{18 mp2:} {19 mr2:}}   }
            }
	}
        { "Beam Position Monitors" BP:
            {
                { "Mono BPM"            Mo:  MDCs pmac10: 05X3     {}      {MOZ:}    {{20 mmo:}}             }
		{ "HDM1 BPM"            Hd1: MDCs pmac10: 05X3     {}      {H1Z:}    {{28 mh1z:}}            }
		{ "HDM1a BPM"           Hd3: MDCs pmac10: 05X3     {}      {H3Z:}    {{ 9 mh3z:}}            }
		{ "HDM2 BPM"            Hd2: MDCs pmac11: 05X3     {}      {H2Z:}    {{ 1 mh2z:}}            }
                { "KB mirrors BPM"      Kb:  MDCs pmac11: 05X3     {}      {KBZ:}    {{16 mkb:}}             }

            }
	}
	{ "Horiz Deflecting Mirrors" HD:
            {
             	{ "HDM Z-pos.         " St:  TBSt pmac10: 11XYZ3 {SL: HT: HR:} {SZ1: SZ2: SZ3:} {{21 mz1: } {22 mz2: } {23 mz3:}}    }
		{ "HDM1 (US) Y-pos.   " Up:  MISt pmac10: 08XY3  {UY: UA:}     {UY1: UY2:}      {{24 muy1:} {25 muy2:}}             }
		{ "HDM2 (DS) Y-pos.   " Dn:  MISt pmac10: 08XY3  {DY: DA:}     {DY1: DY2:}      {{26 mdy1:} {27 mdy2:}}            }
            }
	}
        { "Monochromatic Beam Slits" CS:
            {
                { "Vertical   Aperture" Av:  CSAp pmac11: 08XY3  {ZC: ZS:} {ZT: ZB:} {{2 mz1:} {3 mz2:}}   }
                { "Horizontal Aperture" Ah:  CSAp pmac11: 08XY3  {YC: YS:} {YO: YI:} {{4 my1:} {5 my2:}}   }
            }
	}
        { "KB Mirrors" KB:
            {
                { "Horiz.Mirror Z-pos." Hz:  TBSt pmac11: 11XYZ3 {HL: HW: HR:} {HZ1: HZ2: HZ3:} {{6 mhz1:} {7 mhz2:} {8 mhz3:}}  }
                { "Horiz.Mirror Y-pos." Hy:  MISt pmac11: 08XY3  {HY: HA:}     {HY1: HY2:}      {{9 mhy1:} {10 mhy2:}}             }
                { "Vert. Mirror Z-pos." Vz:  TBSt pmac11: 11XYZ3 {VZ: VA: VR:} {VZ1: VZ2: VZ3:} {{11 mvz1:} {12 mvz2:} {13 mvz3:}}  }
                { "Vert. Mirror Y-pos." Vy:  MISt pmac11: 08XY3  {VL: VW:}     {VY1: VY2:}      {{14 mvy1:} {15 mvy2:}}             }
            }
 	}
        { "Beam Delivery" BD:
            {
                { "Horizontal delivery" Dh:  BDSt  pmac20: 08XY3   {HD: HA:}   {HDS: HUS: }     {{15 mhd:}  {16 mhu:}}              }
                { "Vertical delivery"   Dv:  BDSt  pmac20: 08XY3   {VD: VA:}   {VDS: VUS: }     {{17 mvd:}  {18 mvu:}}              }
            }
 	}
        { "Guard Slits" GS:
            {
                { "Guard Slits Support" St:  XYZPs pmac20: 11XYZ3     {}       {YW: X:  PT:}    {{19 myw:}  {20 mx: }  {21 mpt:}}   }
                { "Vertical aperture"   Av:  HSAp  pmac20: 08XY3   {ZC: ZS:}   {ZT: ZB:}        {{22 mt: }  {23 mb: }}              }
                { "Horizontal aperture" Ah:  HSAp  pmac20: 08XY3   {YC: YS:}   {YO: YI:}        {{24 mo: }  {25 mi: }}              }
            }
 	}
        { "Shutter"     SH:
            {
                { "Shutter Position"    Ps:  SHPs  pmac20: 05X3       {}       {P:}             {{31 mp:}}                          }
            }
 	}
        { "Video cameras" TV:
            {
                { "HiRes.Camera"        V1:  XPs   pmac20: 05X3       {}       {ZM1:}           {{29 mz1:}}                         }
                { "LoRes.Camera"        V2:  XPs   pmac20: 05X3       {}       {ZM2:}           {{30 mz2:}}                         }
            }
 	}
 	{ "Collimator" COL:
 	    {
 		{ "Support"             St:  XYPs  pmac21: 08XY3      {}       {H:   V: }       {{ 11 mh: }  { 12 mv: }}              }
 	    }
 	}
        { "Beam Stop"   BS:
            {
                { "Beam Stop Position"  Ps:  XYZPs pmac20: 11XYZ3     {}       {D:  PT: YW:}    {{26 md:}   {27 mpt:}  {28 myw:}}   }
            }
 	}
 	{ "Pin Diode" PIN:
 	    {
 		{ "Support"             St:  XPs   pmac21: 05X3       {}       {P: }            {{10 mp: }}                         }
 	    }
 	}
        { "Goniometer" GO:
            {
                { "Goniometer Omega"    Om:  XPs   pmac20: 05X3       {}       {OM:}            {{5  mom:}}                         }
                { "Goniometer Kappa"    Kp:  XYZPs pmac20: 11XYZ3     {}       {KP: PH:  CZ: }  {{6  mkp:}  {7  mph:}  {8  mcz:}}   }
                { "Crystal XY Head"     Cr:  XYPs  pmac20: 08XY3      {}       {CX: CY:}        {{9  mcx:}  {10 mcy:}}              }
                { "Goniometer support"  St:  XYZPs pmac20: 11XYZ3     {}       {SH: SW:  SV:}   {{11 msh:}  {12 msw:}  {13 msv:}}   }
            }
 	}
        { "Cryo Jet" JET:
            {
                { "Cryo Jet position"   Ps:  XPs   pmac20: 05X3       {}       {P: }            {{14 mp:}}                          }
            }
 	}
        { "Fluorescent Detector" FLU:
            {
                { "Flu.Detector dist."  Di:  XPs   pmac21: 05X3       {}       {D: }            {{9  md:}}                          }
            }
 	}
        { "Robot" RB:
            {
                { "Robot XYZ"           Rd:  XYZPs pmac21: 11XYZ3     {}       {RX: RY:  RZ: }  {{5  mrx:}  {6  mry:}  {7  mrz:}}   }
                { "Robot Phy"           Rr:  XPs   pmac21: 05X3       {}       {RA:}            {{8  mra:}}                         }
            }
 	}
        { "CCD Support" CCD:
            {
                { "CCD Lateral pos."    Lp:  CCDLp pmac20: 05X3       {}       {DL: }           {{1  ml:}}                          }
                { "CCD Vert.Support"    St:  CCDSt pmac20: 11XYZ3  {D: 2Q:}    {X: VUS: VDS:}   {{2  mx:}   {3  mus:}  {4  mds:}}   }
            }
 	}
 	{ "PMAC-21 XY Positioners " XY:
 	    {
 		{ "Positioner-2 (Pico)"           P2:  XYPs  pmac21: 08XY3      {}       {X2:  Y2: }      {{15 mx2: } {16 my2: }}             }
 		{ "Positioner-3 (ThinGap)"        P3:  XYPs  pmac21: 08XY3      {}       {X3:  Y3: }      {{17 mx3: } {18 my3: }}             }
 		{ "Positioner-6 (Newport)"        P6:  XYPs  pmac21: 08XY3      {}       {X6:  Y6: }      {{23 mx6: } {24 my6: }}             }
 	    }
 	}
 	{ "Huber Slits" HS:
 	    {
                { "Vertical   Aperture" Av:  HSAp  pmac21: 08XY3   {ZC: ZS:}   {ZT: ZB:  }      {{1 mt:  } {2 mb:  }}             }
                { "Horizontal Aperture" Ah:  HSAp  pmac21: 08XY3   {YC: YS:}   {YO: YI:  }      {{3 mo:  } {4 mi:  }}             }
 	    }
 	}
  }


  set IocPmacList {
    {
	iocgmca1: {
	    {
                pmac10:
                { { 1 WS:Av: }
                  { 2 WS:Ah: }
                  { 3 MO:Ps: }
                  { 4 MO:En: }
                  { 5 MO:Tn1:}
                  { 6 MO:Tn2:}
                  { 7 BP:Mo: }
                  { 8 HD:St: }
                  { 9 HD:Up: }
                  {10 HD:Dn: }
                  {11 BP:Hd1:}
                  {12 BP:Hd3:}
                }
	    }
	    {
	        pmac11:
		{ {1 BP:Hd2:}
                  {2 CS:Av: }
                  {3 CS:Ah: }
                  {4 KB:Hz: }
                  {5 KB:Hy: }
                  {6 KB:Vz: }
                  {7 KB:Vy: }
                  {8 BP:Kb: }
                }
	    }
	}
    }
    {
 	iocgmca2: {
 	    {
 		pmac20:
 		{ { 1 CCD:Lp:}
                  { 2 CCD:St:}
                  { 3 GO:Om: }
                  { 4 GO:Kp: }
                  { 5 GO:Cr: }
                  { 6 GO:St: }
                  { 7 JET:Ps:}
                  { 8 BD:Dh: }
                  { 9 BD:Dv: }
                  {10 GS:St: }
                  {11 GS:Av: }
                  {12 GS:Ah: }
                  {13 BS:Ps: }
                  {14 TV:V1: }
                  {15 TV:V2: }
                  {16 SH:Ps: }
                }
 	    }
 	    {
 		pmac21:
 		{ { 1 HS:Av: }
                  { 2 HS:Ah: }
                  { 3 RB:Rd: }
                  { 4 RB:Rr: }
                  { 5 FLU:Di:}
                  { 6 PIN:St:}
		  { 7 COL:St:}
                  { 8 XY:P1: }
                  { 9 XY:P2: }
                  {10 XY:P3: }
                  {11 XY:P4: }
                  {12 XY:P5: }
                  {13 XY:P6: }
                  {14 XY:P7: }
                  {15 XY:P8: }
                  {16 XY:P9: }
                }
 	    }
 	}
    }
  }


## Additional list for brushless motors:
## (Sergey, 2005/03/29)
  set brushlessList { CCD:mx: GO:mom: XY:mx3: XY:my3:}

## Additional list for geodrives:
## The format is: {motor_name pmac_node}
  set geodrivesList [concat {CCD:ml: 24}  {CCD:mx: 25}  {CCD:mus: 28} {CCD:mds: 29} ]

## Additional list for Acc_65E:
  set acc65eList {pmac20: pmac21:}

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
	{ "Horiz Deflecting Mirrors" HD:
            {
             	{ "HDM Z-pos.         " St:  {SL: } }
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
                { "Guard Slits Support" St:  {X:     } }
                { "Vertical aperture"   Av:  {ZC: ZS:} }
                { "Horizontal aperture" Ah:  {YC: YS:} }
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

