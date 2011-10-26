# config23b.tcl

#***********************************************************
  set Title    "23-bm"
  set Facility "23-bm"
  set XX        23
  set ID        "none"

# set ComponentList { $name $cmp $units{$name $asy $calib $dpram $axes $drives $motors} }
# set ComponentList { $name $cmp $units{$name $asy $calib $pmac $dpram $axes $drives $motors{$mtrNo $mtr} } }
#             { "Mono BPM"            Mo:  MDCs pmac11: 05X3     {}      {MOZ:}    {{ 3 mmo:}}             }
  set ComponentList {
        { "White Beam Slits" WS:
            {
                { "Vertical   Aperture" Av:  HSAp pmac10: 08XY3  {ZC: ZS:} {ZT: ZB:} {{ 1 mt: } { 2 mb: }}   }
                { "Horizontal Aperture" Ah:  HSAp pmac10: 08XY3  {YC: YS:} {YO: YI:} {{ 3 mo: } { 4 mi: }}   }
            }
	}
	{ "Vert Collimating Mirror" VC:
            {
             	{ "VCM Z-pos.         " Vz:  TBSt pmac10: 11XYZ3 {VZ: VA: VR:} {Z1: Z2: Z3:} {{ 5 mz1: } { 6 mz2: } { 7 mz3:}}    }
		{ "VCM Y-pos.         " Vy:  MISt pmac10: 08XY3  {VL: VW:}     {Y1: Y2:}     {{ 8 my1: } { 9 my2:}}               }
		{ "VCM Bender         " Be:  MDCs pmac10: 05X3   {}            {BE:}         {{10 mbe:}}                          }
            }
	}

        { "Monochromator" MO:
            {
                { "Mono Positioning"    Ps:  XPs_BM   pmac10: 08XY3    {}        {Z:}          {{11 mz: }}                       }
                { "Mono Energy"         En:  MOEn_BM  pmac10: 11XYZ3   {E: L:}   {QB: T2: T1:} {{12 mr: } {13 mt2:} {14 mt1:}}   }
                { "Crystal-1 Tuning"    Tn1: MDCs     pmac10: 05X3     {}        {R1:    }     {{17 mr1:}}                       }
                { "Crystal-2 Tuning"    Tn2: XYZPs    pmac10: 11XY3    {}        {P2: R2: W2:} {{18 mp2:} {19 mr2:} {20 mw2:}}   }
		{ "Crystal-2 Bender"    Be:  MISt     pmac10: 08XY3    {BE: BT:} {B1: B2:}     {{21 mb1:} {22 mb2:}}             }
            }
	}
        { "Beam Position Monitors" BP:
            {
  		{ "VFM BPM "            Vf:  MDCs pmac11: 05X3     {}      {VFZ:}    {{14 mvf:}}             }
            }
	}
        { "White Beam Attenuator" AT:
            {
                { "Vertical Positioning" Av: XYPs pmac11: 08XY3    {}      {Z1: Z2:} {{1 mz1:} {2 mz2:}}   }
            }
	}
        { "Monochromatic Beam Slits" CS:
            {
                { "Vertical   Aperture" Av:  CSAp pmac11: 08XY3  {ZC: ZS:} {ZT: ZB:} {{4 mz1:} {5 mz2:}}   }
                { "Horizontal Aperture" Ah:  CSAp pmac11: 08XY3  {YC: YS:} {YO: YI:} {{6 my1:} {7 my2:}}   }
            }
	}
        { "Vert Focusing Mirror" VF:
            {
                { "VFM Z-positioning"          Vz:  TBSt pmac11: 11XYZ3 {VZ: VA: VR:} {Z1: Z2: Z3:}  {{ 8 mz1:} { 9 mz2:} {10 mz3:}}}
                { "VFM Y-positioning"          Vy:  MISt pmac11: 08XY3  {VL: VW:}     {Y1: Y2:}      {{11 my1:} {12 my2:}}          }
                { "VFM Bender"                 Be:  MDCs pmac11: 05X3   {}            {BE:}          {{13 mbe:}}                    }
            }
 	}
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
 	{ "Collimator" COL:
 	    {
 		{ "Support"             St:  XYPs  pmac21: 08XY3      {}       {H:  V: }        {{11  mh: }  {12  mv: }}            }
 	    }
 	}
        { "Robot:" RB:
            {
                { "Robot XYZ"           Rd:  XYZPs pmac21: 11XYZ3     {}       {RX: RY:  RZ: }  {{5  mrx:}  {6  mry:}  {7  mrz:}}   }
                { "Robot Phy"           Rr:  XPs   pmac21: 05X3       {}       {RA:}            {{8  mra:}}                         }
            }
 	}
        { "Fluorescent Detector" FLU:
            {
                { "Flu.Detector dist."  Di:  XPs   pmac21: 05X3       {}       {D: }            {{9  md:}}                          }
            }
 	}
 	{ "Pin Diode" PIN:
 	    {
 		{ "Support"             St:  XPs   pmac21: 05X3       {}       {P: }            {{10 mp: }}                         }
 	    }
 	}
 	{ "Huber Slits PMAC-2" HS:
 	    {
                { "Vertical   Aperture" Av:  HSAp  pmac21: 08XY3   {ZC: ZS:}   {ZT: ZB:  }      {{1 mt:  }  {2 mb:  }}  	    }
                { "Horizontal Aperture" Ah:  HSAp  pmac21: 08XY3   {YC: YS:}   {YO: YI:  }      {{3 mo:  }  {4 mi:  }}  	    }
 	    }
 	}
 	{ "PMAC-21 XY Positioners " XY:
 	    {
 		{ "Positioner-1"        P1:  XYPs  pmac21: 08XY3      {}       {X1:  Y1: }      {{13 mx1: } {14 my1: }}             }
 		{ "Positioner-2"        P2:  XYPs  pmac21: 08XY3      {}       {X2:  Y2: }      {{15 mx2: } {16 my2: }}             }
 		{ "Positioner-3"        P3:  XYPs  pmac21: 08XY3      {}       {X3:  Y3: }      {{17 mx3: } {18 my3: }}             }
 		{ "Positioner-4"        P4:  XYPs  pmac21: 08XY3      {}       {X4:  Y4: }      {{19 mx4: } {20 my4: }}             }
 		{ "Positioner-5"        P5:  XYPs  pmac21: 08XY3      {}       {X5:  Y5: }      {{21 mx5: } {22 my5: }}             }
 		{ "Positioner-6"        P6:  XYPs  pmac21: 08XY3      {}       {X6:  Y6: }      {{23 mx6: } {24 my6: }}             }
 		{ "Positioner-7"        P7:  XYPs  pmac21: 08XY3      {}       {X7:  Y7: }      {{25 mx7: } {26 my7: }}             }
 		{ "Positioner-8"        P8:  XYPs  pmac21: 08XY3      {}       {X8:  Y8: }      {{27 mx8: } {28 my8: }}             }
 		{ "Positioner-9"        P9:  XYPs  pmac21: 08XY3      {}       {X9:  Y9: }      {{29 mx9: } {30 my9: }}             }
 	    }
 	}
  }


  set IocPmacList {
    {
	iocgmca1:
	{
	    {
                pmac10:
                { { 1 WS:Av: }
                  { 2 WS:Ah: }
                  { 3 VC:Vz: }
                  { 4 VC:Vy: }
                  { 5 VC:Be: }
                  { 6 MO:Ps: }
                  { 7 MO:En: }
                  { 8 MO:Tn1:}
                  { 9 MO:Tn2:}
                  {10 MO:Be: }
		}
	    }
	    {
	        pmac11:
		{ { 1 AT:Av: }
                  { 2 BP:Mo: }
                  { 3 CS:Av: }
                  { 4 CS:Ah: }
                  { 5 VF:Vz: }
                  { 6 VF:Vy: }
                  { 7 VF:Be: }
                  { 8 BP:Vf: }
		}
	    }
       }
    }
    {
 	iocgmca2:
	{
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
                { { 1 HS:Av:  }
                  { 2 HS:Ah:  }
                  { 3 RB:Rd:  }
                  { 4 RB:Rr:  }
                  { 5 FLU:Di: }
                  { 6 PIN:St: }
 		  { 7 COL:St: }
                  { 8 XY:P1:  }
                  { 9 XY:P2:  }
                  {10 XY:P3:  }
                  {11 XY:P4:  }
                  {12 XY:P5:  }
                  {13 XY:P6:  }
                  {14 XY:P7:  }
                  {15 XY:P8:  }
                  {16 XY:P9:  }
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
# set geodrivesList [concat {CCD:mus: 13} {CCD:mds: 14} {CCD:md: 15} {CCD:ml: 16} ]
  set geodrivesList [concat {CCD:ml: 24} {CCD:md: 25} {CCD:mus: 28} {CCD:mds: 29} ]

## Additional list for Acc_65E:
  set acc65eList {pmac20: pmac21:}

## Additional list for Acc_59E:
  set acc59eList {pmac10: pmac21:}

## Additional settings for the shutter program executed by gonio during frame collection:
## (see 2_m3300-endstation-pmacXX_BLN.pmc)
  set shutterOpenedMvar M3416
  set shutterClosedMvar M3417


####################################################################################
# For users only:
  set ComponentUsers {
        { "Monochromator" MO:
            {
                { "Mono Energy"         En:  {E:  L: } }
                { "Crystal-2 Tuning"    Tn2: {P2:    } }
            }
	}
        { "CCD Support" CCD:
            {
                { "CCD Lateral pos."    LpB: {DL:   } }
                { "CCD Vert.Support"    StB: {D: 2Q:} }
            }
 	}
        { "Goniometer" GO:
            {
                { "Goniometer Omega"    Om:  {OM:}     }
                { "Crystal XY Head"     Cr:  {CX: CY:} }
                { "Goniometer support"  StB: {SH: SV:} }
            }
 	}
        { "Cryo Jet" JET:
            {
                { "Cryo Jet position"   Ps:  {P: } }
            }
 	}
        { "Beam Delivery" BD:
            {
                { "Horizontal delivery" DhB: {HD: }    }
                { "Vertical delivery"   Dv:  {VD: VA:} }
            }
 	}
        { "Guard Slits" GS:
            {
                { "Guard Slits Support" StB: {X:      } }
                { "Vertical aperture"   Av:  {ZC: ZS: } }
                { "Horizontal aperture" Ah:  {YC: YS: } }
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
        { "Video cameras" TV:
            {
                { "HiRes.Camera"        V1:  {ZM1: } }
                { "LoRes.Camera"        V2:  {ZM2: } }
            }
 	}
        { "Shutter"     SH:
            {
                { "Shutter Position"    Ps:  {P: } }
            }
 	}
        { "Fluorescent Detector" FLU:
            {
                { "Flu.Detector dist."  Di:  {D: } }
            }
 	}
 	{ "Pin Diode" PIN:
 	    {
 		{ "Support"             St:  {P: } }
 	    }
 	}
  }

