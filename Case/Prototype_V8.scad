//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file
mainShellThickness = 5 * resolution;
mainDepth = 60;
mainWidth = 92;
mainHeight = 37 + mainShellThickness;
caseCornerRadius = 2.5;
cylinderFragments = 70;

lidThickness = resolution * 4; //z-axis
lidLipWidth = resolution * 2;  //x & y axis thickness of the lip/wall
outerLipHeight = resolution * 4; //height above lidThickness
innerLipHeightAboveOuterLipHeight = resolution * 5;
innerLipHeight = outerLipHeight + innerLipHeightAboveOuterLipHeight; //height above lidThickness
spacingForLidLipFromCaseWall = resolution / 2;
margin = mainShellThickness + lidLipWidth + spacingForLidLipFromCaseWall;
marginFromSnapJointCutaway = resolution;

lidSnapJointProtrusionHeight = 1.5;
lidSnapJointWidth = mainDepth / 2;
lidSnapJointProtrusionLength = mainShellThickness - resolution;
lidSnapJointOffsetFromTop = 3;
extensionBeyondOuterLipForSnapJoint = 2;

lidSnapJointGapWidthForRemoval = 14;
lidSnapJointGapHeightForRemoval = 1.5;

//keep in-sync with the mounting plate file
mountingHoleRadius = 4.45;
mountingPillarRadius = mountingHoleRadius + 1.5;
mountingHoleHeight = 8.68;
mountingPillarHeight = mountingHoleHeight + 0.5;

supportBracketThickness = 0.45;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

frontFaceThickness = 3 * resolution;
displayEdgeBuffer = 0.8;
displayWidth = 22.5 + displayEdgeBuffer*2;
displayDepth = mainShellThickness;
displayHeight = 6 + displayEdgeBuffer*2;
displayYOffset = 8.6;
displayTopOffset = displayHeight + innerLipHeightAboveOuterLipHeight + 4;
displayIndentWidth = 38.5;
displayIndentHeight = 12.4;
displayIndentYOffsetFromDisplayY = -5.7 - displayEdgeBuffer;
displayIndentZOffsetFromDisplayZ = -1.6 - displayEdgeBuffer;
displayPinsYaxisWidth = 2.9;
displayThickness = 3.8; //this is used for the snap-fit mechanism
displayBackLegThickness 
displayBackLegThickness = 2;
knobAndDisplaySupportWallDepth = displayThickness  - (mainShellThickness - frontFaceThickness) + displayBackLegThickness;
knobAndDisplaySupportWallWidth = 3.5;
displaySideRetainingPillarThickness = displayThickness - (mainShellThickness - frontFaceThickness);
knobHoleYoffset = displayYOffset + (displayWidth / 2);
knobHoleZoffset = mainHeight - displayTopOffset - 11.1;
knobHoleRadius = 3.8;
knobHoleDepth = mainShellThickness + knobAndDisplaySupportWallDepth;
knobIndentSquareSideLength = 12.4;
knobHookYaxisOffset = 6;
knobHookYaxisWidth = 0.7;
knobHookXaxisDepth = knobHoleDepth - resolution;
knobHookZaxisHeight = 2.2;
gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout = (mainHeight - displayTopOffset + displayIndentZOffsetFromDisplayZ) - (knobHoleZoffset + knobIndentSquareSideLength/2);
pcbMountSpacingFromWallToCut = resolution * 2.5;
pcbMountHeight = mountingPillarHeight - mainShellThickness + 2; //height above mainShellThickness
pcbBoardThickness = 2;
pcbBoardDepth = 36.8;
pcbBoardWidth = 76.1;
pcbPadSideLength = 9;
pcbWallThickness = 1.8;
pcbOffsetX = mainShellThickness;
pcbOffsetY = mainWidth - mainShellThickness - pcbBoardWidth;
pcbShelfSideLength = 3;
pcbShelfThickness = 1;
pcbRearRightWidth = 14.5;
pcbRearRightDepth = 3.5;
pcbSnapJointHeadHeight = 4;
pcbSnapJointHeadProtrusionLength = pcbMountSpacingFromWallToCut + 1.3;
pcbSnapJointFlatLipLength = pcbSnapJointHeadHeight / 8;
usbSlotHeight = 7;
usbSlotWidth = 26.5 - usbSlotHeight;
usbSlotXoffset = pcbOffsetX + 2 + usbSlotHeight / 2;
usbSlotYoffset = pcbMountHeight; //offset above mainShellThickness
mountingHoleX = pcbOffsetX + pcbBoardDepth + pcbWallThickness + 0.5;


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
  translate([mainDepth - mainShellThickness - knobAndDisplaySupportWallDepth, yOffset - knobIndentSquareSideLength/2 - knobAndDisplaySupportWallWidth, knobHoleZoffset + knobIndentSquareSideLength/2 + gapBetweenTopOfKnobCutoutAndBottomOfDisplayCutout]) {
    createBackLegForDisplay();
  };
  
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
    translate([usbSlotXoffset, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset]) {
      cube([usbSlotWidth, mainShellThickness, usbSlotHeight]);
    };
    //NOTE: Keep in sync way below for cutting out the cylinder pillar
    translate([usbSlotXoffset, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset + usbSlotHeight/2]) {
      rotate([270,90,0]) {
        cylinder(mainShellThickness, usbSlotHeight/2, usbSlotHeight/2, $fn=cylinderFragments);
      };
    };
    //circular edge cut-out for USB and switch
    translate([usbSlotXoffset + usbSlotWidth, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset + usbSlotHeight/2]) {
      rotate([270,90,0]) {
        cylinder(mainShellThickness, usbSlotHeight/2, usbSlotHeight/2, $fn=cylinderFragments);
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
      cube([2.5*pcbPadSideLength - pcbWallThickness, pcbPadSideLength, pcbMountHeight + pcbBoardThickness]);
    };
    translate([pcbOffsetX, pcbOffsetY, mainShellThickness + pcbMountHeight]) {
      cube([2.5*pcbPadSideLength - pcbWallThickness, pcbPadSideLength - pcbWallThickness, pcbBoardThickness]);
    };
    //cut a slot so the side has flex
    translate([pcbOffsetX, pcbOffsetY, mainShellThickness]) {
      cube([2.5*pcbPadSideLength - pcbWallThickness, pcbMountSpacingFromWallToCut, pcbMountHeight]);
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
              [2.5*pcbPadSideLength - pcbMountSpacingFromWallToCut - pcbWallThickness, pcbWallThickness + pcbSnapJointHeadProtrusionLength, 0], //2
              [2.5*pcbPadSideLength - pcbMountSpacingFromWallToCut - pcbWallThickness, 0, 0], //3
              [0, pcbWallThickness + pcbSnapJointHeadProtrusionLength, pcbSnapJointFlatLipLength], //4
              [2.5*pcbPadSideLength - pcbMountSpacingFromWallToCut - pcbWallThickness, pcbWallThickness + pcbSnapJointHeadProtrusionLength, pcbSnapJointFlatLipLength], //5
              [0, pcbWallThickness, pcbSnapJointHeadHeight], //6
              [2.5*pcbPadSideLength - pcbMountSpacingFromWallToCut - pcbWallThickness, pcbWallThickness, pcbSnapJointHeadHeight], //7
              [0, 0, pcbSnapJointHeadHeight], //8
              [2.5*pcbPadSideLength - pcbMountSpacingFromWallToCut - pcbWallThickness, 0, pcbSnapJointHeadHeight]], //9
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
      cube([pcbPadSideLength, pcbPadSideLength, pcbMountHeight + pcbBoardThickness + pcbShelfThickness]);
    };
    translate([pcbOffsetX + pcbBoardDepth - pcbPadSideLength + pcbWallThickness, mainWidth - pcbPadSideLength, mainShellThickness + pcbMountHeight]) {
      cube([pcbPadSideLength - pcbWallThickness, pcbPadSideLength, pcbBoardThickness + pcbShelfThickness]);
    };
  };
  //shelf, front-right
  translate([pcbOffsetX + pcbBoardDepth - pcbShelfSideLength*2 + pcbWallThickness, mainWidth - mainShellThickness - pcbShelfSideLength/2, mainShellThickness + pcbMountHeight + pcbBoardThickness]) { //0.5 buffer for the board
    cube([pcbShelfSideLength*2, pcbShelfSideLength/2, pcbShelfThickness]);
  };
  //3D print support
  translate([pcbOffsetX + pcbBoardDepth - pcbShelfSideLength*2 + pcbWallThickness, mainWidth - mainShellThickness - pcbShelfSideLength/2, mainShellThickness + pcbMountHeight]) { //0.5 buffer for the board
    cube([supportBracketThickness, supportBracketThickness, pcbBoardThickness]);
  };

  //mounting seat for PCB, rear-right side
  translate([pcbOffsetX, mainWidth - mainShellThickness - pcbRearRightWidth, mainShellThickness]) {
    cube([pcbRearRightDepth, pcbRearRightWidth, pcbMountHeight]); //measurements taken from PCB board
  };
  
  createLeftLidSnapJoint3dPrintedSupports(); //left side
  translate([0, mainWidth, 0]) {
    mirror([0,1,0]) {
      createLeftLidSnapJoint3dPrintedSupports(); //right side
    };
  };




  //jail-bars over display slots
  translate([mainDepth - frontFaceThickness, mainWidth/2 - displayYOffset - displayWidth/2 - supportBracketThickness/4, mainHeight - displayTopOffset]) {
    cube([frontFaceThickness, supportBracketThickness/2, displayHeight]);
  };
  translate([mainDepth - frontFaceThickness, mainWidth/2 + displayYOffset + displayWidth/2 - supportBracketThickness/4, mainHeight - displayTopOffset]) {
    cube([frontFaceThickness, supportBracketThickness/2, displayHeight]);
  };
  //jail-bar over USB slot:
  usbJailBarSpacing = 2;
  translate([usbSlotXoffset + usbSlotWidth/2 - supportBracketThickness/2, mainWidth - mainShellThickness, mainShellThickness + usbSlotYoffset]) {
    cube([supportBracketThickness, mainShellThickness, usbSlotHeight]);
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
 
  //support snap joins for left & right displays
  createSnapJointSetForDisplays(mainWidth/2 - knobHoleYoffset);
  createSnapJointSetForDisplays(mainWidth/2 + knobHoleYoffset);
  
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

//3D printing supports for left-side lid snap-fit indent
module createLeftLidSnapJoint3dPrintedSupports() {
  snapJoint3dPrintedSupportRadius = supportBracketThickness/2;
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
  
