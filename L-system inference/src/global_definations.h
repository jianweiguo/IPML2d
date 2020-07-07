#ifndef GLOABL_DEFINE_H
#define GLOABL_DEFINE_H

// Qt
//#include <QString>
#include <QPointF>
#include "R2/R2.h"
#include <string>

std::string alphabet[8] = { "F", "S", "A", "B", "C", "D", "E", "H" };
std::string brachClassId[10] = { "b60", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "b9" };

static const double c1 = 0.35;
static const double c2 = 0.5;
static const double c3 = 1.0;

//my_color = { [1 1 0], [1 0 1], [0 1 1], [1 0 0], [0 1 0], [0 0 1], [0.6, 0.9, 0.1], ...
//[0.2, 0.55, 1.0], [0, 0, 0], [1.0, 0.0, 1.0], [42 / 255, 213 / 255, 170 / 255], [0.24, 0.28, 0.8], [1.0, 0.78, 0.05] };

QColor labelColors1[12] = {
	QColor::fromRgbF(c3, c2, c2),
	QColor::fromRgbF(c2, c3, c2),
	QColor::fromRgbF(c2, c2, c3),
	QColor::fromRgbF(c2, c3, c3),
	QColor::fromRgbF(c3, c2, c3),
	QColor::fromRgbF(c3, c3, c2),
	QColor::fromRgbF(c1, c2, c2),
	QColor::fromRgbF(c2, c1, c2),
	QColor::fromRgbF(c2, c2, c1),
	QColor::fromRgbF(c2, c1, c1),
	QColor::fromRgbF(c1, c2, c1),
	QColor::fromRgbF(c1, c1, c2)
};

QColor labelColors2[11] = {
	QColor::fromRgbF(0.54, 0.0, 0.54), //Qt::darkMagenta, 139,0,139
	QColor::fromRgbF(0.85, 0.44, 0.57), //Qt::PaleVioletRed, 219,112,147
	QColor::fromRgbF(0.0, 0.54, 0.54), //Qt::darkCyan,
	QColor::fromRgbF(0.5, 0.5, 1.0),
	QColor::fromRgbF(1.0, 0.78, 0.05),
	QColor::fromRgbF(0.0, 0.0, 1.0), //Qt::blue,
	QColor::fromRgbF(1, 0, 1), //Qt::magenta,
	QColor::fromRgbF(0.0, 1.0, 0.0), //Qt::green,
	QColor::fromRgbF(1, 0, 0), //Qt::red,
	QColor::fromRgbF(0.24, 0.28, 0.8),
	QColor::fromRgbF(0.2, 0.55, 1.0)
	/*QColor::fromRgbF(c2, c2, c1),
	QColor::fromRgbF(c2, c1, c1),
	QColor::fromRgbF(c1, c2, c1),
	QColor::fromRgbF(c1, c1, c2),
	QColor::fromRgbF(42 / 255, 213 / 255, 170 / 255),
	QColor::fromRgbF(1.0, 1.0, 0.0),
	QColor::fromRgbF(0.6, 0.9, 0.1),*/
};

QColor labelColors[16] = {
	QColor::fromRgbF(c2, c2, c3),
	Qt::red,
	Qt::green,
	Qt::blue,
	Qt::cyan,
	Qt::magenta,
	Qt::yellow,
	Qt::darkRed,
	Qt::darkGreen,
	Qt::darkBlue,
	Qt::darkCyan,
	Qt::darkMagenta,
	Qt::darkYellow,
	Qt::darkGray,
	Qt::lightGray,
	Qt::gray
};

QColor strahlerNumberColors[16] = {
	Qt::cyan,
	QColor::fromRgb(100,149,237), //Cornflower Blue
	QColor::fromRgb(244,164,96), //Sandy Brown
	QColor::fromRgb(60,179,113), //Medium Sea Green
	Qt::magenta,
	QColor::fromRgb(138,43,226), //Blue-violet
	Qt::red,
	Qt::green,
	Qt::blue,
	QColor::fromRgb(222,184,135), //Burlywood
	Qt::darkRed,
	Qt::darkGreen,
	Qt::darkCyan,
	Qt::darkMagenta,
	Qt::darkYellow,
	Qt::darkBlue
};

float strahlerNumberWidth[8] = { 
	1.0, 
	1.4, 
	1.8, 
	2.2, 
	2.6, 
	3.0, 
	3.4, 
	3.8 
};


inline QVector<QColor> rndColors(int count){
	QVector<QColor> colors;
	float currentHue = 0.0;
	for (int i = 0; i < count; i++){
		colors.push_back(QColor::fromHslF(currentHue, 1.0, 0.5));
		currentHue += 0.618033988749895f;
		currentHue = std::fmod(currentHue, 1.0f);
	}
	return colors;
}

//inline int randInt(int low, int high)
//{
//	// Random number between low and high
//	return qrand() % ((high + 1) - low) + low;
//}

inline QPointF my_rotate(QPointF s, double angleD){
	double angle = angleD*M_PI / 180.0;
	double xv = s.x()*cos(angle) + s.y()*sin(angle);
	double yv = -s.x()*sin(angle) + s.y()*cos(angle);
	QPointF t(xv, yv);
	return t;
}

inline QPointF my_rotate2(QPointF s, double angleD){
	//double angle = angleD*M_PI / 180.0;
	double xv = s.x()*cos(angleD) + s.y()*sin(angleD);
	double yv = -s.x()*sin(angleD) + s.y()*cos(angleD);
	QPointF t(xv, yv);
	return t;
}

inline QPointF my_transform(QPointF s, QPointF center, double scale, double angleD){
	double angle = angleD*M_PI / 180.0;
	double xv = s.x()*cos(angle) + s.y()*sin(angle);
	double yv = -s.x()*sin(angle) + s.y()*cos(angle);
	QPointF t(xv, yv);
	t *= scale;
	t += center;
	return t;
}

inline double angleFromY(R2Vector dir) // anticlockwise is positive
{
	dir.Normalize();
	float fAngle = acos(dir.Y());
	/*if (dir.X() < 0)
	{
	fAngle = 2 * M_PI - fAngle;
	}
	return -fAngle;*/

	if (dir.X() < 0)
	{
		fAngle = -fAngle;
	}
	return fAngle;
}

inline double angleFromY_LC(R2Vector dir) // anticlockwise is positive
{
	dir.Normalize();
	float fAngle = acos(dir.Y());
	/*if (dir.X() < 0)
	{
	fAngle = 2 * M_PI - fAngle;
	}
	return -fAngle;*/

	if (dir.X() > 0)
	{
		fAngle = -fAngle;
	}
	return fAngle;
}

inline bool is_in_imageSpace(QPointF point, double width, double height){
	if (point.x() < width && point.x() > 0 && point.y() < height && point.y() > 0){
		return true;
	}
	return false;
}
#endif