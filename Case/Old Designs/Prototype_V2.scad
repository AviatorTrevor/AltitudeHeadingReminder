//everything is in units of millimeters
resolution = 0.35;

mainDepth = 90;
mainWidth = 140;
mainHeight = 42;
mainShellThickness = 5 * resolution;

displayWidth = 71.9;
displayDepth = mainShellThickness;
displayHeight = 24.65;
displaySupportColumnWidth = resolution * 2;
displayScrewHoleYoffset = 2.1;
displayScrewBottomHoleZoffset = 2.45;
displayScrewHoleCylinderDepth = 7.5;
knobHoleYoffset = (mainWidth - displayWidth) / 4;
knobHoleZoffset = (mainHeight / 2);
knobHoleRadius = 3.6;
knobHoleDepth = mainShellThickness;
knobScrewHoleYoffset = 9;
knobScrewBottomHoleZoffset = 6;
knobScrewTopHoleZoffset = 11;
knobScrewHoleRadius = 1.6;
knobSupportWallThickness = resolution * 2;
knobSupportWidth = 12 + 2 * knobSupportWallThickness;
knobSupportHeight = 12.3 + 2 * knobSupportWallThickness;
knobSupportDepth = 2;
airSlotDepth = resolution * 3;
airSlotWidth = mainShellThickness;
airSlotHeight = mainHeight * 0.5;
usbSlotDepth = 10.5;
usbSlotWidth = mainShellThickness;
usbSlotHeight = 8.8;
mountingHoleRadius = 2.78;
cylinderFragments = 40;
breadboardDepth = 57.8;
breadboardPlatformHeight = 6;
breadboardPlatformBeamWidth = 3;
breadboardFenceHeight = breadboardPlatformHeight + 3;
breadboardFenceThickness = resolution * 2;

union() {
  difference() {
    //main shell
    cube([mainDepth, mainWidth, mainHeight]);
    translate([mainShellThickness, mainShellThickness, mainShellThickness]) {//cut-out inside
      cube([mainDepth - mainShellThickness * 2, mainWidth - mainShellThickness * 2, mainHeight]);
    };
    
    //display
    translate([mainDepth - displayDepth, mainWidth/2 - displayWidth/2, mainHeight/2 - displayHeight/2]) {
      cube([displayDepth,displayWidth,displayHeight]);
    };
    
    //display top-left hole
    /*translate([mainDepth - displayDepth, mainWidth/2 - displayWidth/2 - displayScrewHoleYoffset, mainHeight/2 + displayHeight/2 + displayScrewBottomHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
      };
    };*/
    
    //display bottom-left hole
    /*translate([mainDepth - displayDepth, mainWidth/2 - displayWidth/2 - displayScrewHoleYoffset, mainHeight/2 - displayHeight/2 - displayScrewBottomHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
      };
    };*/
    
    //display top-right hole
    /*translate([mainDepth - displayDepth, mainWidth/2 + displayWidth/2 + displayScrewHoleYoffset, mainHeight/2 + displayHeight/2 + displayScrewBottomHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
      };
    };*/
    
    //display bottom-right hole
    /*translate([mainDepth - displayDepth, mainWidth/2 + displayWidth/2 + displayScrewHoleYoffset, mainHeight/2 - displayHeight/2 - displayScrewBottomHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
      };
    };*/
    
    //left knob
    translate([mainDepth - knobHoleDepth, knobHoleYoffset, knobHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobHoleRadius,knobHoleRadius,$fn=cylinderFragments);
      };
    };
    
    //left knob bottom screw hole
    /*translate([mainDepth - knobHoleDepth, knobHoleYoffset + knobScrewHoleYoffset, knobHoleZoffset - knobScrewBottomHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
      };
    };*/
    
    //left knob top screw hole
    /*translate([mainDepth - knobHoleDepth, knobHoleYoffset + knobScrewHoleYoffset, knobHoleZoffset + knobScrewTopHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
      };
    };*/
    
    //right knob
    translate([mainDepth - knobHoleDepth, mainWidth - knobHoleYoffset, knobHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobHoleRadius,knobHoleRadius,$fn=cylinderFragments);
      };
    };
    
    //right knob bottom screw hole
    /*translate([mainDepth - knobHoleDepth, mainWidth - knobHoleYoffset + knobScrewHoleYoffset, knobHoleZoffset - knobScrewBottomHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
      };
    };*/
    
    //right knob top screw hole
    /*translate([mainDepth - knobHoleDepth, mainWidth - knobHoleYoffset + knobScrewHoleYoffset, knobHoleZoffset + knobScrewTopHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
      };
    };*/
    
    //air slot left
    translate([mainDepth/5 - airSlotDepth/2, 0, mainHeight/2 - airSlotHeight/2]) {
      cube([airSlotDepth, airSlotWidth, airSlotHeight]);
    };
    
    //air slot right
    translate([mainDepth/5 - airSlotDepth/2, mainWidth - airSlotWidth, mainHeight/2 - airSlotHeight/2]) {
      cube([airSlotDepth, airSlotWidth, airSlotHeight]);
    };
    
    //usb cable slot right side
    translate([breadboardDepth/2 - usbSlotDepth/2 - 1.6, mainWidth - usbSlotWidth, breadboardPlatformHeight + 4]) {
      cube([usbSlotDepth, usbSlotWidth, usbSlotHeight]);
    };
    
    //bottom mounting hole
    translate([mainDepth/2, mainWidth/2, 0]) {
      cylinder(mainShellThickness, mountingHoleRadius, mountingHoleRadius,$fn=cylinderFragments);
    };
    
    //back mounting hole
    translate([0, mainWidth/2, mainHeight/3 * 2]) {
      rotate([0,90,0]) {
        cylinder(mainShellThickness, mountingHoleRadius, mountingHoleRadius,$fn=cylinderFragments);
      };
    };
  };
  
  //Display support column #1
  translate([mainDepth - displayDepth, mainWidth/2 - displayWidth/2 + displayWidth/6, mainHeight/2 - displayHeight/2]) {
      cube([displayDepth,displaySupportColumnWidth,displayHeight]);
  };
  
  //Display support column #2
  translate([mainDepth - displayDepth, mainWidth/2 - displayWidth/2 + displayWidth/6 * 2, mainHeight/2 - displayHeight/2]) {
      cube([displayDepth,displaySupportColumnWidth,displayHeight]);
  };
  
  //Display support column #3 (middle column)
  translate([mainDepth - displayDepth, mainWidth/2, mainHeight/2 - displayHeight/2]) {
      cube([displayDepth,displaySupportColumnWidth,displayHeight]);
  };
  
  //Display support column #4
  translate([mainDepth - displayDepth, mainWidth/2 + displayWidth/6, mainHeight/2 - displayHeight/2]) {
      cube([displayDepth,displaySupportColumnWidth,displayHeight]);
  };
  
  //Display support column #5
  translate([mainDepth - displayDepth, mainWidth/2 + displayWidth/6 * 2, mainHeight/2 - displayHeight/2]) {
      cube([displayDepth,displaySupportColumnWidth,displayHeight]);
  };
  
  //Left Knob Support Structure
  difference() {
    //cube of support structure
    translate([mainDepth - mainShellThickness - knobSupportDepth, knobHoleYoffset - knobSupportWidth / 2, knobHoleZoffset - knobSupportHeight / 2]) {
      cube([knobSupportDepth, knobSupportWidth, knobSupportHeight]);
    }
    
    //hollowing out the center
    translate([mainDepth - mainShellThickness - knobSupportDepth, knobHoleYoffset - knobSupportWidth / 2 + knobSupportWallThickness, knobHoleZoffset - knobSupportHeight / 2 + knobSupportWallThickness]) {
      cube([knobSupportDepth, knobSupportWidth - knobSupportWallThickness * 2, knobSupportHeight - knobSupportWallThickness * 2]);
    }
  }
  
  //Right Knob Support Structure
  difference() {
    //cube of support structure
    translate([mainDepth - mainShellThickness - knobSupportDepth, mainWidth - knobHoleYoffset - knobSupportWidth / 2, knobHoleZoffset - knobSupportHeight / 2]) {
      cube([knobSupportDepth, knobSupportWidth, knobSupportHeight]);
    }
    
    //hollowing out the center
    translate([mainDepth - mainShellThickness - knobSupportDepth, mainWidth - knobHoleYoffset - knobSupportWidth / 2 + knobSupportWallThickness, knobHoleZoffset - knobSupportHeight / 2 + knobSupportWallThickness]) {
      cube([knobSupportDepth, knobSupportWidth - knobSupportWallThickness * 2, knobSupportHeight - knobSupportWallThickness * 2]);
    }
  }
  
  //display top-left hole support
  translate([mainDepth - mainShellThickness - displayScrewHoleCylinderDepth, mainWidth/2 - displayWidth/2 - displayScrewHoleYoffset, mainHeight/2 + displayHeight/2 + displayScrewBottomHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(displayScrewHoleCylinderDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
    };
  };
  
  //display bottom-left hole support
  translate([mainDepth - mainShellThickness - displayScrewHoleCylinderDepth, mainWidth/2 - displayWidth/2 - displayScrewHoleYoffset, mainHeight/2 - displayHeight/2 - displayScrewBottomHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(displayScrewHoleCylinderDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
    };
  };
  
  //display top-right hole support
  translate([mainDepth - mainShellThickness - displayScrewHoleCylinderDepth, mainWidth/2 + displayWidth/2 + displayScrewHoleYoffset, mainHeight/2 + displayHeight/2 + displayScrewBottomHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(displayScrewHoleCylinderDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
    };
  };
  
  //display bottom-right hole support
  translate([mainDepth - mainShellThickness - displayScrewHoleCylinderDepth, mainWidth/2 + displayWidth/2 + displayScrewHoleYoffset, mainHeight/2 - displayHeight/2 - displayScrewBottomHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(displayScrewHoleCylinderDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=cylinderFragments);
    };
  };
  

  //breadboard platform #1 (furthest left)
  translate([mainShellThickness, mainWidth/8 - breadboardPlatformBeamWidth/2, mainShellThickness]) {
    cube([breadboardDepth, breadboardPlatformBeamWidth, breadboardPlatformHeight]);
  };
  
  //breadboard platform #2
  translate([mainShellThickness, mainWidth/8 * 3 - breadboardPlatformBeamWidth/2, mainShellThickness]) {
    cube([breadboardDepth, breadboardPlatformBeamWidth, breadboardPlatformHeight]);
  };
  
  //breadboard platform #3
  translate([mainShellThickness, mainWidth/8 * 5 - breadboardPlatformBeamWidth/2, mainShellThickness]) {
    cube([breadboardDepth, breadboardPlatformBeamWidth, breadboardPlatformHeight]);
  };
  
  //breadboard platform #4
  translate([mainShellThickness, mainWidth/8 * 7 - breadboardPlatformBeamWidth/2, mainShellThickness]) {
    cube([breadboardDepth, breadboardPlatformBeamWidth, breadboardPlatformHeight]);
  };
  
  breadboardFenceHeight = breadboardPlatformHeight + 2;
breadboardFenceThickness = resolution * 2;
  //breadboard fence
  translate([mainShellThickness + breadboardDepth, mainShellThickness, mainShellThickness]) {
    cube([breadboardFenceThickness, mainWidth - mainShellThickness*2, breadboardFenceHeight]);
  }
 
};
  
