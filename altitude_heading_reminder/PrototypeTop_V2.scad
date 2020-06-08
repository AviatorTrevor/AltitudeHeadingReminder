//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file
mainDepth = 90;
mainWidth = 140;
mainHeight = 42;
mainShellThickness = 5 * resolution;

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
  }
};