/************************
Goal is to convert a track points into a cutg
To check if 2 tracks agree with each other
Only tested for tracks curving to the left
Will not work otherwise

But you garentte that the curve will neve goes flat (i.e. slope = 0), then it will work 
with tracks going in both direction
************************/
#include "HistToCutG.hh"
#include <vector>
#include "TProfile.h"
#include "TH2.h"
#include "TAxis.h"

double Normal2Pts(const Point& t_p1, const Point& t_p2)
{
	double slope = (t_p2.y - t_p1.y)/(t_p2.x - t_p1.x);
	return -1./slope;
}

Point PointSlopeExtend(double slope, const Point& t_pt, double t_offset, bool t_can_flat)
{
	double angle = atan(slope);
	double x_offset = t_offset*cos(angle);
	double y_offset = t_offset*sin(angle);
	// this condition is needed to flip the side of the points when the curve is going flat
	if(((t_offset > 0 && y_offset < 0) || (t_offset < 0 && y_offset > 0)) && t_can_flat)
	{
		x_offset = - x_offset;
		y_offset = - y_offset;
	}
	return {t_pt.x + x_offset, t_pt.y + y_offset};
}

TCutG HistToCutG(TH2& t_hist, double swap_x, Direction t_dir)
{
	auto prof = t_hist.ProfileY();
	auto xaxis = prof->GetXaxis();
	auto nbinsx = xaxis->GetNbins();
	const double width = 20;
	int switch_index;
	std::vector<Point> points;
	for(int i = 0; i < nbinsx; ++i)
		if(fabs(prof->GetBinContent(i))>0.1)
		{
			if((t_dir == Direction::left) && (prof->GetBinContent(i) < swap_x)) break;
			if((t_dir == Direction::right) && (prof->GetBinContent(i) > swap_x)) break;
			points.push_back({prof->GetBinContent(i), xaxis->GetBinCenter(i)});
		}

	if(t_dir != Direction::NoFlat)
	{
		prof = t_hist.ProfileX();
		xaxis = prof->GetXaxis();
		nbinsx = xaxis->GetNbins();
		switch_index = points.size();
		if(t_dir == Direction::left)
			for(int i = nbinsx - 1; i >= 0; --i)
				if(xaxis->GetBinCenter(i) < swap_x && prof->GetBinContent(i) > 0)
					points.push_back({xaxis->GetBinCenter(i), prof->GetBinContent(i)});
		if(t_dir == Direction::right)
			for(int i = 0; i < nbinsx; ++i)
				if(xaxis->GetBinCenter(i) > swap_x && prof->GetBinContent(i) > 0)
					points.push_back({xaxis->GetBinCenter(i), prof->GetBinContent(i)});
	}
			
	

	TCutG cutg;
	for(int i = 1; i < points.size() - 1; ++i)
	{
		auto new_pt = PointSlopeExtend(Normal2Pts(points[i-1], points[i + 1]), points[i], width, (i > switch_index));
		cutg.SetPoint(cutg.GetN(), new_pt.x, new_pt.y);
	}
	auto num = cutg.GetN();
	for(int i = points.size() - 1; i > 0; --i)
	{
		auto new_pt = PointSlopeExtend(Normal2Pts(points[i-1], points[i + 1]), points[i], -width, (i > switch_index));
		cutg.SetPoint(cutg.GetN(), new_pt.x, new_pt.y);
	}
	double firstx, firsty;
	cutg.GetPoint(0, firstx, firsty);
	cutg.SetPoint(cutg.GetN(), firstx, firsty);

	return cutg;
	// will draw cut with methods layed in https://stackoverflow.com/questions/42165631/in-matplotlib-how-can-i-plot-a-multi-colored-line-like-a-rainbow/42190453#42190453
	// normal of the other 2 points
	
}

double PercentageInsideCut(const TCutG& t_cutg, const TGraph& t_graph)
{
	auto size = t_graph.GetN();
	int num_inside = 0;
	for(int i = 0; i < size; ++i)
	{
		double x, y;
		t_graph.GetPoint(i, x, y);
		if(t_cutg.IsInside(x, y)) ++num_inside;
	}
	return (double) num_inside / (double) size;
}
