
//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file
mainShellThickness = 5 * resolution;
mainDepth = 57;
mainWidth = 91;
mainHeight = 37 + mainShellThickness;
screwCylinderRadius = 2.5;
screwHoleCylinderRadius = 0.8;
screwHoleCylinderOffsetFromFloor = mainHeight / 2;
cylinderFragments = 70;

//keep in-sync with the mounting plate file
mountingHoleRadius = 2.78;

displayWidth = 24;
displayDepth = mainShellThickness;
displayHeight = 7.5;
displayYOffset = 8.6;
displayTopOffset = displayHeight + 6;
knobHoleYoffset = (mainWidth / 2) - displayYOffset - (displayWidth / 2);
knobHoleZoffset = mainHeight - displayTopOffset - 11.8;
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
knobHookYaxisOffset = 6;
knobHookYaxisWidth = 0.7;
knobHookXaxisDepth = mainShellThickness - resolution;
knobHookZaxisHeight = 2.2;
pcbScrewMountHeight = 7.5;
pcbScrewCylinderRadius = screwCylinderRadius;
pcbScrewHoleCylinderRadius = screwHoleCylinderRadius;
pcbBoardThickness = 1.6;
pcbBoardDepth = 36.9;
pcbBoardWidth = 75.3;
pcbOffsetX = screwCylinderRadius * 2 + 1.5; //2.4 is distance from screw hole to edge of PCB board in the rear-right
pcbOffsetY = mainWidth - mainShellThickness - pcbBoardWidth;
pcbScrewRearLeftX = pcbBoardDepth - 33.147 + pcbOffsetX;
pcbScrewRearLeftY = 3.937 + pcbOffsetY;
pcbScrewFrontLeftX = pcbBoardDepth - 6.731 + pcbOffsetX;
pcbScrewFrontLeftY = 3.429 + pcbOffsetY;
pcbScrewFrontRightX = pcbBoardDepth - 3.048 + pcbOffsetX;
pcbScrewFrontRightY = 71.2 + pcbOffsetY;
pcbScrewRearRightX = pcbBoardDepth - 33.274 + pcbOffsetX;
pcbScrewRearRightY = 63.754 + pcbOffsetY;
usbSlotXoffset = pcbOffsetX + 2.2;
usbSlotYoffset = pcbScrewMountHeight - 1.7;
usbSlotWidth = 27;
usbSlotHeight = 7;
//mountingHoleX = mainDepth - mainShellThickness - mountingHoleRadius - 3.3;
mountingHoleX = pcbOffsetX + pcbBoardDepth + mountingHoleRadius + 2.5;


union() {
  difference() {
    //main shell
    cube([mainDepth, mainWidth, mainHeight]);
    translate([mainShellThickness, mainShellThickness, mainShellThickness]) {//cut-out inside
      cube([mainDepth - mainShellThickness * 2, mainWidth - mainShellThickness * 2, mainHeight]);
    };
    
    //left display
    translate([mainDepth - displayDepth, mainWidth/2 - displayWidth - displayYOffset, mainHeight - displayTopOffset]) {
      cube([displayDepth,displayWidth,displayHeight]);
    };
    
    //right display
    translate([mainDepth - displayDepth, mainWidth/2 + displayYOffset, mainHeight - displayTopOffset]) {
      cube([displayDepth,displayWidth,displayHeight]);
    };
    
    //left knob
    translate([mainDepth - knobHoleDepth, knobHoleYoffset, knobHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobHoleRadius,knobHoleRadius,$fn=cylinderFragments);
      };
    };
    //left knob hook slot
    translate([mainDepth - knobHoleDepth, knobHoleYoffset - knobHookYaxisOffset - knobHookYaxisWidth, knobHoleZoffset - knobHookZaxisHeight / 2]) {
      cube([knobHookXaxisDepth, knobHookYaxisWidth, knobHookZaxisHeight]);
    };
    
    //right knob
    translate([mainDepth - knobHoleDepth, mainWidth - knobHoleYoffset, knobHoleZoffset]) {
      rotate([0,90,0]) {
        cylinder(knobHoleDepth,knobHoleRadius,knobHoleRadius,$fn=cylinderFragments);
      };
    };
    //right knob hook slot
    translate([mainDepth - knobHoleDepth, mainWidth - knobHoleYoffset + knobHookYaxisOffset, knobHoleZoffset - knobHookZaxisHeight / 2]) {
      cube([knobHookXaxisDepth, knobHookYaxisWidth, knobHookZaxisHeight]);
    };
    
    //usb cable slot right side
    translate([usbSlotXoffset, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset]) {
      cube([usbSlotWidth, mainShellThickness, usbSlotHeight]);
    };
    
    //bottom mounting hole
    translate([mountingHoleX, mainWidth/2, 0]) {
      cylinder(mainShellThickness, mountingHoleRadius, mountingHoleRadius, $fn=cylinderFragments);
    };
    
    //back mounting hole
    translate([0, mainWidth/2, mainHeight - mountingHoleRadius - 10]) {
      rotate([0,90,0]) {
        cylinder(mainShellThickness, mountingHoleRadius, mountingHoleRadius, $fn=cylinderFragments);
      };
    };
    
    //cut out for case lid screw mounts and rounded edges, rear-left
    translate([0, 0, 0]) {
      cube([screwCylinderRadius, screwCylinderRadius, mainHeight]);
    };
    translate([screwCylinderRadius - screwHoleCylinderRadius, screwCylinderRadius - screwHoleCylinderRadius, mainShellThickness]) {
      cube([screwHoleCylinderRadius*2, screwHoleCylinderRadius*2, mainHeight - mainShellThickness]);
    };
    
    //cut out for case lid screw mounts and rounded edges, front-left
    translate([mainDepth - screwCylinderRadius, 0, 0]) {
      cube([screwCylinderRadius, screwCylinderRadius, mainHeight]);
    };
    translate([mainDepth - screwCylinderRadius - screwHoleCylinderRadius, screwCylinderRadius - screwHoleCylinderRadius, mainShellThickness]) {
      cube([screwHoleCylinderRadius*2, screwHoleCylinderRadius*2, mainHeight - mainShellThickness]);
    };
    
    //cut out for case lid screw mounts and rounded edges, front-right
    translate([mainDepth - screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
      cube([screwCylinderRadius, screwCylinderRadius, mainHeight]);
    };
    translate([mainDepth - screwCylinderRadius - screwHoleCylinderRadius, mainWidth - screwCylinderRadius - screwHoleCylinderRadius, mainShellThickness]) {
      cube([screwHoleCylinderRadius*2, screwHoleCylinderRadius*2, mainHeight - mainShellThickness]);
    };
    
    //cut out for case lid screw mounts and rounded edges, rear-right
    translate([0, mainWidth - screwCylinderRadius, 0]) {
      cube([screwCylinderRadius, screwCylinderRadius, mainHeight]);
    };
    translate([screwCylinderRadius - screwHoleCylinderRadius, mainWidth - screwCylinderRadius - screwHoleCylinderRadius, mainShellThickness]) {
      cube([screwHoleCylinderRadius*2, screwHoleCylinderRadius*2, mainHeight - mainShellThickness]);
    };
  };
  
  //adding cylinder for case lid screw mounts, rear-left
  difference() {
    translate([screwCylinderRadius, screwCylinderRadius, 0]) {
      cylinder(mainHeight, screwCylinderRadius, screwCylinderRadius, $fn=cylinderFragments);
    };
    translate([screwCylinderRadius, screwCylinderRadius, screwHoleCylinderOffsetFromFloor]) {
      cylinder(mainHeight - screwHoleCylinderOffsetFromFloor, screwHoleCylinderRadius, screwHoleCylinderRadius, $fn=cylinderFragments);
    }
  };

  //adding cylinder for case lid screw mounts, front-left
  difference() {
    translate([mainDepth - screwCylinderRadius, screwCylinderRadius, 0]) {
      cylinder(mainHeight, screwCylinderRadius, screwCylinderRadius, $fn=cylinderFragments);
    };
    translate([mainDepth - screwCylinderRadius, screwCylinderRadius, screwHoleCylinderOffsetFromFloor]) {
      cylinder(mainHeight - screwHoleCylinderOffsetFromFloor, screwHoleCylinderRadius, screwHoleCylinderRadius, $fn=cylinderFragments);
    }
  };
  
  //adding cylinder for case lid screw mounts, front-right
  difference() {
    translate([mainDepth - screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
      cylinder(mainHeight, screwCylinderRadius, screwCylinderRadius, $fn=cylinderFragments);
    };
    translate([mainDepth - screwCylinderRadius, mainWidth - screwCylinderRadius, screwHoleCylinderOffsetFromFloor]) {
      cylinder(mainHeight - screwHoleCylinderOffsetFromFloor, screwHoleCylinderRadius, screwHoleCylinderRadius, $fn=cylinderFragments);
    }
  };
  
  //adding cylinder for case lid screw mounts, rear-right
  difference() {
    translate([screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
      cylinder(mainHeight, screwCylinderRadius, screwCylinderRadius, $fn=cylinderFragments);
    };
    translate([screwCylinderRadius, mainWidth - screwCylinderRadius, screwHoleCylinderOffsetFromFloor]) {
      cylinder(mainHeight - screwHoleCylinderOffsetFromFloor, screwHoleCylinderRadius, screwHoleCylinderRadius, $fn=cylinderFragments);
    }
  };
  
  //display "jail bar" support for 3D plastic printing process
  /* TODO: removing this for now since it didn't print on Arianne's 3D printer
  translate([mainDepth - mainShellThickness, mainWidth/2 - 16, mainHeight - displayTopOffset]) {
    cube([mainShellThickness, resolution, displayHeight]);
  };
  translate([mainDepth - mainShellThickness, mainWidth/2 - 24, mainHeight - displayTopOffset]) {
    cube([mainShellThickness, resolution, displayHeight]);
  };
  translate([mainDepth - mainShellThickness, mainWidth/2 + 16, mainHeight - displayTopOffset]) {
    cube([mainShellThickness, resolution, displayHeight]);
  };
  translate([mainDepth - mainShellThickness, mainWidth/2 + 24, mainHeight - displayTopOffset]) {
    cube([mainShellThickness, resolution, displayHeight]);
  };
  //jail-bar over USB slot:
  translate([screwCylinderRadius * 2 + usbSlotWidth/3, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset]) {
    cube([resolution, mainShellThickness, usbSlotHeight]);
  };
  translate([screwCylinderRadius * 2 + usbSlotWidth/3 * 2, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset]) {
    cube([resolution, mainShellThickness, usbSlotHeight]);
  };*/
  
  
  //PCB left-front side screw/mount
  difference() {
    translate([pcbScrewFrontLeftX, pcbScrewFrontLeftY, mainShellThickness]) {
      cylinder(pcbScrewMountHeight, pcbScrewCylinderRadius, pcbScrewCylinderRadius, $fn=cylinderFragments);
    };
    /*TODO removing the cutout:
    translate([pcbScrewFrontLeftX, pcbScrewFrontLeftY, mainShellThickness]) {
      cylinder(pcbScrewMountHeight, pcbScrewHoleCylinderRadius, pcbScrewHoleCylinderRadius, $fn=cylinderFragments);
    };*/
  };
  
  //PCB right-front side screw/mount
  difference() {
    translate([pcbScrewFrontRightX, pcbScrewFrontRightY, mainShellThickness]) {
      cylinder(pcbScrewMountHeight, pcbScrewCylinderRadius, pcbScrewCylinderRadius, $fn=cylinderFragments);
    };
    translate([pcbScrewFrontRightX, pcbScrewFrontRightY, mainShellThickness]) {
      cylinder(pcbScrewMountHeight, pcbScrewHoleCylinderRadius, pcbScrewHoleCylinderRadius, $fn=cylinderFragments);
    };
  };
  
  //PCB right-rear side screw/mount
  difference() {
    translate([pcbScrewRearRightX, pcbScrewRearRightY, mainShellThickness]) {
      cylinder(pcbScrewMountHeight, pcbScrewCylinderRadius, pcbScrewCylinderRadius, $fn=cylinderFragments);
    };
    /*TODO: removing the cutout:
    translate([pcbScrewRearRightX, pcbScrewRearRightY, mainShellThickness]) {
      cylinder(pcbScrewMountHeight, pcbScrewHoleCylinderRadius, pcbScrewHoleCylinderRadius, $fn=cylinderFragments);
    };*/
  };
  
  //PCB left-rear side screw/mount
  difference() {
    translate([pcbScrewRearLeftX, pcbScrewRearLeftY, mainShellThickness]) {
      cylinder(pcbScrewMountHeight, pcbScrewCylinderRadius, pcbScrewCylinderRadius, $fn=cylinderFragments);
    };
    translate([pcbScrewRearLeftX, pcbScrewRearLeftY, mainShellThickness]) {
      cylinder(pcbScrewMountHeight, pcbScrewHoleCylinderRadius, pcbScrewHoleCylinderRadius, $fn=cylinderFragments);
    };
  };
  
  
  /*chunky 1700mAh battery
   *https://amzn.to/38PP5lB
   *volume 15142.4 cubic millimeters*/
  /*translate([mainShellThickness + 1, mainShellThickness + 1, mainShellThickness + 1]) {
    cube([28, 52, 10.4]);
  };*/
  
  /*1000 mAh battery item A
   *https://amzn.to/3iOChR7
   *volume 9360 cubic millimeters*/
  /*translate([mainShellThickness + 5, mainShellThickness + 5, mainShellThickness + 5]) {
    cube([7.8, 40, 30]);
  };*/
  
  /*1000 mAh battery item B
   *https://amzn.to/2WaNbad
   *volume 10000 cubic millimeters
   ***candidate****/
  /*translate([mainShellThickness + 1, mainShellThickness + 3.3, mainShellThickness + 0.5]) {
    cube([20, 50, 10]);
  };*/
  
  /*1000 mAh battery item C
   *https://amzn.to/3ekTApi
   *volume 9016.8 cubic millimeters*/
  /*translate([mainShellThickness + 5, mainShellThickness + 5, mainShellThickness + 5]) {
    cube([5.1, 52, 34]);
  };*/
  
  /*750 mAh battery item D
   *https://amzn.to/2APc0AP
   *volume 8252.4 cubic millimeters*/
  /*translate([mainShellThickness + 5, mainShellThickness + 5, mainShellThickness + 5]) {
    cube([7.8, 46, 23]);
  };*/
  
  /*750 mAh battery flat item E
   *https://amzn.to/3egqKGE
   *volume 7500 cubic millimeters*/
  /*translate([mainShellThickness + 1, mainShellThickness + 5, mainShellThickness + 5]) {
    cube([30, 50, 5]);
  };*/
  
  /*750 mAh battery flat item F
   *https://amzn.to/2C5dhnX
   *volume 9360 cubic millimeters*/
  /*translate([mainShellThickness + 5, mainShellThickness + 5, mainShellThickness + 5]) {
    cube([7.8, 40, 30]);
  };*/
  
  //left rotary knob approximation for installation
  /*translate([mainDepth - mainShellThickness - 33, mainShellThickness + 8, mainShellThickness + 6]) {
    cube([33, 26, 19.5]);
  };*/
  //left rotary knob approximation post installation
  /*translate([mainDepth - mainShellThickness - 15, mainShellThickness + 8, mainShellThickness + 6]) {
    cube([33, 26, 19.5]);
  };*/
  
  //right rotary knob approximation for installation
  /*translate([mainDepth - mainShellThickness - 33, mainShellThickness + 47.5, mainShellThickness + 6]) {
    cube([33, 26, 19.5]);
  };*/
  //right rotary knob approximation post installation
  /*translate([mainDepth - mainShellThickness - 15, mainShellThickness + 47.5, mainShellThickness + 6]) {
    cube([33, 26, 19.5]);
  };*/
};
  
