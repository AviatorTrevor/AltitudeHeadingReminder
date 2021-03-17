
//everything is in units of millimeters
resolution = 0.35;

//variables to keep in-sync with the other scad file
mainShellThickness = 5 * resolution;
mainDepth = 57;
mainWidth = 91;
mainHeight = 37 + mainShellThickness;
caseCornerRadius = 2.5;
cylinderFragments = 70;

//keep in-sync with the main case file
mountingHoleRadius = 4.32;
mountingPillarRadius = mountingHoleRadius + 1.5;
mountingHoleHeight = 8.68;
mountingPillarHeight = mountingHoleHeight + 0.5;

plateDepth = mainDepth / 1.5;
plateWidth = mainWidth / 1.5;
plateThickness = mainShellThickness + 0.5;
plateHoleRadius = mountingHoleRadius + resolution / 2;
plateMountingHoleX = plateDepth - plateThickness - mountingHoleRadius - 3.3; //keep aligned with mountingHoleX variable

difference() {
    cube([plateDepth, plateWidth, plateThickness]);
    
    //bottom mounting hole
    translate([plateMountingHoleX, plateWidth/2, 0]) {
      cylinder(plateThickness, plateHoleRadius, plateHoleRadius, $fn=cylinderFragments);
    };
};