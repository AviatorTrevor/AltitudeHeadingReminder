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

lidThickness = resolution * 3; //z-axis
lidLipWidth = resolution * 2;  //x & y axis thickness of the lip/wall
outerLipHeight = resolution * 4; //height above lidThickness
innerLipHeight = outerLipHeight + resolution * 5; //height above lidThickness
margin = mainShellThickness + lidLipWidth;
marginFromSnapJointCutaway = resolution;

lidSnapJointProtrusionHeight = 2;
lidSnapJointWidth = mainDepth / 2;
lidSnapJointProtrusionLength = mainShellThickness * 3 / 5;
lidSnapJointOffsetFromTop = 1.2;
lidSnapJointHingeThickness = outerLipHeight / 3; //has to be quite a bit smaller than outerLipHeight

buzzerRadius = 30.4 / 2; //30.4 is diameter
buzzerScrewOffset = 20.5; //20.5 mm from center of buzzer hole to center of screw hole
buzzerScrewRadius = 1.9;
buzzerX = mainDepth - buzzerRadius * 1.4;
buzzerY = mainWidth / 2;

screwHeadCylinderRadius = 1.8;
screwHeadSinkDepth = lidThickness * 0.6;

module innerCube() {
  translate([margin, margin, lidThickness]) {
    cube([mainDepth - margin*2, mainWidth - margin*2, outerLipHeight]);
  };
};

module cutOutLeftSideForHinge() {
  //subtracting to make room for it to flex along the lip
  translate([mainDepth/2 - lidSnapJointWidth/2, lidSnapJointHingeThickness, lidThickness + lidSnapJointHingeThickness]) {
    cube([lidSnapJointWidth, mainShellThickness - lidSnapJointHingeThickness, outerLipHeight - lidSnapJointHingeThickness*2]);
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
      translate([mainShellThickness, mainShellThickness, lidThickness]) {
        cube([mainDepth - mainShellThickness*2, mainWidth - mainShellThickness*2, outerLipHeight]);
      };
    };
    
    //add rails for better seating
    difference() {
      translate([mainShellThickness, mainShellThickness, 0]) {
        cube([mainDepth - mainShellThickness*2, mainWidth - mainShellThickness*2, innerLipHeight]);
      };
      translate([mainShellThickness + lidLipWidth, mainShellThickness + lidLipWidth, lidThickness]) {
        cube([mainDepth - mainShellThickness*2 - lidLipWidth*2, mainWidth - mainShellThickness*2 - lidLipWidth*2, innerLipHeight - lidThickness]);
      };
      
      //cut away slots for left side snap fit stuff
      translate([mainDepth/2 - lidSnapJointWidth/2 - marginFromSnapJointCutaway, mainShellThickness, 0]) {
        cube([lidSnapJointWidth + marginFromSnapJointCutaway*2, lidLipWidth, innerLipHeight]);
      };
      //cut away slots for right side snap fit stuff
      translate([mainDepth/2 - lidSnapJointWidth/2 - marginFromSnapJointCutaway, mainWidth - mainShellThickness - lidLipWidth, 0]) {
        cube([lidSnapJointWidth + marginFromSnapJointCutaway*2, lidLipWidth, innerLipHeight]);
      };
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
    cylinder(lidThickness, buzzerRadius, buzzerRadius, $fn=cylinderFragments);
  };
  
  
  /*TODO later for non-3D-printed version:
  linear_extrude(resolution) {
    translate([11, mainWidth / 2, resolution]) {
      rotate([180, 0, 90]){
        text("Altitude/Heading Reminder", size = 4, halign = "center", valign = "center", font="Impact:style=Regular");
      }
    };
  };
  
  linear_extrude(resolution) {
    translate([5, mainWidth / 2, resolution]) {
      rotate([180, 0, 90]){
        text("Made by: Trevor Bartlett", size = 3, halign = "center", valign = "center", font="Impact:style=Regular");
      }
    };
  };*/
  
  cutOutLeftSideForHinge();
  translate([0,mainWidth,0]) {
    mirror([0,1,0]) {
      cutOutLeftSideForHinge();
    };
  };
};

//adding the hook on the left side for the snap fit
module leftSideSnapHook() {
  union() {
    translate([mainDepth/2 - lidSnapJointWidth/2, mainShellThickness, lidThickness + outerLipHeight - lidSnapJointHingeThickness]) {
      cube([lidSnapJointWidth, lidSnapJointHingeThickness, lidSnapJointHingeThickness + lidSnapJointOffsetFromTop + lidSnapJointProtrusionHeight]);
    };
    
    polyhedron(
      points=[[mainDepth/2 - lidSnapJointWidth/2,mainShellThickness,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop], //0
              [mainDepth/2 - lidSnapJointWidth/2,mainShellThickness,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop + lidSnapJointProtrusionHeight], //1
              [mainDepth/2 - lidSnapJointWidth/2,mainShellThickness - lidSnapJointProtrusionLength,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop], //2
              [mainDepth/2 + lidSnapJointWidth/2,mainShellThickness,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop], //3
              [mainDepth/2 + lidSnapJointWidth/2,mainShellThickness,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop + lidSnapJointProtrusionHeight], //4
              [mainDepth/2 + lidSnapJointWidth/2,mainShellThickness - lidSnapJointProtrusionLength,lidThickness + outerLipHeight + lidSnapJointOffsetFromTop]], //5
      faces=[[1,2,5,4],
             [2,0,3,5],
             [0,1,4,3],
             [2,1,0],
             [5,3,4]]
    );
  };
};

leftSideSnapHook();

//right side snap fit
translate([0,mainWidth,0]) {
  mirror([0,1,0]) {
    leftSideSnapHook();
  };
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

