
//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file
pcbBoardHeight = 36.9;
pcbBoardWidth = 75.7;
innerLipHeightAboveOuterLipHeight = resolution * 5;
mainShellThickness = 5 * resolution;
mainDepth = 60;
mainWidth = 84.7;
mainHeight = pcbBoardHeight + mainShellThickness + innerLipHeightAboveOuterLipHeight;
caseCornerRadius = 2.5;
cylinderFragments = 70;

//keep in-sync with the main case file
mountingHoleRadius = 4.5;
mountingQuarterInchRadius = 3.5;
mountingPillarRadius = mountingHoleRadius + 2.5;
mountingHoleHeight = 9.8;
mountingTransitionHeight = 4;
mountingQuarterInchCylinderHeight = mainHeight - mountingHoleHeight - mountingTransitionHeight - 16;
mountingPillarHeight = mainHeight - 14.5;

plateDepth = mainDepth * 0.9;
plateWidth = mainWidth * 0.9;
plateThickness = resolution * 8;
plateHoleRadius = mountingHoleRadius + resolution / 2;
plateMountingHoleX = plateDepth - mountingPillarRadius - 3;

difference() {
  union() {
    cube([plateDepth, plateWidth, plateThickness]);
    
    translate([plateMountingHoleX, plateWidth/2, 0]) {
      cylinder(mountingHoleHeight, mountingPillarRadius, mountingPillarRadius, $fn=cylinderFragments);
    };
  };
  
    
  //bottom mounting hole
  translate([plateMountingHoleX, plateWidth/2, 0]) {
    cylinder(mountingHoleHeight, plateHoleRadius, plateHoleRadius, $fn=cylinderFragments);
  };
};
