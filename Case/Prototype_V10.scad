//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file
pcbBoardHeight = 36.8;
pcbBoardWidth = 76.1;
innerLipHeightAboveOuterLipHeight = resolution * 5;
mainShellThickness = 5 * resolution;
mainDepth = 52;
mainWidth = 92;
mainHeight = pcbBoardHeight + mainShellThickness + innerLipHeightAboveOuterLipHeight;
caseCornerRadius = 2.5;
cylinderFragments = 70;

lidThickness = resolution * 4; //z-axis
lidLipWidth = resolution * 2;  //x & y axis thickness of the lip/wall
outerLipHeight = resolution * 5; //height above lidThickness
innerLipHeight = outerLipHeight + innerLipHeightAboveOuterLipHeight; //height above lidThickness
spacingForLidLipFromCaseWall = resolution / 2;
margin = mainShellThickness + lidLipWidth + spacingForLidLipFromCaseWall;
marginFromSnapJointCutaway = resolution;

lidSnapJointProtrusionHeight = 1.5;
lidSnapJointWidth = mainDepth / 3;
lidSnapJointProtrusionLength = mainShellThickness - resolution;
lidSnapJointOffsetFromTop = 3;
extensionBeyondOuterLipForSnapJoint = 2;

//keep in-sync with the mounting plate file
mountingHoleRadius = 4.5;
mountingPillarRadius = mountingHoleRadius + 1.5;
mountingHoleHeight = 8.7;
mountingPillarHeight = mountingHoleHeight + 0.5;

supportBracketThickness = 0.45;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

batteryThicknessAndSpacing = 8;
frontFaceThickness = 3 * resolution;
displayEdgeBuffer = 0.8;
displayWidth = 22.5 + displayEdgeBuffer*2;
displayDepth = mainShellThickness;
displayHeight = 6 + displayEdgeBuffer*2;
displayYOffset = 8.6;
displayIndentWidth = 38.5;
displayIndentHeight = 12.4;
displayIndentYOffsetFromDisplayY = -5.7 - displayEdgeBuffer;
displayIndentZOffsetFromDisplayZ = -2.7 - displayEdgeBuffer;
displayTopOffset = displayIndentHeight + displayIndentZOffsetFromDisplayZ + innerLipHeightAboveOuterLipHeight;
displayPinsYaxisWidth = 2.9;
displayThickness = 3.8; //this is used for the snap-fit mechanism 
displayBackLegThickness = 2;
knobAndDisplaySupportWallDepth = displayThickness  - (mainShellThickness - frontFaceThickness) + displayBackLegThickness;
knobAndDisplaySupportWallWidth = 3.5;
displaySideRetainingPillarThickness = displayThickness - (mainShellThickness - frontFaceThickness);
knobHoleYoffset = displayYOffset + (displayWidth / 2);
knobHoleZoffset = mainHeight - displayTopOffset - 12.1;
knobHoleRadius = 3.8;
knobHoleDepth = mainShellThickness + knobAndDisplaySupportWallDepth;
knobIndentSquareSideLength = 12.4;
knobHookYaxisOffset = 5.1;
knobHookYaxisWidth = 1.7;
knobHookXaxisDepth = knobHoleDepth - resolution;
knobHookZaxisHeight = 2.2;
gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout = (mainHeight - displayTopOffset + displayIndentZOffsetFromDisplayZ) - (knobHoleZoffset + knobIndentSquareSideLength/2);
pcbBoardThickness = 2;
pcbMountingWallThickness = 2;
pcbOffsetX = mainShellThickness + batteryThicknessAndSpacing;
pcbOffsetY = mainWidth - mainShellThickness - pcbBoardWidth;
pcbBottomRightWidth = 5.6;
pcbBottomRightHeight = 8;
pcbLeftWidth = 3.1;
usbSlotWidth = 7;
usbSlotHeight = 27 - usbSlotWidth;
usbSlotXoffset = pcbOffsetX; //offset above mainShellThickness
usbSlotZoffset = 8.5 + usbSlotWidth / 2;
mountingHoleX = mainDepth - mountingPillarRadius - 6;


//MODULE snap joint for lid
module createLeftSideLidSnapJoint()
{
  translate([mainDepth/2 - lidSnapJointWidth/2, 0, mainHeight - lidSnapJointOffsetFromTop - lidSnapJointProtrusionHeight - 0.5]) {
    cube([lidSnapJointWidth, mainShellThickness, lidSnapJointProtrusionHeight + 1]);
  };
};

module cutoutLeftKnobStuff() {
  //left knob
  translate([mainDepth - knobHoleDepth, mainWidth/2 - knobHoleYoffset, knobHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobHoleRadius,knobHoleRadius,$fn=cylinderFragments);
    };
  };
  //left knob hook slot
  translate([mainDepth - knobHoleDepth, mainWidth/2 - knobHoleYoffset - knobHookYaxisOffset - knobHookYaxisWidth, knobHoleZoffset - knobHookZaxisHeight / 2]) {
    cube([knobHookXaxisDepth, knobHookYaxisWidth, knobHookZaxisHeight]);
  };
  //left knob indent
  translate([mainDepth - knobHoleDepth, mainWidth/2 - knobHoleYoffset - knobIndentSquareSideLength/2, knobHoleZoffset - knobIndentSquareSideLength/2]) {
    cube([knobHoleDepth - mainShellThickness, knobIndentSquareSideLength, knobIndentSquareSideLength + gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout]);
  };
};
module cutoutRightKnobStuff() {
  //right knob
  translate([mainDepth - knobHoleDepth, mainWidth/2 + knobHoleYoffset, knobHoleZoffset]) {
    rotate([0,90,0]) {
      cylinder(knobHoleDepth,knobHoleRadius,knobHoleRadius,$fn=cylinderFragments);
    };
  };
  //right knob hook slot
  translate([mainDepth - knobHoleDepth, mainWidth/2 + knobHoleYoffset + knobHookYaxisOffset, knobHoleZoffset - knobHookZaxisHeight / 2]) {
    cube([knobHookXaxisDepth, knobHookYaxisWidth, knobHookZaxisHeight]);
  };
  //right knob indent
  translate([mainDepth - knobHoleDepth, mainWidth/2 + knobHoleYoffset - knobIndentSquareSideLength/2, knobHoleZoffset - knobIndentSquareSideLength/2]) {
    cube([knobHoleDepth - mainShellThickness, knobIndentSquareSideLength, knobIndentSquareSideLength + gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout]);
  };
};

module cutoutLeftDisplayIndent() {
  //left display indent to make the wall thinner
  translate([mainDepth - displayDepth, mainWidth/2 - displayWidth - displayYOffset + displayIndentYOffsetFromDisplayY, mainHeight - displayTopOffset + displayIndentZOffsetFromDisplayZ]) {
    cube([mainShellThickness - frontFaceThickness, displayIndentWidth, displayIndentHeight]);
  };
  //extra cutout for pins
  translate([mainDepth - displayDepth, mainWidth/2 - displayWidth - displayYOffset + displayIndentYOffsetFromDisplayY + 1, mainHeight - displayTopOffset + displayIndentZOffsetFromDisplayZ]) {
    cube([mainShellThickness - resolution*1.5, displayPinsYaxisWidth, displayIndentHeight]);
  };
};

module cutoutRightDisplayIndent() {
  //right display indent to make the wall thinner
  translate([mainDepth - displayDepth, mainWidth/2 + displayYOffset + displayIndentYOffsetFromDisplayY, mainHeight - displayTopOffset + displayIndentZOffsetFromDisplayZ]) {
    cube([mainShellThickness - frontFaceThickness, displayIndentWidth, displayIndentHeight]);
  };
  //extra cutout for pins
  translate([mainDepth - displayDepth, mainWidth/2 + displayYOffset + displayIndentYOffsetFromDisplayY, mainHeight - displayTopOffset + displayIndentZOffsetFromDisplayZ]) {
    cube([mainShellThickness - resolution*1.5, displayPinsYaxisWidth, displayIndentHeight]);
  };
};

//yOffset = the center of the rotary knob hole
module createBackLegSetForDisplays(yOffset) {
  translate([mainDepth - mainShellThickness - knobAndDisplaySupportWallDepth, yOffset + knobIndentSquareSideLength/2, knobHoleZoffset + knobIndentSquareSideLength/2 + gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout]) {
    createBackLegForDisplay();
  };
};

module createBackLegForDisplay() {
  //leg
  cube([displayBackLegThickness, knobAndDisplaySupportWallWidth, displayIndentHeight]);
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
    cutoutLeftDisplayIndent();
    
    //right display
    translate([mainDepth - displayDepth, mainWidth/2 + displayYOffset, mainHeight - displayTopOffset]) {
      cube([displayDepth, displayWidth, displayHeight]);
    };
    cutoutRightDisplayIndent();
    
    cutoutLeftKnobStuff();
    cutoutRightKnobStuff();
    
    //usb cable and power switch slot right side.
    translate([usbSlotXoffset, mainWidth - mainShellThickness, mainShellThickness + usbSlotZoffset]) {
      cube([usbSlotWidth, mainShellThickness, usbSlotHeight]);
    };
    //rounded edge cut-out at bottom
    translate([usbSlotXoffset + usbSlotWidth/2, mainWidth - mainShellThickness, mainShellThickness + usbSlotZoffset]) {
      rotate([270,90,0]) {
        cylinder(mainShellThickness, usbSlotWidth/2, usbSlotWidth/2, $fn=cylinderFragments);
      };
    };
    //rounded edge cut-out at top
    translate([usbSlotXoffset + usbSlotWidth/2, mainWidth - mainShellThickness, mainShellThickness + usbSlotHeight + usbSlotZoffset]) {
      rotate([270,90,0]) {
        cylinder(mainShellThickness, usbSlotWidth/2, usbSlotWidth/2, $fn=cylinderFragments);
      };
    };
    
    //bottom mounting hole
    translate([mountingHoleX, mainWidth/2, 0]) {
      cylinder(mainShellThickness, mountingHoleRadius, mountingHoleRadius, $fn=cylinderFragments);
    };
    
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
    //special-case, remove the stuff for indented-display again
    cutoutRightDisplayIndent();
  };

  //adding cylinder for case rounded edge, rear-right
  difference() {
    translate([caseCornerRadius, mainWidth - caseCornerRadius, 0]) {
      cylinder(mainHeight, caseCornerRadius, caseCornerRadius, $fn=cylinderFragments);
    };
    translate([mainShellThickness, mainWidth - caseCornerRadius*2, mainShellThickness]) {
      cube([caseCornerRadius*2 - mainShellThickness, caseCornerRadius*2 - mainShellThickness, mainHeight - mainShellThickness]);
    };
  };

  
  //supporting vertical wall for left rotary
  //left knob supporting walls
  difference() {
    union() {
      translate([mainDepth - mainShellThickness - knobAndDisplaySupportWallDepth, mainWidth/2 - knobHoleYoffset - knobIndentSquareSideLength/2 - knobAndDisplaySupportWallWidth, mainShellThickness]) {
        cube([knobAndDisplaySupportWallDepth, knobAndDisplaySupportWallWidth, knobHoleZoffset + knobIndentSquareSideLength/2 + gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout - mainShellThickness]);
      };
      translate([mainDepth - mainShellThickness - knobAndDisplaySupportWallDepth, mainWidth/2 - knobHoleYoffset + knobIndentSquareSideLength/2, mainShellThickness]) {
        cube([knobAndDisplaySupportWallDepth, knobAndDisplaySupportWallWidth, knobHoleZoffset + knobIndentSquareSideLength/2 + gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout - mainShellThickness]);
      };
    };
    cutoutLeftKnobStuff();
  };
  //right knob supporting walls
  difference() {
    union() {
      translate([mainDepth - mainShellThickness - knobAndDisplaySupportWallDepth, mainWidth/2 + knobHoleYoffset - knobIndentSquareSideLength/2 - knobAndDisplaySupportWallWidth, mainShellThickness]) {
        cube([knobAndDisplaySupportWallDepth, knobAndDisplaySupportWallWidth, knobHoleZoffset + knobIndentSquareSideLength/2 + gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout - mainShellThickness]);
      };
      translate([mainDepth - mainShellThickness - knobAndDisplaySupportWallDepth, mainWidth/2 + knobHoleYoffset + knobIndentSquareSideLength/2, mainShellThickness]) {
        cube([knobAndDisplaySupportWallDepth, knobAndDisplaySupportWallWidth, knobHoleZoffset + knobIndentSquareSideLength/2 + gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout - mainShellThickness]);
      };
    };
    cutoutRightKnobStuff();
  };


  //PCB bottom right back-side support
  translate([mainShellThickness, mainWidth - mainShellThickness - pcbBottomRightWidth, mainShellThickness]) {
    cube([pcbOffsetX - mainShellThickness, pcbBottomRightWidth, pcbBottomRightHeight]);
  };
  //PCB bottom right front-side support
  translate([pcbOffsetX + pcbBoardThickness, mainWidth - mainShellThickness - pcbBottomRightWidth, mainShellThickness]) {
    cube([pcbMountingWallThickness, pcbBottomRightWidth, pcbBottomRightHeight]);
  };
  
  //PCB left side back-side support
  translate([mainShellThickness, pcbOffsetY - pcbMountingWallThickness, mainShellThickness]) {
    cube([pcbOffsetX - mainShellThickness, pcbLeftWidth + pcbMountingWallThickness, pcbBoardHeight + pcbMountingWallThickness]);
  };
  //PCB left side front-side support
  translate([pcbOffsetX + pcbBoardThickness, pcbOffsetY - pcbMountingWallThickness, mainShellThickness]) {
    cube([pcbMountingWallThickness, pcbLeftWidth + pcbMountingWallThickness, pcbBoardHeight + pcbMountingWallThickness]);
  };
  //PCB left side, left containing wall
  translate([pcbOffsetX, pcbOffsetY - pcbMountingWallThickness, mainShellThickness]) {
    cube([pcbBoardThickness, pcbMountingWallThickness, pcbBoardHeight + pcbMountingWallThickness]);
  };
 
  //support snap joins for left & right displays
  createBackLegSetForDisplays(mainWidth/2 - knobHoleYoffset);
  createBackLegSetForDisplays(mainWidth/2 + knobHoleYoffset);
  
  //left display, left support bracket
  translate([mainDepth - frontFaceThickness - displayThickness, mainWidth/2 - displayWidth - displayYOffset + displayIndentYOffsetFromDisplayY - displaySideRetainingPillarThickness, mainShellThickness]) {
    cube([displayThickness - (mainShellThickness - frontFaceThickness), displaySideRetainingPillarThickness, mainHeight - mainShellThickness - displayTopOffset + displayIndentZOffsetFromDisplayZ + displayIndentHeight]);
  };
  //center support bracket
  translate([mainDepth - frontFaceThickness - displayThickness, mainWidth/2 - displayWidth - displayYOffset + displayIndentYOffsetFromDisplayY + displayIndentWidth, mainShellThickness]) {
    cube([displayThickness - (mainShellThickness - frontFaceThickness), displayYOffset*2 + displayWidth - displayIndentWidth, mainHeight - mainShellThickness - displayTopOffset + displayIndentZOffsetFromDisplayZ + displayIndentHeight]);
  };
  //right display, right support bracket
  translate([mainDepth - frontFaceThickness - displayThickness, mainWidth/2 + displayYOffset + displayIndentYOffsetFromDisplayY + displayIndentWidth, mainShellThickness]) {
    cube([displayThickness - (mainShellThickness - frontFaceThickness), displaySideRetainingPillarThickness, mainHeight - mainShellThickness - displayTopOffset + displayIndentZOffsetFromDisplayZ + displayIndentHeight]);
  };
};
  
