/*TODO:
snaps for lid
snaps for OLEDs
better design for mounting bracket
mounting solution for buzzer
*/


//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file
mainShellThickness = 5 * resolution;
frontFaceThickness = 3 * resolution;
mainDepth = 57;
mainWidth = 91;
mainHeight = 37 + mainShellThickness;
caseCornerRadius = 2.5;
cylinderFragments = 70;

lidSnapJointProtrusionHeight = 2;
lidSnapJointWidth = mainDepth / 2;
lidSnapJointProtrusionLength = mainShellThickness * 3 / 5;
lidSnapJointOffsetFromTop = 1.2;

lidSnapJointGapHeightForRemoval = 1.5;
lidSnapJointGapWidthForRemoval = 10;

//keep in-sync with the mounting plate file
mountingHoleRadius = 4.32;
mountingPillarRadius = mountingHoleRadius + 1.5;
mountingHoleHeight = 8.68;
mountingPillarHeight = mountingHoleHeight + 0.5;

displayEdgeBuffer = 0.8;
displayWidth = 22.5 + displayEdgeBuffer*2;
displayDepth = mainShellThickness;
displayHeight = 6 + displayEdgeBuffer*2;
displayYOffset = 8.6;
displayTopOffset = displayHeight + 6;
displayIndentWidth = 38;
displayIndentHeight = 12;
displayIndentYOffsetFromDisplayY = -6.9 + displayEdgeBuffer;
displayIndentZOffsetFromDisplayZ = -3.9 + displayEdgeBuffer;
knobHoleYoffset = (mainWidth / 2) - displayYOffset - (displayWidth / 2);
knobHoleZoffset = mainHeight - displayTopOffset - 11.8;
knobHoleRadius = 3.6;
knobHoleDepth = mainShellThickness;
knobIndentSquareSideLength = 12;
knobHookYaxisOffset = 6;
knobHookYaxisWidth = 0.7;
knobHookXaxisDepth = mainShellThickness - resolution;
knobHookZaxisHeight = 2.2;
pcbMountSpacingFromWallToCut = resolution;
pcbMountHeight = mountingPillarHeight - mainShellThickness + 2; //height above mainShellThickness
pcbBoardThickness = 1.6;
pcbBoardDepth = 36;
pcbBoardWidth = 74.9;
pcbPadSideLength = 9;
pcbWallThickness = 0.9;
pcbOffsetX = mainShellThickness;
pcbOffsetY = mainWidth - mainShellThickness - pcbBoardWidth;
pcbShelfSideLength = 3;
pcbShelfThickness = 1;
pcbRearRightWidth = 14.5;
pcbRearRightDepth = 3.5;
pcbSnapJointHeadHeight = 4;
pcbSnapJointHeadProtrusionLength = pcbMountSpacingFromWallToCut + 0.8;
pcbSnapJointFlatLipLength = pcbSnapJointHeadHeight / 8;
usbSlotHeight = 7;
usbSlotWidth = 26 - usbSlotHeight;
usbSlotXoffset = pcbOffsetX + 3 + usbSlotHeight / 2;
usbSlotYoffset = pcbMountHeight; //offset above mainShellThickness
mountingHoleX = pcbOffsetX + pcbBoardDepth + pcbWallThickness + mountingHoleRadius - 2;


//MODULE snap joint for lid
module createLeftSideLidSnapJoint()
{
  translate([mainDepth/2 - lidSnapJointWidth/2, mainShellThickness - lidSnapJointProtrusionLength, mainHeight - lidSnapJointOffsetFromTop - lidSnapJointProtrusionHeight]) {
    polyhedron(
      points=[[0,lidSnapJointProtrusionLength,0], //0
              [0,0,lidSnapJointProtrusionHeight], //1
              [lidSnapJointWidth,0,lidSnapJointProtrusionHeight], //2
              [lidSnapJointWidth,lidSnapJointProtrusionLength,0], //3
              [0,lidSnapJointProtrusionLength,lidSnapJointProtrusionHeight], //4
              [lidSnapJointWidth,lidSnapJointProtrusionLength,lidSnapJointProtrusionHeight]], //5
      faces=[[0,1,2,3],
             [1,4,5,2],
             [4,0,3,5],
             [4,1,0],
             [3,2,5]
             ]
    );
  };
  
  //gap for removing lid
  translate([mainDepth/2 - lidSnapJointGapWidthForRemoval/2, 0, mainHeight - lidSnapJointGapHeightForRemoval]) {
    cube([lidSnapJointGapWidthForRemoval, mainShellThickness, lidSnapJointGapHeightForRemoval]);
  };
};


union() {
  difference() {
    //main shell
    cube([mainDepth, mainWidth, mainHeight]);
    //cutting out the inside of the shell
    translate([mainShellThickness, mainShellThickness, mainShellThickness]) {
      cube([mainDepth - mainShellThickness * 2, mainWidth - mainShellThickness * 2, mainHeight]);
    };
    
    //left display
    translate([mainDepth - displayDepth, mainWidth/2 - displayWidth - displayYOffset, mainHeight - displayTopOffset]) {
      cube([displayDepth, displayWidth, displayHeight]);
    };
    //left display indent to make the wall thinner
    translate([mainDepth - displayDepth, mainWidth/2 - displayWidth - displayYOffset + displayIndentYOffsetFromDisplayY, mainHeight - displayTopOffset + displayIndentZOffsetFromDisplayZ]) {
      cube([mainShellThickness - frontFaceThickness, displayIndentWidth, displayIndentHeight]);
    };
    
    //right display
    translate([mainDepth - displayDepth, mainWidth/2 + displayYOffset, mainHeight - displayTopOffset]) {
      cube([displayDepth, displayWidth, displayHeight]);
    };
    //right display indent to make the wall thinner
    translate([mainDepth - displayDepth, mainWidth/2 + displayYOffset + displayIndentYOffsetFromDisplayY, mainHeight - displayTopOffset + displayIndentZOffsetFromDisplayZ]) {
      cube([mainShellThickness - frontFaceThickness, displayIndentWidth, displayIndentHeight]);
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
    //left knob indent
    translate([mainDepth - knobHoleDepth, knobHoleYoffset - knobIndentSquareSideLength/2, knobHoleZoffset - knobIndentSquareSideLength/2]) {
      cube([mainShellThickness - frontFaceThickness, knobIndentSquareSideLength, knobIndentSquareSideLength + 3]);
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
    //right knob indent
    translate([mainDepth - knobHoleDepth, mainWidth - knobHoleYoffset - knobIndentSquareSideLength/2, knobHoleZoffset - knobIndentSquareSideLength/2]) {
      cube([mainShellThickness - frontFaceThickness, knobIndentSquareSideLength, knobIndentSquareSideLength + 3]);
    };
    
    //usb cable and power switch slot right side.
    translate([usbSlotXoffset, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset]) {
      cube([usbSlotWidth, mainShellThickness, usbSlotHeight]);
    };
    //NOTE: Keep in sync way below for cutting out the cylinder pillar
    translate([usbSlotXoffset, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset + usbSlotHeight/2]) {
      rotate([270,90,0]) {
        cylinder(mainShellThickness, usbSlotHeight/2, usbSlotHeight/2, $fn=cylinderFragments);
      };
    };
    //circular edge cut-out
    translate([usbSlotXoffset + usbSlotWidth, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset + usbSlotHeight/2]) {
      rotate([270,90,0]) {
        cylinder(mainShellThickness, usbSlotHeight/2, usbSlotHeight/2, $fn=cylinderFragments);
      };
    };
    
    //bottom mounting hole
    translate([mountingHoleX, mainWidth/2, 0]) {
      cylinder(mainShellThickness, mountingHoleRadius, mountingHoleRadius, $fn=cylinderFragments);
    };
    //back mounting hole
    /*TODO: I've removed for now
    translate([0, mainWidth/2, mainHeight - mountingHoleRadius - 10]) {
      rotate([0,90,0]) {
        cylinder(mainShellThickness, mountingHoleRadius, mountingHoleRadius, $fn=cylinderFragments);
      };
    };*/
    
    //cut out for case lid screw mounts and rounded edges, rear-left
    translate([0, 0, 0]) {
      cube([caseCornerRadius, caseCornerRadius, mainHeight]);
    };
    
    //cut out for case lid screw mounts and rounded edges, front-left
    translate([mainDepth - caseCornerRadius, 0, 0]) {
      cube([caseCornerRadius, caseCornerRadius, mainHeight]);
    };
    
    //cut out for case lid screw mounts and rounded edges, front-right
    translate([mainDepth - caseCornerRadius, mainWidth - caseCornerRadius, 0]) {
      cube([caseCornerRadius, caseCornerRadius, mainHeight]);
    };
    
    //cut out for case lid screw mounts and rounded edges, rear-right
    translate([0, mainWidth - caseCornerRadius, 0]) {
      cube([caseCornerRadius, caseCornerRadius, mainHeight]);
    };
    
    createLeftSideLidSnapJoint();
    
    translate([0,mainWidth,0]) {
      mirror([0,1,0]) {
        createLeftSideLidSnapJoint();
      };
    };
  };
  
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  //outside main "difference" function
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  
  //bottom mounting hole
  difference() {
    translate([mountingHoleX, mainWidth/2, 0]) {
      cylinder(mountingPillarHeight, mountingPillarRadius, mountingPillarRadius, $fn=cylinderFragments);
    };
    translate([mountingHoleX, mainWidth/2, 0]) {
      cylinder(mountingHoleHeight, mountingHoleRadius, mountingHoleRadius, $fn=cylinderFragments);
    };
  };
  
  //adding cylinder for case rounded edge, rear-left
  difference() {
    translate([caseCornerRadius, caseCornerRadius, 0]) {
      cylinder(mainHeight, caseCornerRadius, caseCornerRadius, $fn=cylinderFragments);
    };
    translate([mainShellThickness, mainShellThickness, mainShellThickness]) {
      cube([caseCornerRadius*2 - mainShellThickness, caseCornerRadius*2 - mainShellThickness, mainHeight - mainShellThickness]);
    };
  };

  //adding cylinder for case rounded edge, front-left
  difference() {
    translate([mainDepth - caseCornerRadius, caseCornerRadius, 0]) {
      cylinder(mainHeight, caseCornerRadius, caseCornerRadius, $fn=cylinderFragments);
    };
    translate([mainDepth - caseCornerRadius*2, mainShellThickness, mainShellThickness]) {
      cube([caseCornerRadius*2 - mainShellThickness, caseCornerRadius*2 - mainShellThickness, mainHeight - mainShellThickness]);
    };
  };

  //adding cylinder for case rounded edge, front-right
  difference() {
    translate([mainDepth - caseCornerRadius, mainWidth - caseCornerRadius, 0]) {
      cylinder(mainHeight, caseCornerRadius, caseCornerRadius, $fn=cylinderFragments);
    };
    translate([mainDepth - caseCornerRadius*2, mainWidth - caseCornerRadius*2, mainShellThickness]) {
      cube([caseCornerRadius*2 - mainShellThickness, caseCornerRadius*2 - mainShellThickness, mainHeight - mainShellThickness]);
    };
  };

  //adding cylinder for case rounded edge, rear-right
  difference() {
    translate([caseCornerRadius, mainWidth - caseCornerRadius, 0]) {
      cylinder(mainHeight, caseCornerRadius, caseCornerRadius, $fn=cylinderFragments);
    };
    translate([mainShellThickness, mainWidth - caseCornerRadius*2, mainShellThickness]) {
      cube([caseCornerRadius*2 - mainShellThickness, caseCornerRadius*2 - mainShellThickness, mainHeight - mainShellThickness]);
    };
    
    //NOTE: keep in sync with USB code far above, USB curved cut-out
    translate([usbSlotXoffset, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset + usbSlotHeight/2]) {
      rotate([270,90,0]) {
        cylinder(mainShellThickness, usbSlotHeight/2, usbSlotHeight/2, $fn=cylinderFragments);
      };
    };
  };

  
  //mounting seat for PCB, rear-left side
  difference() {
    translate([pcbOffsetX, pcbOffsetY - pcbWallThickness, mainShellThickness]) {
      cube([pcbPadSideLength - pcbWallThickness, pcbPadSideLength, pcbMountHeight + pcbBoardThickness]);
    };
    translate([pcbOffsetX, pcbOffsetY, mainShellThickness + pcbMountHeight]) {
      cube([pcbPadSideLength - pcbWallThickness, pcbPadSideLength - pcbWallThickness, pcbBoardThickness]);
    };
    //cut a slot so the side has flex
    translate([pcbOffsetX, pcbOffsetY, mainShellThickness]) {
      cube([pcbPadSideLength - pcbWallThickness, pcbMountSpacingFromWallToCut, pcbMountHeight]);
    };
    //cut a slot away from the side wall so it can flex
    translate([mainShellThickness, pcbOffsetY - pcbWallThickness, mainShellThickness]) {
      cube([pcbMountSpacingFromWallToCut, pcbWallThickness, pcbMountHeight + pcbBoardThickness]);
    };
  };
  //snap joint
  translate([pcbOffsetX + pcbMountSpacingFromWallToCut, pcbOffsetY - pcbWallThickness, mainShellThickness + pcbMountHeight + pcbBoardThickness]) {
    polyhedron(
      points=[[0,0,0], //0
              [0, pcbWallThickness + pcbSnapJointHeadProtrusionLength, 0], //1
              [pcbPadSideLength - pcbMountSpacingFromWallToCut - pcbWallThickness, pcbWallThickness + pcbSnapJointHeadProtrusionLength, 0], //2
              [pcbPadSideLength - pcbMountSpacingFromWallToCut - pcbWallThickness, 0, 0], //3
              [0, pcbWallThickness + pcbSnapJointHeadProtrusionLength, pcbSnapJointFlatLipLength], //4
              [pcbPadSideLength - pcbMountSpacingFromWallToCut - pcbWallThickness, pcbWallThickness + pcbSnapJointHeadProtrusionLength, pcbSnapJointFlatLipLength], //5
              [0, pcbWallThickness, pcbSnapJointHeadHeight], //6
              [pcbPadSideLength - pcbMountSpacingFromWallToCut - pcbWallThickness, pcbWallThickness, pcbSnapJointHeadHeight], //7
              [0, 0, pcbSnapJointHeadHeight], //8
              [pcbPadSideLength - pcbMountSpacingFromWallToCut - pcbWallThickness, 0, pcbSnapJointHeadHeight]], //9
      faces=[[0,1,2,3],
             [1,4,5,2],
             [4,6,7,5],
             [6,8,9,7],
             [9,8,0,3],
             [1,0,8,6,4],
             [5,7,9,3,2]]
    );
  };
  
  //mounting seat for PCB, front-left side
  difference() {
    translate([pcbOffsetX + pcbBoardDepth - pcbPadSideLength + pcbWallThickness, pcbOffsetY - pcbWallThickness, mainShellThickness]) {
      cube([pcbPadSideLength, pcbPadSideLength, pcbMountHeight + pcbBoardThickness]);
    };
    translate([pcbOffsetX + pcbBoardDepth - pcbPadSideLength + pcbWallThickness, pcbOffsetY, mainShellThickness + pcbMountHeight]) {
      cube([pcbPadSideLength - pcbWallThickness, pcbPadSideLength - pcbWallThickness, pcbBoardThickness]);
    };
    //cut a slot so the side has flex
    translate([pcbOffsetX + pcbBoardDepth - pcbPadSideLength + pcbWallThickness, pcbOffsetY, mainShellThickness]) {
      cube([pcbPadSideLength, pcbMountSpacingFromWallToCut, pcbMountHeight + pcbBoardThickness]);
    };
    //another cut for flexing consistency
    translate([pcbOffsetX + pcbBoardDepth - pcbMountSpacingFromWallToCut, pcbOffsetY - pcbWallThickness, mainShellThickness]) {
      cube([pcbWallThickness + pcbMountSpacingFromWallToCut, pcbWallThickness, pcbMountHeight + pcbBoardThickness]);
    };
  };
  //snap joint
  translate([pcbOffsetX + pcbBoardDepth - pcbPadSideLength + pcbWallThickness, pcbOffsetY - pcbWallThickness, mainShellThickness + pcbMountHeight + pcbBoardThickness]) {
    polyhedron(
      points=[[0,0,0], //0
              [0, pcbWallThickness + pcbSnapJointHeadProtrusionLength, 0], //1
              [pcbPadSideLength - pcbWallThickness - pcbMountSpacingFromWallToCut, pcbWallThickness + pcbSnapJointHeadProtrusionLength, 0], //2
              [pcbPadSideLength - pcbWallThickness - pcbMountSpacingFromWallToCut, 0, 0], //3
              [0, pcbWallThickness + pcbSnapJointHeadProtrusionLength, pcbSnapJointFlatLipLength], //4
              [pcbPadSideLength - pcbWallThickness - pcbMountSpacingFromWallToCut, pcbWallThickness + pcbSnapJointHeadProtrusionLength, pcbSnapJointFlatLipLength], //5
              [0, pcbWallThickness, pcbSnapJointHeadHeight], //6
              [pcbPadSideLength - pcbWallThickness - pcbMountSpacingFromWallToCut, pcbWallThickness, pcbSnapJointHeadHeight], //7
              [0, 0, pcbSnapJointHeadHeight], //8
              [pcbPadSideLength - pcbWallThickness - pcbMountSpacingFromWallToCut, 0, pcbSnapJointHeadHeight]], //9
      faces=[[0,1,2,3],
             [1,4,5,2],
             [4,6,7,5],
             [6,8,9,7],
             [9,8,0,3],
             [1,0,8,6,4],
             [5,7,9,3,2]]
    );
  };
  
  //mounting seat for PCB, front-right side
  difference() {
    translate([pcbOffsetX + pcbBoardDepth - pcbPadSideLength + pcbWallThickness, mainWidth - pcbPadSideLength, mainShellThickness]) {
      cube([pcbPadSideLength, pcbPadSideLength, pcbMountHeight + pcbBoardThickness]);
    };
    translate([pcbOffsetX + pcbBoardDepth - pcbPadSideLength + pcbWallThickness, mainWidth - pcbPadSideLength, mainShellThickness + pcbMountHeight]) {
      cube([pcbPadSideLength - pcbWallThickness, pcbPadSideLength, pcbBoardThickness]);
    };
  };
  //shelf, front-right
  translate([pcbOffsetX + pcbBoardDepth - pcbShelfSideLength*1.5 + pcbWallThickness, mainWidth - mainShellThickness - pcbShelfSideLength/2, mainShellThickness + pcbMountHeight + pcbBoardThickness]) { //0.5 buffer for the board
    cube([pcbShelfSideLength*1.5, pcbShelfSideLength/2, pcbShelfThickness]);
  };
  //3D print support
  translate([pcbOffsetX + pcbBoardDepth - pcbShelfSideLength*1.5 + pcbWallThickness, mainWidth - mainShellThickness - pcbShelfSideLength/2, mainShellThickness + pcbMountHeight]) { //0.5 buffer for the board
    cube([resolution, resolution, pcbBoardThickness]);
  };

  //mounting seat for PCB, rear-right side
  translate([pcbOffsetX, mainWidth - mainShellThickness - pcbRearRightWidth, mainShellThickness]) {
    cube([pcbRearRightDepth, pcbRearRightWidth, pcbMountHeight]); //measurements taken from PCB board
  };
  //shelf, rear-right
  /*TODO: removed for now
  translate([mainShellThickness, mainWidth - mainShellThickness - pcbShelfSideLength, mainShellThickness + pcbMountHeight + pcbBoardThickness + 0.5]) { //0.5 buffer for the board
    cube([pcbShelfSideLength, pcbShelfSideLength, pcbShelfThickness]);
  };*/
  
  //600mAh battery
  /*translate([mainShellThickness + 2, 25, mainShellThickness]) {
    cube([30, 44, 5]) {
    };
  };*/
  
  createLeftLidSnapJoint3dPrintedSupports(); //left side
  translate([0, mainWidth, 0]) {
    mirror([0,1,0]) {
      createLeftLidSnapJoint3dPrintedSupports(); //right side
    };
  };




  //jail-bars over display slots
  translate([mainDepth - frontFaceThickness, mainWidth/2 - displayYOffset - displayWidth/3, mainHeight - displayTopOffset]) {
    cube([frontFaceThickness, resolution, displayHeight]);
  };
  translate([mainDepth - frontFaceThickness, mainWidth/2 - displayYOffset - displayWidth/3 * 2 - resolution, mainHeight - displayTopOffset]) {
    cube([frontFaceThickness, resolution, displayHeight]);
  };
  translate([mainDepth - frontFaceThickness, mainWidth/2 + displayYOffset + displayWidth/3, mainHeight - displayTopOffset]) {
    cube([frontFaceThickness, resolution, displayHeight]);
  };
  translate([mainDepth - frontFaceThickness, mainWidth/2 + displayYOffset + displayWidth/3 * 2 + resolution, mainHeight - displayTopOffset]) {
    cube([frontFaceThickness, resolution, displayHeight]);
  };
  //jail-bar over USB slot:
  usbJailBarSpacing = 2;
  translate([usbSlotXoffset + usbSlotWidth/4 - usbJailBarSpacing, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset]) {
    cube([resolution, mainShellThickness, usbSlotHeight]);
  };
  translate([usbSlotXoffset + usbSlotWidth/4 * 2, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset]) {
    cube([resolution, mainShellThickness, usbSlotHeight]);
  };
  translate([usbSlotXoffset + usbSlotWidth/4 * 3 + usbJailBarSpacing, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset]) {
    cube([resolution, mainShellThickness, usbSlotHeight]);
  };
};

//3D printing supports for left-side lid snap-fit indent
module createLeftLidSnapJoint3dPrintedSupports() {
  snapJoint3dPrintedSupportRadius = resolution/2;
  translate([mainDepth/2 + lidSnapJointGapWidthForRemoval/2 + (lidSnapJointWidth - lidSnapJointGapWidthForRemoval)/4, mainShellThickness - snapJoint3dPrintedSupportRadius, mainHeight - lidSnapJointOffsetFromTop - lidSnapJointProtrusionHeight]) {
    cylinder(lidSnapJointProtrusionHeight,snapJoint3dPrintedSupportRadius,snapJoint3dPrintedSupportRadius,$fn=8);
  };
  translate([mainDepth/2 + lidSnapJointGapWidthForRemoval/2 + snapJoint3dPrintedSupportRadius, mainShellThickness - snapJoint3dPrintedSupportRadius, mainHeight - lidSnapJointOffsetFromTop - lidSnapJointProtrusionHeight]) {
    cylinder(lidSnapJointProtrusionHeight,snapJoint3dPrintedSupportRadius,snapJoint3dPrintedSupportRadius,$fn=8);
  };
  //3D printing supports for right-side lid snap-fit indent
  translate([mainDepth/2 - lidSnapJointGapWidthForRemoval/2 - (lidSnapJointWidth - lidSnapJointGapWidthForRemoval)/4, mainShellThickness - snapJoint3dPrintedSupportRadius, mainHeight - lidSnapJointOffsetFromTop - lidSnapJointProtrusionHeight]) {
    cylinder(lidSnapJointProtrusionHeight,snapJoint3dPrintedSupportRadius,snapJoint3dPrintedSupportRadius,$fn=8);
  };
  translate([mainDepth/2 - lidSnapJointGapWidthForRemoval/2 - snapJoint3dPrintedSupportRadius, mainShellThickness - snapJoint3dPrintedSupportRadius, mainHeight - lidSnapJointOffsetFromTop - lidSnapJointProtrusionHeight]) {
    cylinder(lidSnapJointProtrusionHeight,snapJoint3dPrintedSupportRadius,snapJoint3dPrintedSupportRadius,$fn=8);
  };
};
  
