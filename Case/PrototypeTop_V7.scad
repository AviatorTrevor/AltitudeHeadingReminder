//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file
mainShellThickness = 5 * resolution;
mainDepth = 57;
mainWidth = 91;
mainHeight = 37 + mainShellThickness;
screwCylinderRadius = 2.5;
screwHoleCylinderRadius = 1.1;
screwHoleCylinderOffsetFromFloor = mainHeight / 2;
cylinderFragments = 70;

margin = mainShellThickness + resolution;
lidThickness = resolution * 5; //z-axis
lidWidth = resolution * 2; //x & y axis thickness of the wall
innerLipHeight = resolution * 5;

buzzerRadius = 30.4 / 2; //30.4 is diameter
buzzerScrewOffset = 20.5; //20.5 mm from center of buzzer hole to center of screw hole
buzzerScrewRadius = 1.9;
buzzerScrewHeadRadius = 3.5;
buzzerX = mainDepth - buzzerRadius * 1.4;
buzzerY = mainWidth / 2;

screwHeadCylinderRadius = 1.8;
screwHeadSinkDepth = lidThickness * 0.6;

difference() {
  union() {
    //top cover of the lid
    cube([mainDepth, mainWidth, lidThickness]);
    
    //the inner lip
    translate([margin, margin, lidThickness]) {
      cube([mainDepth - margin*2, mainWidth - margin*2, innerLipHeight]);
    };
  };
  
  //cutting out the inside to save on plastic used
  translate([margin + lidWidth, margin + lidWidth, lidThickness]) {
    cube([mainDepth - margin*2 - lidWidth*2, mainWidth - margin*2 - lidWidth*2, innerLipHeight]);
  };
  
  
  
  //cutting away room at the corner of the lip to make room for the cylinders where screws go.
  //rear-left
  translate([0, 0, lidThickness]) {
    cube([screwCylinderRadius*2 + resolution, screwCylinderRadius*2 + resolution, innerLipHeight]);
  };
  translate([0, 0, 0]) {
    cube([screwCylinderRadius, screwCylinderRadius, lidThickness]);
  };
  translate([screwCylinderRadius - screwHoleCylinderRadius, screwCylinderRadius - screwHoleCylinderRadius, 0]) {
    cube([screwHoleCylinderRadius*2, screwHoleCylinderRadius*2, lidThickness]);
  };
  
  //cutting away room at the corner of the lip to make room for the cylinders where screws go.
  //front-left
  translate([mainDepth - screwCylinderRadius*2 - resolution, 0, lidThickness]) {
    cube([screwCylinderRadius*2 + resolution, screwCylinderRadius*2 + resolution, innerLipHeight]);
  };
  translate([mainDepth - screwCylinderRadius, 0, 0]) {
    cube([screwCylinderRadius, screwCylinderRadius, lidThickness]);
  };
  translate([mainDepth - screwCylinderRadius - screwHoleCylinderRadius, screwCylinderRadius - screwHoleCylinderRadius, 0]) {
    cube([screwHoleCylinderRadius*2, screwHoleCylinderRadius*2, lidThickness]);
  };
  
  //cutting away room at the corner of the lip to make room for the cylinders where screws go.
  //front-right
  translate([mainDepth - screwCylinderRadius*2 - resolution, mainWidth - screwCylinderRadius*2 - resolution, lidThickness]) {
    cube([screwCylinderRadius*2 + resolution, screwCylinderRadius*2 + resolution, innerLipHeight]);
  };
  translate([mainDepth - screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
    cube([screwCylinderRadius, screwCylinderRadius, lidThickness]);
  };
  translate([mainDepth - screwCylinderRadius - screwHoleCylinderRadius, mainWidth - screwCylinderRadius - screwHoleCylinderRadius, 0]) {
    cube([screwHoleCylinderRadius*2, screwHoleCylinderRadius*2, lidThickness]);
  };
  
  //cutting away room at the corner of the lip to make room for the cylinders where screws go.
  //rear-right
  translate([0, mainWidth - screwCylinderRadius*2 - resolution, lidThickness]) {
    cube([screwCylinderRadius*2 + resolution, screwCylinderRadius*2 + resolution, innerLipHeight]);
  };
  translate([0, mainWidth - screwCylinderRadius, 0]) {
    cube([screwCylinderRadius, screwCylinderRadius, lidThickness]);
  };
  translate([screwCylinderRadius - screwHoleCylinderRadius, mainWidth - screwCylinderRadius - screwHoleCylinderRadius, 0]) {
    cube([screwHoleCylinderRadius*2, screwHoleCylinderRadius*2, lidThickness]);
  };
  
  //buzzer alarm hole
  translate([buzzerX, buzzerY, 0]) {
    cylinder(lidThickness, buzzerRadius, buzzerRadius, $fn=cylinderFragments);
  };
  
  //buzzer left-screw
  translate([buzzerX, buzzerY - buzzerScrewOffset, 0]) {
    cylinder(lidThickness, buzzerScrewRadius, buzzerScrewRadius, $fn=cylinderFragments);
  };
  /*TODO for stronger material:
  translate([buzzerX, buzzerY - buzzerScrewOffset, 0]) {
    cylinder(screwHeadSinkDepth, buzzerScrewHeadRadius, buzzerScrewHeadRadius, $fn=cylinderFragments);
  };*/
  
  //buzzer right-screw
  translate([buzzerX, buzzerY + buzzerScrewOffset, 0]) {
    cylinder(lidThickness, buzzerScrewRadius, buzzerScrewRadius, $fn=cylinderFragments);
  };
  /*TODO for stronger material:
  translate([buzzerX, buzzerY + buzzerScrewOffset, 0]) {
    cylinder(screwHeadSinkDepth, buzzerScrewHeadRadius, buzzerScrewHeadRadius, $fn=cylinderFragments);
  };*/
  
  
  //place for screw to sink into, rear-left (keep this in sync with section below)
  /*TODO for stronger material:
  translate([screwCylinderRadius, screwCylinderRadius, 0]) {
    cylinder(screwHeadSinkDepth, screwHeadCylinderRadius, screwHeadCylinderRadius, $fn=cylinderFragments);
  };*/
  
  //place for screw to sink into, front-left (keep this in sync with section below)
  /*TODO for stronger material:
  translate([mainDepth - screwCylinderRadius, screwCylinderRadius, 0]) {
    cylinder(screwHeadSinkDepth, screwHeadCylinderRadius, screwHeadCylinderRadius, $fn=cylinderFragments);
  };*/
  
  //place for screw to sink into, front-right (keep this in sync with section below)
  /*TODO for stronger material:
  translate([mainDepth - screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
    cylinder(screwHeadSinkDepth, screwHeadCylinderRadius, screwHeadCylinderRadius, $fn=cylinderFragments);
  };*/
  
  //place for screw to sink into, rear-right (keep this in sync with section below)
  /*TODO for stronger material:
  translate([screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
    cylinder(screwHeadSinkDepth, screwHeadCylinderRadius, screwHeadCylinderRadius, $fn=cylinderFragments);
  };*/
  
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
};



difference() {
  //add the curve to the rear-left
  translate([screwCylinderRadius, screwCylinderRadius, 0]) {
    cylinder(lidThickness, screwCylinderRadius, screwCylinderRadius, $fn=cylinderFragments);
  };
  //screw hole, rear-left
  translate([screwCylinderRadius, screwCylinderRadius, 0]) {
    cylinder(lidThickness, screwHoleCylinderRadius, screwHoleCylinderRadius, $fn=cylinderFragments);
  };
  //place for screw to sink into, rear-left (keep this in sync with section above)
  /*TODO later for non-3D-printed version:
  translate([screwCylinderRadius, screwCylinderRadius, 0]) {
    cylinder(screwHeadSinkDepth, screwHeadCylinderRadius, screwHeadCylinderRadius, $fn=cylinderFragments);
  };*/
};


difference() { 
  //add the curve to the front-left
  translate([mainDepth - screwCylinderRadius, screwCylinderRadius, 0]) {
    cylinder(lidThickness, screwCylinderRadius, screwCylinderRadius, $fn=cylinderFragments);
  };
  //screw hole, front-left
  translate([mainDepth - screwCylinderRadius, screwCylinderRadius, 0]) {
    cylinder(lidThickness, screwHoleCylinderRadius, screwHoleCylinderRadius, $fn=cylinderFragments);
  };
  //place for screw to sink into, front-left (keep this in sync with section above)
  /*TODO later for non-3D-printed version:
  translate([mainDepth - screwCylinderRadius, screwCylinderRadius, 0]) {
    cylinder(screwHeadSinkDepth, screwHeadCylinderRadius, screwHeadCylinderRadius, $fn=cylinderFragments);
  };*/
};
  

difference() {
  //add the curve to the front-right
  translate([mainDepth - screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
    cylinder(lidThickness, screwCylinderRadius, screwCylinderRadius, $fn=cylinderFragments);
  };
  //screw hole, front-right
  translate([mainDepth - screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
    cylinder(lidThickness, screwHoleCylinderRadius, screwHoleCylinderRadius, $fn=cylinderFragments);
  };
  //place for screw to sink into, front-right (keep this in sync with section above)
  /*TODO later for non-3D-printed version:
  translate([mainDepth - screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
    cylinder(screwHeadSinkDepth, screwHeadCylinderRadius, screwHeadCylinderRadius, $fn=cylinderFragments);
  };*/
};

difference() {
  //add the curve to the rear-right
  translate([screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
    cylinder(lidThickness, screwCylinderRadius, screwCylinderRadius, $fn=cylinderFragments);
  };
  //screw hole, rear-right
  translate([screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
    cylinder(lidThickness, screwHoleCylinderRadius, screwHoleCylinderRadius, $fn=cylinderFragments);
  };
  //place for screw to sink into, rear-right (keep this in sync with section above)
  /*TODO later for non-3D-printed version:
  translate([screwCylinderRadius, mainWidth - screwCylinderRadius, 0]) {
    cylinder(screwHeadSinkDepth, screwHeadCylinderRadius, screwHeadCylinderRadius, $fn=cylinderFragments);
  };*/
};

