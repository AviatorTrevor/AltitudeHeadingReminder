//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file
mainDepth = 50;
mainWidth = 85;
mainHeight = 40;
mainShellThickness = 5 * resolution;
screwCylinderRadius = 2.5;
screwHoleCylinderRadius = 0.8;
screwHoleCylinderOffsetFromFloor = mainHeight / 2;
cylinderFragments = 70;

margin = mainShellThickness + resolution;
lidThickness = resolution * 5; //z-axis
lidWidth = resolution * 2; //x & y axis thickness of the wall
innerLipHeight = resolution * 10;

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
};

