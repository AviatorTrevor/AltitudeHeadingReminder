//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file.
pcbBoardHeight = 36.9;
pcbBoardWidth = 75.7;
innerLipHeightAboveOuterLipHeight = resolution * 5;
mainShellThickness = 5 * resolution;
mainDepth = 64;
mainWidth = 84.7;
mainHeight = pcbBoardHeight + mainShellThickness + innerLipHeightAboveOuterLipHeight;
caseCornerRadius = 2.5;
cylinderFragments = 70;

batteryThicknessAndSpacing = 8;
pcbBoardThickness = 2;
pcbMountingWallThickness = 2;
pcbBottomRightWidth = 5.6;
pcbLidTopRightZAxisHeight = 1.2;
pcbOffsetX = mainShellThickness + batteryThicknessAndSpacing;
pcbOffsetY = mainWidth - mainShellThickness - pcbBoardWidth;
pcbBottomRightHeight = 8;
pcbLeftWidth = 3.1;

usbSlotWidth = 7;
usbSlotHeight = 27;
usbSlotXoffset = pcbOffsetX; //offset above mainShellThickness
usbSlotZoffset = 8.5 + usbSlotWidth / 2;

lidThickness = resolution * 4; //z-axis
lidLipWidth = resolution * 2;  //x & y axis thickness of the lip/wall
outerLipHeight = resolution * 5; //height above lidThickness
innerLipHeight = outerLipHeight + innerLipHeightAboveOuterLipHeight; //height above lidThickness
spacingForLidLipFromCaseWall = resolution / 2;
margin = mainShellThickness + lidLipWidth + spacingForLidLipFromCaseWall;
marginFromSnapJointCutaway = resolution;

lidSnapJointProtrusionHeight = 1.5;
lidSnapJointWidth = mainDepth / 3;
lidSnapJointProtrusionLength = mainShellThickness;
lidSnapJointOffsetFromTop = 3;
extensionBeyondOuterLipForSnapJoint = 2;

frontFaceThickness = 3 * resolution;
displayThickness = 3.5; //this is used for the snap-fit mechanism 
displayBackLegThickness = 1.2;
knobAndDisplaySupportWallDepth = displayThickness  - (mainShellThickness - frontFaceThickness) + displayBackLegThickness;

//keep in-sync with the mounting plate file
mountingHoleRadius = 4.5;
mountingQuarterInchRadius = 3.9;
mountingPillarRadius = mountingHoleRadius + 2.5;
mountingHoleHeight = 9.8;
mountingTransitionHeight = 4;
mountingQuarterInchCylinderHeight = mainHeight - mountingHoleHeight - mountingTransitionHeight - 16;
mountingPillarHeight = mainHeight - 14.5;

supportBracketThickness = 0.45;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
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
displayPinsYaxisWidth = 2.7;
knobAndDisplaySupportWallWidth = 3.5;
displaySideRetainingPillarThickness = displayThickness - (mainShellThickness - frontFaceThickness);
knobHoleYoffset = displayYOffset + (displayWidth / 2);
knobHoleZoffset = mainShellThickness + 11.9;
knobHoleRadius = 3.8;
knobHoleDepth = mainShellThickness + knobAndDisplaySupportWallDepth;
knobIndentSquareSideLength = 12.4;
knobHookZaxisOffset = 5.1;
knobHookYaxisWidth = 2.2;
knobHookXaxisDepth = knobHoleDepth - resolution*1.5;
knobHookZaxisHeight = 1.7;
gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout = (mainHeight - displayTopOffset + displayIndentZOffsetFromDisplayZ) - (knobHoleZoffset + knobIndentSquareSideLength/2);
mountingHoleX = mainDepth - mountingPillarRadius - 3.3;


//MODULE snap joint for lid
module createLeftSideLidSnapJoint()
{
  translate([mainDepth/2 - lidSnapJointWidth/2, 0, mainHeight - lidSnapJointOffsetFromTop - lidSnapJointProtrusionHeight + 0.2]) { //NOTE: keep 0.2 in sync
    cube([lidSnapJointWidth, mainShellThickness, lidSnapJointProtrusionHeight + 0.5]);
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
  translate([mainDepth - knobHoleDepth, mainWidth/2 - knobHoleYoffset - knobHookYaxisWidth / 2, knobHoleZoffset - knobHookZaxisOffset - knobHookZaxisHeight]) {
    cube([knobHookXaxisDepth, knobHookYaxisWidth, knobHookZaxisHeight]);
  };
};
module cutoutRightKnobStuff() {
  //right knob
  translate([0,mainWidth,0]) {
    mirror([0,1,0]) {
      cutoutLeftKnobStuff();
    };
  };
};

module cutoutLeftDisplayIndent() {
  //left display indent to make the wall thinner
  translate([mainDepth - displayDepth, mainWidth/2 - displayWidth - displayYOffset + displayIndentYOffsetFromDisplayY, mainHeight - displayTopOffset + displayIndentZOffsetFromDisplayZ]) {
    cube([mainShellThickness - frontFaceThickness, displayIndentWidth, displayIndentHeight]);
  };
  //extra cutout for pins
  translate([mainDepth - displayDepth, mainWidth/2 - displayWidth - displayYOffset + displayIndentYOffsetFromDisplayY, mainHeight - displayTopOffset + displayIndentZOffsetFromDisplayZ]) {
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
  
  //3D print supports for lid snap joint, left side
  translate([mainDepth/2 - supportBracketThickness/2, 0, mainHeight - lidSnapJointOffsetFromTop - lidSnapJointProtrusionHeight + 0.2]) { //NOTE: keep 0.2 in sync
    cube([supportBracketThickness, mainShellThickness, lidSnapJointProtrusionHeight + 0.5]);
  };
  
  //3D print supports for lid snap joint, right side
  translate([mainDepth/2 - supportBracketThickness/2, mainWidth - mainShellThickness, mainHeight - lidSnapJointOffsetFromTop - lidSnapJointProtrusionHeight + 0.2]) { //NOTE: keep 0.2 in sync
    cube([supportBracketThickness, mainShellThickness, lidSnapJointProtrusionHeight + 0.5]);
  };
  
  //bottom mounting hole
  difference() {
    translate([mountingHoleX, mainWidth/2, 0]) {
      cylinder(mountingPillarHeight, mountingPillarRadius, mountingPillarRadius, $fn=cylinderFragments);
    };
    translate([mountingHoleX, mainWidth/2, mountingHoleHeight]) {
      cylinder(mountingTransitionHeight, mountingHoleRadius, mountingQuarterInchRadius, $fn=cylinderFragments);
    };
    translate([mountingHoleX, mainWidth/2, mountingHoleHeight + mountingTransitionHeight]) {
      cylinder(mountingQuarterInchCylinderHeight, mountingQuarterInchRadius, mountingQuarterInchRadius, $fn=cylinderFragments);
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
    //special-case, remove the stuff for indented-display again
    cutoutLeftDisplayIndent();
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


  //PCB right back-side support
  translate([mainShellThickness, mainWidth - mainShellThickness - pcbBottomRightWidth, mainShellThickness]) {
    cube([pcbOffsetX - mainShellThickness, pcbBottomRightWidth, pcbBottomRightHeight]);
  };
  //PCB bottom right front-side support
  translate([pcbOffsetX + pcbBoardThickness, mainWidth - mainShellThickness - pcbBottomRightWidth, mainShellThickness]) {
    cube([pcbMountingWallThickness, pcbBottomRightWidth, pcbBottomRightHeight]);
  };
  
  //PCB left side back-side support
  translate([mainShellThickness, pcbOffsetY - pcbMountingWallThickness, mainShellThickness]) {
    cube([pcbOffsetX - mainShellThickness, pcbLeftWidth + pcbMountingWallThickness, pcbBoardHeight]);
  };
  //PCB left side front-side support
  translate([pcbOffsetX + pcbBoardThickness, pcbOffsetY - pcbMountingWallThickness, mainShellThickness]) {
    cube([pcbMountingWallThickness, pcbLeftWidth + pcbMountingWallThickness, pcbBoardHeight]);
  };
  //PCB left side, left containing wall
  translate([pcbOffsetX, pcbOffsetY - pcbMountingWallThickness, mainShellThickness]) {
    cube([pcbBoardThickness, pcbMountingWallThickness, pcbBoardHeight]);
  };
 
  //supporting back leg for right display
  translate([mainDepth - mainShellThickness - knobAndDisplaySupportWallDepth, mainWidth/2 + knobHoleYoffset + knobIndentSquareSideLength/2 + knobAndDisplaySupportWallWidth, mainShellThickness]) {
    cube([displayBackLegThickness, knobAndDisplaySupportWallWidth + 6.7, displayIndentHeight + knobHoleZoffset + knobIndentSquareSideLength/2 + gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout - innerLipHeightAboveOuterLipHeight]);
  };
  
  //supporting back leg for left display
  translate([mainDepth - mainShellThickness - knobAndDisplaySupportWallDepth, mainWidth/2 - knobHoleYoffset + knobIndentSquareSideLength/2 + knobAndDisplaySupportWallWidth, mainShellThickness]) {
    cube([displayBackLegThickness, knobAndDisplaySupportWallWidth + 9.7, displayIndentHeight + knobHoleZoffset + knobIndentSquareSideLength/2 + gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout - innerLipHeightAboveOuterLipHeight]);
  };
  
  //left display, left support bracket
  translate([mainDepth - frontFaceThickness - displayThickness, mainWidth/2 - displayWidth - displayYOffset + displayIndentYOffsetFromDisplayY - displaySideRetainingPillarThickness + 0.15, mainShellThickness]) {
    cube([displayThickness - (mainShellThickness - frontFaceThickness), displaySideRetainingPillarThickness, mainHeight - mainShellThickness - displayTopOffset + displayIndentZOffsetFromDisplayZ + displayIndentHeight]);
  };
  //center support bracket
  translate([mainDepth - frontFaceThickness - displayThickness, mainWidth/2 - displayWidth - displayYOffset + displayIndentYOffsetFromDisplayY + displayIndentWidth, mainShellThickness]) {
    cube([displayThickness - (mainShellThickness - frontFaceThickness) + mainShellThickness, displayYOffset*2 + displayWidth - displayIndentWidth + 0.15, mainHeight - mainShellThickness - displayTopOffset + displayIndentZOffsetFromDisplayZ + displayIndentHeight]);
  };
  
  //3D support pillar jail-bars for left display
  translate([mainDepth - frontFaceThickness, mainWidth/2 - displayYOffset - displayWidth/2 - supportBracketThickness*0.45, mainHeight - displayTopOffset]) {
    cube([frontFaceThickness, supportBracketThickness*0.9, displayHeight]);
  };
  //3D support pillar jail-bars for right display
  translate([mainDepth - frontFaceThickness, mainWidth/2 + displayYOffset + displayWidth/2 - supportBracketThickness*0.45, mainHeight - displayTopOffset]) {
    cube([frontFaceThickness, supportBracketThickness*0.9, displayHeight]);
  };
};
  
