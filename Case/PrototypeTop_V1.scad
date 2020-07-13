//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file
mainDepth = 80;
mainWidth = 140;
mainHeight = 42;
mainShellThickness = 4.9;

lidThickness = resolution * 5; //z-axis
lidWidth = resolution * 2; //x & y axis thickness of the wall

difference() {
  union() {
    //top cover of the lid
    cube([mainDepth, mainWidth, lidThickness]);
    
    //the inner lip
    translate([mainShellThickness, mainShellThickness, 0]) {
      cube([mainDepth - mainShellThickness*2, mainWidth - mainShellThickness*2, lidThickness*3]);
    };
  };
  
  //cutting out the inside to save on plastic used
  translate([mainShellThickness + lidWidth, mainShellThickness + lidWidth, lidThickness]) {
    cube([mainDepth - mainShellThickness*2 - lidWidth*2, mainWidth - mainShellThickness*2 - lidWidth*2, lidThickness*2]);
  }
};