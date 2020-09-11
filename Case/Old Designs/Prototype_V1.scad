//everything is in units of millimeters
resolution = 0.35;

mainDepth = 80.15;
mainWidth = 140;
mainHeight = 42;
mainShellThickness = 4.9;

displayWidth = 70.7;
displayDepth = mainShellThickness;
displayHeight = 20.65;
displayScrewHoleYoffset = 2.1;
displayScrewBottomHoleZoffset = 2.45;
knobHoleOffset = (mainWidth - displayWidth) / 4;
knobHoleRadius = 3.17;
knobHoleDepth = mainShellThickness;
knobScrewHoleYoffset = 11.1125;
knobScrewBottomHoleZoffset = 5.556;
knobScrewTopHoleZoffset = 7.45;
knobScrewHoleRadius = 1.58;
airSlotDepth = mainDepth * 0.3;
airSlotWidth = mainShellThickness;
airSlotHeight = mainHeight * 0.03;
usbSlotDepth = 10;
usbSlotWidth = mainShellThickness;
usbSlotHeight = 8;
mountingHoleRadius = 2.78;

difference() {
  //main shell
  cube([mainDepth, mainWidth, mainHeight]);
  translate([mainShellThickness/2,mainShellThickness/2,mainShellThickness/2]) {//cut-out inside
    cube([mainDepth - mainShellThickness, mainWidth - mainShellThickness, mainHeight]);
  };
  
  //display
  translate([mainDepth - displayDepth/2, mainWidth/2 - displayWidth/2, mainHeight/2 - displayHeight/2]) {
    cube([displayDepth,displayWidth,displayHeight]);
  };
  
  //display top-left hole
  translate([mainDepth - displayDepth/2, mainWidth/2 - displayWidth/2 - displayScrewHoleYoffset, mainHeight/2 + displayHeight/2 + displayScrewBottomHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=60);
    };
  };
  
  //display bottom-left hole
  translate([mainDepth - displayDepth/2, mainWidth/2 - displayWidth/2 - displayScrewHoleYoffset, mainHeight/2 - displayHeight/2 - displayScrewBottomHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=60);
    };
  };
  
  //display top-right hole
  translate([mainDepth - displayDepth/2, mainWidth/2 + displayWidth/2 + displayScrewHoleYoffset, mainHeight/2 + displayHeight/2 + displayScrewBottomHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=60);
    };
  };
  
  //display bottom-right hole
  translate([mainDepth - displayDepth/2, mainWidth/2 + displayWidth/2 + displayScrewHoleYoffset, mainHeight/2 - displayHeight/2 - displayScrewBottomHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=60);
    };
  };
  
  //left knob
  translate([mainDepth - knobHoleDepth/2, knobHoleOffset, mainHeight/2]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobHoleRadius,knobHoleRadius,$fn=60);
    };
  };
  
  //left knob bottom screw hole
  translate([mainDepth - knobHoleDepth/2, knobHoleOffset + knobScrewHoleYoffset, mainHeight/2 - knobScrewBottomHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=60);
    };
  };
  
  //left knob top screw hole
  translate([mainDepth - knobHoleDepth/2, knobHoleOffset + knobScrewHoleYoffset, mainHeight/2 + knobScrewTopHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=60);
    };
  };
  
  //right knob
  translate([mainDepth - knobHoleDepth/2, mainWidth - knobHoleOffset, mainHeight/2]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobHoleRadius,knobHoleRadius,$fn=60);
    };
  };
  
  //right knob bottom screw hole
  translate([mainDepth - knobHoleDepth/2, mainWidth - knobHoleOffset + knobScrewHoleYoffset, mainHeight/2 - knobScrewBottomHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=60);
    };
  };
  
  //right knob top screw hole
  translate([mainDepth - knobHoleDepth/2, mainWidth - knobHoleOffset + knobScrewHoleYoffset, mainHeight/2 + knobScrewTopHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobScrewHoleRadius,knobScrewHoleRadius,$fn=60);
    };
  };
  
  //air slot left
  translate([mainDepth/2 - airSlotDepth/2, 0, mainHeight/2 - airSlotHeight/2]) {
    cube([airSlotDepth, airSlotWidth, airSlotHeight]);
  };
  
  //air slot right
  translate([mainDepth/2 - airSlotDepth/2, mainWidth - airSlotWidth, mainHeight/2 - airSlotHeight/2]) {
    cube([airSlotDepth, airSlotWidth, airSlotHeight]);
  };
  
  //usb cable slot right side
  translate([mainDepth/2 - usbSlotDepth/2 - 1.6, mainWidth - usbSlotWidth, 15]) {
    cube([usbSlotDepth, usbSlotWidth, usbSlotHeight]);
  };
  
  //bottom mounting hole
  translate([mainDepth/2, mainWidth/2, 0]) {
    cylinder(mainShellThickness, mountingHoleRadius, mountingHoleRadius,$fn=60);
  };
  
  //back mounting hole
  translate([0, mainWidth/2, mainHeight/2]) {
    rotate([0,90,0]) {
      cylinder(mainShellThickness, mountingHoleRadius, mountingHoleRadius,$fn=60);
    };
  };
};

  
