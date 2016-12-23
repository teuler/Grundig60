include <threads.scad>

b_xrim      = 3;
b_yrim      = 3;
b_dx        = 55;
b_dy        = 72;
b_thickness = 1.5;
b_shift     = 0;

scr_dz      = 6;
scr_dia     = 5;
scr_hole_d  = 3.0;
scr_hole2_d = 5;
pos_scr     = [[b_dx-3-b_xrim ,18+51-b_yrim,0],
               [b_dx-20,10,0],
               [b_dx-29-20,18-3,0],
               [13-b_xrim,b_dy-13-b_yrim,0]];

pos_b       = [-b_dx/2,-b_dy/2, 0];
pos_scr_off = [-b_dx/2,-b_dy/2, 0];


module board() {
 /* difference() {
        union() {
            for(pos=pos_scr) {
                rotate(0, [1,0,0]) translate(pos)
                translate(pos_scr_off)
                translate([scr_dia/2,scr_dia/2,h_from_tube -b_thickness*0.8])
                cylinder($fn=100, h=scr_dz+b_thickness/2, r=scr_dia/2);
            }
            translate(pos_b) 
            cube([b_dx, b_dy, b_thickness]);
        }
        union() {
            for(pos=pos_scr) {
                rotate(0, [1,0,0]) translate(pos) 
                translate(pos_scr_off)
                translate([scr_dia/2,scr_dia/2,-10])
                cylinder($fn=100, h=scr_dz*3, r=scr_hole2_d/2);
            }
            for(pos=pos_scr) {
                rotate(0, [1,0,0]) translate(pos) 
                translate(pos_scr_off)
                translate([scr_dia/2,scr_dia/2,h_from_tube -b_thickness*2])
                cylinder($fn=100, h=scr_dz*2, r=scr_hole_d/2);
            }
        }*/
    
    for(pos=pos_scr) {
        rotate(0, [1,0,0]) translate(pos)
        translate(pos_scr_off)
        translate([scr_dia/2,scr_dia/2,0.25])
        cylinder($fn=100, h=scr_dz+b_thickness, r=scr_dia/2);
    }
    translate(pos_b) 
    cube([b_dx, b_dy, b_thickness]);
}

module threads() {
    for(pos=pos_scr) {
        rotate(0, [1,0,0]) translate(pos) 
        translate(pos_scr_off)
        translate([scr_dia/2,scr_dia/2,-3])
        //cylinder($fn=100, h=scr_dz*2, r=scr_hole_d/2);
        metric_thread(diameter=2.5, pitch=0.45, length=scr_dz+b_thickness+5+10, internal=true);
    }
}

module holes() {
    rotate(0, [1,0,0])
    translate(pos_b)
    translate([b_dx-4,11,-10])
    cylinder($fn=100, h=20, r=4/2);

    rotate(0, [1,0,0])
    translate(pos_b)
    translate([25,27,-10])
    cylinder($fn=100, h=20, r=3/2);

    rotate(0, [1,0,0])
    translate(pos_b)
    translate([b_dx-13,46,-10])
    cylinder($fn=100, h=20, r=3/2);
    
    rotate(0, [1,0,0])
    translate(pos_b)
    translate([28-1.5,2,-10])
    cylinder($fn=100, h=20, r=4/2);

    translate(pos_b) 
    translate([b_dx+b_xrim-30,15,-5]) 
    cube([30, 30, 10]);

    translate(pos_b) 
    translate([-1,-1,-5]) 
    cube([7, 26, 10]);
}

module base() {
    difference() {
        board();
        union() {
            threads();    
            holes();
        }
    }
}

base();





/*
d_wall		= 1.25;
tol			= 0.2;

fr_dx		= 65 +2*d_wall;
fr_dy		= 140; //120 +d_wall;
fr_height  = 30;

lt_dx      = 20;

scr1_dia   = 7;
scr1_ctr_x = -8;
scr1_ctr_dy= 17;

scr2_dia   = 6;
scr_dy_wall= 25;
scr_dx_left= 15;
scr2_pos   = [[scr_dx_left+15,scr_dy_wall+2,d_wall/2], 
			 [scr_dx_left+42.5,scr_dy_wall+2,d_wall/2], 
              [scr_dx_left+15,scr_dy_wall+59,d_wall/2], 
              [scr_dx_left+42.5,scr_dy_wall+59,d_wall/2]];
scr2_height= 6;
scr2_h_dia = 3.0;
scr3_dia   = scr2_dia;
scr3_pos   = [[4,   110,   d_wall/2],
			 [4,   110+21,d_wall/2],			 
			 [4+57,110,   d_wall/2],
			 [4+57,110+21,d_wall/2]];
scr3_height= 11;
scr3_h_dia = scr2_h_dia;


led_dia    = 3;
led_pos    = [[1*fr_dx/6,scr1_ctr_dy/2+fr_height/2, -10],
   			 [3*fr_dx/6,scr1_ctr_dy/2+fr_height/2, -10],
			 [5*fr_dx/6,scr1_ctr_dy/2+fr_height/2, -10]];

bnc_dia    = 10;
bnc_pos    = [[1*fr_dx/6,fr_height/3, -10],
//			 [fr_dx/2,fr_height/3*2, -10-fr_dy]
			 [3*fr_dx/6,fr_height/3, -10]];

module Screw1Holes()
{
	_r		= scr1_dia/2 +tol;
	_x		= scr1_ctr_x;
	_y1		= fr_height/2 +scr1_ctr_dy/2;
	_y2		= fr_height/2 -scr1_ctr_dy/2;
	rotate(90, [1,0,0]) translate([_x,_y1,-10]) 
	cylinder($fn=100, h=20, r=_r);
	rotate(90, [1,0,0]) translate([_x,_y2,-10]) 
	cylinder($fn=100, h=20, r=_r);
};


module LEDHoles()
{
	_r		= led_dia/2 +tol/2;
    for(pos=led_pos) { 
		rotate(90, [1,0,0]) translate(pos) 
		cylinder($fn=100, h=9.5, r=_r);
	}
};

module LEDPosts()
{
	_r		= led_dia/2 +tol/2;
    for(pos=led_pos) { 
		rotate(90, [1,0,0]) translate(pos) translate([0,0,4])
		cylinder($fn=100, h=3+d_wall*2, r=_r+2);
	}
};


module BNCHoles()
{
	_r		= bnc_dia/2 +tol/2;
    for(pos=bnc_pos) { 
		difference() {
			rotate(90, [1,0,0]) translate(pos) 
			cylinder($fn=100, h=20, r=_r);
			rotate(90, [1,0,0]) translate([pos[0]-_r,pos[1]+4.8*_r/6,pos[2]])
			cube([2*_r,5,25]);
		}	
	}
};

module cableHoles()
{
	_x		= fr_dx/3*2;
	_y 		= fr_dx/7;
	rotate(90, [1,0,0]) translate([_x,_y,-fr_dy-10]) 
	cylinder($fn=100, h=20, r=7.5);
}

module FrontPlateHoles()
{
	LEDHoles();
	BNCHoles();
}

module BaseplateHoles()
{
    _dx   = fr_dx/4;
    _dy   = fr_dx/4;
    _f    = 1.4;
    _d    = 0.10*_dx;
    _z    = -5;
    _dyst = 1.65;
    _yoff = 0.3;
    for(i=[0,1.05,2,3.05]) {
	    translate([0.50*_dx-_d,(_yoff+_dyst*i)*_dx,_z]) cube([_dx*_f,_dy,10]);
    		translate([2.25*_dx-_d,(_yoff+_dyst*i)*_dx,_z]) cube([_dx*_f,_dy,10]);   
    }
}

module RC_Box()
{
	difference() 
	{
		difference()
		{
			cube([fr_dx, fr_dy, fr_height]);
			translate([d_wall,d_wall*2,d_wall]) cube([fr_dx-2*d_wall,fr_dy-3*d_wall,fr_height]);
		}
    		BaseplateHoles();
	}

    difference()
    {
		translate([-lt_dx,0,0]) cube([lt_dx, d_wall*2, fr_height]);
		Screw1Holes();
    }

	rotate(-90, [0,0,1])
    for(pos=scr2_pos) { 
		rotate(90, [0,0,1]) translate(pos) 
		difference() {
			cylinder($fn=100, h=scr2_height, r=scr2_dia/2);
			cylinder($fn=100, h=scr2_height+5, r=scr2_h_dia/2);
		}
	}
	rotate(-90, [0,0,1])
    for(pos=scr3_pos) { 
		rotate(90, [0,0,1]) translate(pos) 
		difference() {
			cylinder($fn=100, h=scr3_height, r=scr3_dia/2);
			cylinder($fn=100, h=scr3_height+5, r=scr3_h_dia/2);
		}
	}
	LEDPosts();
}

difference()
{
	RC_Box();
	FrontPlateHoles();
	cableHoles();
}
*/

