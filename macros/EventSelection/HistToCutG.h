/************************
Goal is to convert a track points into a cutg
To check if 2 tracks agree with each other
Only tested for tracks curving to the left
Will not work otherwise

But you garentte that the curve will neve goes flat (i.e. slope = 0), then it will work 
with tracks going in both direction
************************/
#ifndef HISTTOCUTG_H
#define HISTTOCUTG_H

struct Point{double x, y;};
enum class Direction{left, right, NoFlat};


double Normal2Pts(const Point& t_p1, const Point& t_p2);

Point PointSlopeExtend(double slope, const Point& t_pt, double t_offset, bool t_can_flat=false);

TCutG HistToCutG(TH2& t_hist, double swap_x = -200, Direction t_dir = Direction::left);

double PercentageInsideCut(const TCutG& t_cutg, const TGraph& t_graph);

#endif
