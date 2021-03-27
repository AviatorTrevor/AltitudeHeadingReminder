//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file
pcbBoardHeight = 36.8;
pcbBoardWidth = 75.8;
innerLipHeightAboveOuterLipHeight = resolution * 5;
mainShellThickness = 5 * resolution;
mainDepth = 55;
mainWidth = 92;
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
lidSnapJointProtrusionLength = mainShellThickness - resolution;
lidSnapJointOffsetFromTop = 3;
extensionBeyondOuterLipForSnapJoint = 2;

frontFaceThickness = 3 * resolution;
displayThickness = 3.2; //this is used for the snap-fit mechanism 
displayBackLegThickness = 2;
knobAndDisplaySupportWallDepth = displayThickness  - (mainShellThickness - frontFaceThickness) + displayBackLegThickness;

lidSnapJointHingeThickness = outerLipHeight * 0.4; //has to be quite a bit smaller than outerLipHeight
lidSnapJointLegThickness = resolution * 3;
lidSnapJointCutoutGapFromEdge = resolution * 2;

outerLipWidth = mainShellThickness + spacingForLidLipFromCaseWall;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

buzzerRadius = 30.4 / 2; //30.4 is diameter
buzzerScrewOffset = 20.5; //20.5 mm from center of buzzer hole to center of screw hole
buzzerScrewRadius = 1.9;
buzzerX = mainDepth - buzzerRadius - mainShellThickness - knobAndDisplaySupportWallDepth + 1;
buzzerY = mainWidth / 2;
buzzerScrewHeight = 5;
buzzerScrewHeightForBaseHolding = 2.5;
buzzerPlatformOffset = 3.8; //offset from center of cylinder to edge of wall to clear the buzzer
buzzerPlatformThickness = 7 * resolution;
buzzerScrewPylonZOffset = -0.8; //distance from the lid to fit the buzzer into the slot

screwHeadCylinderRadius = 1.8;
screwHeadSinkDepth = lidThickness * 0.6;

snapHook3dPrintingSupportPillarThickness = resolution * 1.4;
buzzer3dPrintingSupportPillarThicknessRadius = resolution * 0.9;

module rotate_about_pt(z, y, pt) {
    translate(pt)
        rotate([0, y, z])
            translate(-pt)
                children();   
}

module innerCube() {
  translate([margin, margin, lidThickness]) {
    cube([mainDepth - margin*2, mainWidth - margin*2, innerLipHeight]);
  };
};

module cutOutLeftSideForHinge() {
  //cut away the inner wall for left side snap fit stuff + buffer on either side
  translate([mainDepth/2 - lidSnapJointWidth/2 - marginFromSnapJointCutaway, outerLipWidth, lidThickness]) {
    cube([lidSnapJointWidth + marginFromSnapJointCutaway*2, lidLipWidth, innerLipHeight]);
  };
};

module leftBuzzerPlatform() {
  translate([buzzerX, buzzerY - buzzerScrewOffset, lidThickness + buzzerScrewHeight - buzzerScrewPylonZOffset]) {
    cylinder(buzzerScrewHeightForBaseHolding/2, buzzerScrewRadius * 0.2, buzzerScrewRadius, $fn=cylinderFragments);
  };
  translate([buzzerX, buzzerY - buzzerScrewOffset, lidThickness + buzzerScrewHeight + buzzerScrewHeightForBaseHolding/2 - buzzerScrewPylonZOffset]) {
    cylinder(buzzerScrewHeightForBaseHolding/2 + buzzerPlatformThickness, buzzerScrewRadius, buzzerScrewRadius, $fn=cylinderFragments);
  };
  //3D printed support for screw mount
  translate([buzzerX, buzzerY - buzzerScrewOffset, lidThickness]) {
    cylinder(buzzerScrewHeight - buzzerScrewPylonZOffset, buzzer3dPrintingSupportPillarThicknessRadius, buzzer3dPrintingSupportPillarThicknessRadius, $fn=cylinderFragments);
  };
  
  //horizontal platform to hold buzzer
  translate([buzzerX - buzzerScrewRadius, buzzerY - buzzerScrewOffset - buzzerPlatformOffset - buzzerPlatformThickness, lidThickness + buzzerScrewHeight + buzzerScrewHeightForBaseHolding - buzzerScrewPylonZOffset]) {
    cube([2*buzzerScrewRadius, buzzerPlatformOffset + buzzerPlatformThickness,buzzerPlatformThickness]);
  };
  //vertical platform to mount the horizontal platform
  translate([buzzerX - buzzerScrewRadius, buzzerY - buzzerScrewOffset - buzzerPlatformOffset - buzzerPlatformThickness, lidThickness]) {
    cube([2*buzzerScrewRadius, buzzerPlatformThickness, buzzerScrewHeight + buzzerScrewHeightForBaseHolding - buzzerScrewPylonZOffset]);
  };
};

difference() {
  union() {
    //top cover of the lid
    cube([mainDepth, mainWidth, lidThickness]);
      
    //outer lip
    difference() {
      translate([0, 0, lidThickness]) {
        cube([mainDepth, mainWidth, outerLipHeight]);
      };
      translate([outerLipWidth, outerLipWidth, lidThickness]) {
        cube([mainDepth - outerLipWidth*2, mainWidth - outerLipWidth*2, outerLipHeight]);
      };
    };
    
    //add rails for better seating
    difference() {
      translate([margin - lidLipWidth, margin - lidLipWidth, lidThickness]) {
        cube([mainDepth - margin*2 + lidLipWidth*2, mainWidth - margin*2 + lidLipWidth*2, innerLipHeight]);
      };
      innerCube();
    };
  };

  //cutting out the inside to save on plastic used
  innerCube();
  
  //cutting away room at the corner of the lip to make room for the cylinders where screws go.
  //rear-left
  translate([0, 0, 0]) {
    cube([caseCornerRadius, caseCornerRadius, lidThickness + outerLipHeight]);
  };
  
  //cutting away room at the corner of the lip to make room for the cylinders where screws go.
  //front-left
  translate([mainDepth - caseCornerRadius, 0, 0]) {
    cube([caseCornerRadius, caseCornerRadius, lidThickness + outerLipHeight]);
  };
  
  //cutting away room at the corner of the lip to make room for the cylinders where screws go.
  //front-right
  translate([mainDepth - caseCornerRadius, mainWidth - caseCornerRadius, 0]) {
    cube([caseCornerRadius, caseCornerRadius, lidThickness + outerLipHeight]);
  };
  
  //cutting away room at the corner of the lip to make room for the cylinders where screws go.
  //rear-right
  translate([0, mainWidth - caseCornerRadius, 0]) {
    cube([caseCornerRadius, caseCornerRadius, lidThickness + outerLipHeight]);
  };
  
  //buzzer alarm hole
  translate([buzzerX, buzzerY, 0]) {
    cylinder(lidThickness, buzzerRadius, buzzerRadius, $fn=cylinderFragments*2);
  };
  
  cutOutLeftSideForHinge();
  translate([0,mainWidth - 0.001,0]) {
    mirror([0,1,0]) {
      cutOutLeftSideForHinge();
    };
  };
};

module snapHook3dPrintingSupportPillarBeneathProtrusion() {
  cube([snapHook3dPrintingSupportPillarThickness, snapHook3dPrintingSupportPillarThickness, lidSnapJointOffsetFromTop]);
};

module leftSideSnapHook() {
  union() {
    //legs of the snap hook
    translate([mainDepth/2 - lidSnapJointWidth/2, outerLipWidth, lidThickness]) {
      cube([lidSnapJointWidth, lidSnapJointLegThickness, outerLipHeight + lidSnapJointOffsetFromTop + lidSnapJointProtrusionHeight]);
    };
    
    //snap hook 3D triangle
    polyhedron(
      points=[[mainDepth/2 - lidSnapJointWidth/2,outerLipWidth,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop], //0
              [mainDepth/2 - lidSnapJointWidth/2,outerLipWidth,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop + lidSnapJointProtrusionHeight], //1
              [mainDepth/2 - lidSnapJointWidth/2,outerLipWidth - lidSnapJointProtrusionLength,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop], //2
              [mainDepth/2 + lidSnapJointWidth/2,outerLipWidth,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop], //3
              [mainDepth/2 + lidSnapJointWidth/2,outerLipWidth,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop + lidSnapJointProtrusionHeight], //4
              [mainDepth/2 + lidSnapJointWidth/2,outerLipWidth - lidSnapJointProtrusionLength,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop]], //5
      faces=[[1,2,5,4],
             [2,0,3,5],
             [0,1,4,3],
             [2,1,0],
             [5,3,4]]
    );
    
    //3D printing support pillars on the outside under the head
    translate([mainDepth/2 - lidSnapJointWidth/2, outerLipWidth - lidSnapJointProtrusionLength, lidThickness + outerLipHeight]) {
      snapHook3dPrintingSupportPillarBeneathProtrusion();
    };
    translate([mainDepth/2 - snapHook3dPrintingSupportPillarThickness/2, outerLipWidth - lidSnapJointProtrusionLength, lidThickness + outerLipHeight]) {
      snapHook3dPrintingSupportPillarBeneathProtrusion();
    };
    translate([mainDepth/2 + lidSnapJointWidth/2 - snapHook3dPrintingSupportPillarThickness, outerLipWidth - lidSnapJointProtrusionLength, lidThickness + outerLipHeight]) {
      snapHook3dPrintingSupportPillarBeneathProtrusion();
    };
  };
};

leftSideSnapHook();

//right side snap fit
translate([0,mainWidth,0]) {
  mirror([0,1,0]) {
    leftSideSnapHook();
  };
};


rotate_about_pt(-15, 0, [buzzerX, buzzerY, 0]) {
  leftBuzzerPlatform();
};

rotate_about_pt(-15, 0, [buzzerX, buzzerY, 0]) {
  translate([0,mainWidth,0]) {
    mirror([0,1,0]) {
      leftBuzzerPlatform();
    };
  };
};


//rear-right PCB board mounting
translate([margin, margin, lidThickness]) {
  cube([pcbOffsetX - margin, pcbBottomRightWidth - (lidLipWidth + spacingForLidLipFromCaseWall), pcbLidTopRightZAxisHeight + innerLipHeight]);
};
//front-right PCB board mounting
translate([pcbOffsetX + pcbBoardThickness, margin, lidThickness]) {
  cube([pcbMountingWallThickness, pcbBottomRightWidth - (lidLipWidth + spacingForLidLipFromCaseWall), pcbLidTopRightZAxisHeight + innerLipHeight]);
};
//front right PCB board top
translate([pcbOffsetX, margin, lidThickness]) {
  cube([pcbMountingWallThickness, pcbBottomRightWidth - (lidLipWidth + spacingForLidLipFromCaseWall),innerLipHeight]);
};

//left-side PCB board mounting
translate([pcbOffsetX, mainWidth - pcbOffsetY - pcbLeftWidth, lidThickness]) {
  cube([pcbBoardThickness, pcbLeftWidth, innerLipHeight]);
};

//add the curve to the rear-left
difference() {
  translate([caseCornerRadius, caseCornerRadius, 0]) {
  cylinder(lidThickness + outerLipHeight, caseCornerRadius, caseCornerRadius, $fn=cylinderFragments);
  };
  innerCube();
};


//add the curve to the front-left
difference() {
  translate([mainDepth - caseCornerRadius, caseCornerRadius, 0]) {
  cylinder(lidThickness + outerLipHeight, caseCornerRadius, caseCornerRadius, $fn=cylinderFragments);
  };
  innerCube();
};

//add the curve to the front-right
difference() {
  translate([mainDepth - caseCornerRadius, mainWidth - caseCornerRadius, 0]) {
  cylinder(lidThickness + outerLipHeight, caseCornerRadius, caseCornerRadius, $fn=cylinderFragments);
  };
  innerCube();
};

//add the curve to the rear-right
difference() {
  translate([caseCornerRadius, mainWidth - caseCornerRadius, 0]) {
  cylinder(lidThickness + outerLipHeight, caseCornerRadius, caseCornerRadius, $fn=cylinderFragments);
  };
  innerCube();
};

//USB slot curved end
difference() {
  translate([usbSlotXoffset, 0, outerLipHeight + lidThickness]) {
    cube([usbSlotWidth, outerLipWidth ,usbSlotWidth/2]);
  };
  
  translate([usbSlotXoffset + usbSlotWidth/2, 0, outerLipHeight + lidThickness + usbSlotWidth/2]) {
    rotate([270,90,0]) {
      cylinder(outerLipWidth, usbSlotWidth/2, usbSlotWidth/2, $fn=cylinderFragments);
    };
  };
};
