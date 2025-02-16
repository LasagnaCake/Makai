#include <ctl/ctl.hpp>
#include <ctlex/ctlex.hpp>

using namespace CTL::Ex::Collision;
using namespace C2D;


int main() {
	DEBUGLN("<colliding>");
	if(withinBounds(Point(0), Point(0)))			DEBUGLN("    Point-to-Point OK!");
	if(withinBounds(0, Circle(0, 1)))				DEBUGLN("    Point-to-Circle OK!");
	if(withinBounds(Circle(0, 1), Circle(0, 1)))	DEBUGLN("    Circle-to-Circle OK!");
	if(withinBounds(Circle(0, 1), Box(0, 1)))		DEBUGLN("    Circle-to-Box OK!");
	if(withinBounds(Box(0, 1), Box(0, 1)))			DEBUGLN("    Box-to-Box OK!");
	DEBUGLN("</colliding>");
	DEBUGLN("<not-colliding>");
	if(!withinBounds(Point(-1), Point(1)))			DEBUGLN("    Point-to-Point OK!");
	if(!withinBounds(Point(-1), Circle(1, 1)))		DEBUGLN("    Point-to-Circle OK!");
	if(!withinBounds(Circle(-1, 1), Circle(1, 1)))	DEBUGLN("    Circle-to-Circle OK!");
	if(!withinBounds(Circle(-1, 1), Box(1, 1)))		DEBUGLN("    Circle-to-Box OK!");
	if(!withinBounds(Box(-2, 1), Box(2, 1)))		DEBUGLN("    Box-to-Box OK!");
	DEBUGLN("</not-colliding>");
}