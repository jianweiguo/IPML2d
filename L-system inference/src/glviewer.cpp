// Qt
//#include <QtGui>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <QGlobal.h>
#include <QTime>
#include <limits>
//#include <QPointF>
//#include <QPolygonF>

#include"math.h"
// local
#include "glviewer.h"
#include "window.h"
#include "patterns.h"
#include "global_definations.h"
#include "PerlinNoise.hpp"

GlViewer::GlViewer(QWidget *pParent) : QGLWidget(QGLFormat(QGL::SampleBuffers), pParent)
{
    m_scene = NULL;
   
	m_show_tree = true;
	m_show_tree_render = false;
	m_render = false;
	m_image_id = 0;
	m_show_image = false;
	m_showSampledPoints = false;

    m_line_thickness = 2.0;
	m_rotate_angle = 0.0;
	scale_factor_ = 14.0;

    //MainWindow::getInstance()->doubleSpinBox_pointSize->setValue(m_point_size);
    m_scale = 1.0;
    m_center_x =  0.0;
    m_center_y =  0.0;
	
	m_bbxID = -1;
	m_weight_thrs = 1.0;
	m_centerDis_thrs = 1.0;
	m_cornerDis_thrs = 0.1;
	m_mianBranchAngle_thrs = 5.0;

	m_optAlgorithm_ = "Our";

	m_background = QBrush(QColor(255, 255, 255));
    setAutoFillBackground(false);
}

void GlViewer::resizeGL(int width, int height) 
{
    glViewport(0, 0, width, height);
    double aspect_ratio = double(height) / double(width);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -aspect_ratio, aspect_ratio, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GlViewer::initializeGL() 
{
    glClearColor(1., 1., 1., 0.);
	glEnable(GL_MULTISAMPLE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SMOOTH);
}

void GlViewer::paintEvent(QPaintEvent *event)
{
    makeCurrent();
	if (!m_scene) return;
	//paintGL();
	
	glClear(GL_COLOR_BUFFER_BIT);

	QPainter painter; 
	painter.begin(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	//painter.fillRect(event->rect(), m_background);
	//painter.setBackground(Qt::white);

	QPointF BL(0, 0);
	QPointF TR(width(), height());
	QPointF widget_center(width()/2, height()/2);
	QPointF Origin(width() / 2, height());

	vector<t_line> tree2d = m_scene->m_tree2d;

	//vector<Bounding_box> rects = m_scene->m_bbx;

	QVector<QColor> colors = rndColors(10);

	float width_factor = 1.1;

	//following parameters are used for add noise on the line segments
	double frequency = 8.0;
	int octaves = 6;
	std::uint32_t seed = 11447;
	//double noise_scale = 7.0;
	double noise_scale = 1.0;
	double noise_base = 0.25;
	const siv::PerlinNoise perlin(seed);

	//here we show the static tree
	if (m_show_tree){
		//double scale_factor = (width() / m_scene->m_tree_maxlen)*0.75;
		double scale_factor = m_scene->m_tree_scale_factor;

		for (int i = 0; i < tree2d.size(); i++){
			t_line line = tree2d[i];
			int gid = line.group_id;
			QPointF s(line.p_start.X(), -line.p_start.Y());
			QPointF t(line.p_end.X(), -line.p_end.Y());
			s -= m_scene->m_tree_center;
			t -= m_scene->m_tree_center;

			painter.save();
			int cdx = i % 10;
			float w;
			if (line.strahler_number == 0) {
				w = line.width;
				//painter.setPen(QPen(colors[cdx], m_line_thickness*w));
				painter.setPen(QPen(Qt::black, m_line_thickness*w));
			}
			else {
				w = width_factor*(line.strahler_number + 1);
				painter.setPen(QPen(strahlerNumberColors[line.strahler_number], m_line_thickness*w));
			}

			//painter.translate(widget_center.x(), widget_center.y());
			//painter.scale(scale_factor, scale_factor);
			s = my_transform(s, widget_center, scale_factor, 0.0);
			t = my_transform(t, widget_center, scale_factor, 0.0);
			painter.drawLine(s, t);

			if (m_showSampledPoints){
				double line_len_input = (line.p_start - line.p_end).Length();
				double line_len = (line.p_start - line.p_end).Length()*scale_factor;
				int sampled_points = std::max(4, int(line_len / 2.0));
				//int sampled_points = 6;
				double step = line_len_input / (sampled_points - 1);
				QPointF last_point;
				
				//const siv::PerlinNoise perlin(seed);
				const double fx = sampled_points / frequency;

				for (int j = 0; j < sampled_points; j++){
					R2Vector new_point = line.p_start + line.direction*(step*j);

					R2Vector new_point_noise;
					if (j == 0 || j == sampled_points - 1){
						new_point_noise = new_point;
					}
					else{
						double angle_noise = (rand() % (15 * 2)) - 15;
						QPointF rotate_dir = my_rotate(QPointF(line.direction.X(), line.direction.Y()), 90 + angle_noise);
						R2Vector orthogonal_dir(rotate_dir.x(), rotate_dir.y());
						orthogonal_dir.Normalize();
						//double noise_scale = 7.0*step;
						//double rand_val = (randInt(0, 4) / 2.0 - 1.0)*noise_scale;
						double pp = perlin.octaveNoise0_1(new_point_noise.X(), new_point_noise.Y(), octaves);
						//double pp = perlin.octaveNoise0_1(j/fx, octaves);
						double rand_val = (pp - noise_base)*noise_scale*step;
						new_point_noise = new_point + rand_val*orthogonal_dir;
					}
					
					QPointF np(new_point_noise.X(), -new_point_noise.Y());
					np -= m_scene->m_tree_center;
					np = my_transform(np, widget_center, scale_factor, 0.0);

					//painter.setPen(QPen(QColor::fromRgb(221, 119, 119), 1));
					//painter.setBrush(QBrush(QColor::fromRgb(221, 119, 119)));
					//painter.drawEllipse(np, 2, 2);

					if (j != 0){
						painter.drawLine(last_point, np);
					}
					last_point = np;
				}
			}

			painter.restore();
		}
	}

	//now we can show the transformed tree and save it to JPG/PNG file
	if (m_show_tree_render){
		//double scale_factor = (width() / m_scene->m_tree_maxlen)*0.75;

		for (int i = 0; i < tree2d.size(); i++){
			t_line line = tree2d[i];
			QPointF s(line.p_start.X(), -line.p_start.Y());
			QPointF t(line.p_end.X(), -line.p_end.Y());
			s -= m_scene->m_tree_center;
			t -= m_scene->m_tree_center;

			s = my_transform(s, widget_center, scale_factor_, m_rotate_angle);
			t = my_transform(t, widget_center, scale_factor_, m_rotate_angle);

			painter.save();
			float w;
			if (line.strahler_number == 0) {
				w = line.width;
				//painter.setPen(QPen(colors[cdx], m_line_thickness*w));
				painter.setPen(QPen(Qt::black, m_line_thickness*w));
			}
			else {
				w = width_factor*(line.strahler_number + 1);
				painter.setPen(QPen(strahlerNumberColors[line.strahler_number], m_line_thickness*w));
			}
			/*painter.translate(widget_center.x(), widget_center.y());
			painter.scale(scale_factor, scale_factor);
			painter.rotate(m_rotate_angle);*/

			painter.drawLine(s, t);

			if (m_showSampledPoints){
				double line_len_input = (line.p_start - line.p_end).Length();
				double line_len = (line.p_start - line.p_end).Length()*scale_factor_;
				int sampled_points = std::max(4, int(line_len / 2.0));
				//int sampled_points = 6;
				double step = line_len_input / (sampled_points - 1);
				QPointF last_point;

				//const siv::PerlinNoise perlin(seed);
				const double fx = sampled_points / frequency;

				for (int j = 0; j < sampled_points; j++){
					R2Vector new_point = line.p_start + line.direction*(step*j);

					R2Vector new_point_noise;
					if (j == 0 || j == sampled_points - 1){
						new_point_noise = new_point;
					}
					else{
						double angle_noise = (rand() % (15 * 2)) - 15;
						QPointF rotate_dir = my_rotate(QPointF(line.direction.X(), line.direction.Y()), 90 + angle_noise);
						R2Vector orthogonal_dir(rotate_dir.x(), rotate_dir.y());
						orthogonal_dir.Normalize();
						//double noise_scale = 7.0*step;
						//double rand_val = (randInt(0, 4) / 2.0 - 1.0)*noise_scale;
						double pp = perlin.octaveNoise0_1(new_point_noise.X(), new_point_noise.Y(), octaves);
						//double pp = perlin.octaveNoise0_1(j/fx, octaves);
						double rand_val = (pp - noise_base)*noise_scale*step;
						new_point_noise = new_point + rand_val*orthogonal_dir;
					}

					QPointF np(new_point_noise.X(), -new_point_noise.Y());
					np -= m_scene->m_tree_center;
					np = my_transform(np, widget_center, scale_factor_, m_rotate_angle);

					if (j != 0){
						painter.drawLine(last_point, np);
					}
					last_point = np;
				}
			}
			painter.restore();
		}

		if (m_render){
			QImage image(width(), height(), QImage::Format_RGB32);
			image.fill(Qt::white);
			QPainter painter2;
			painter2.begin(&image);
			painter2.setRenderHint(QPainter::Antialiasing, true);

			m_bbx.clear();
			int nb_groups = m_scene->number_groups;
			//int nb_groups = tree2d.size();
			for (int i = 0; i < nb_groups; i++){
				Bounding_box bbx;
				bbx.group_id = i;
				m_bbx.push_back(bbx);
			}

			for (int i = 0; i < tree2d.size(); i++){
				t_line line = tree2d[i];
				QPointF s(line.p_start.X(), -line.p_start.Y()); //we reverse the y value to make the coordinate system same to the QWidget
				QPointF t(line.p_end.X(), -line.p_end.Y());

				//we get the angle between the group's direction and the Y-axis
				//R2Vector dir = line.direction;
				R2Vector dir(line.direction.X(), -line.direction.Y());

				int gid = line.group_id;
				//int gid = i;
				if (gid == 1){
					int aaa = 10;
				}
				if (!m_bbx[gid].set_angle){
					m_bbx[gid].angleFromY = angleFromY(dir);
					m_bbx[gid].set_angle = true;
				}


				m_bbx[gid].class_id = line.class_id;
				m_bbx[gid].branch_status = line.branch_status;
				m_bbx[gid].iteration = line.iteration;

				//Rotate each line of this group to make the group's bbx align with positive Y-axis
				s = my_rotate2(s, -m_bbx[gid].angleFromY);
				t = my_rotate2(t, -m_bbx[gid].angleFromY);

				if (s.x() > m_bbx[gid].x_max) m_bbx[gid].x_max = s.x();
				if (s.x() < m_bbx[gid].x_min) m_bbx[gid].x_min = s.x();
				if (s.y() > m_bbx[gid].y_max) m_bbx[gid].y_max = s.y(); 
				if (s.y() < m_bbx[gid].y_min) m_bbx[gid].y_min = s.y();

				if (t.x() > m_bbx[gid].x_max) m_bbx[gid].x_max = t.x();
				if (t.x() < m_bbx[gid].x_min) m_bbx[gid].x_min = t.x();
				if (t.y() > m_bbx[gid].y_max) m_bbx[gid].y_max = t.y();
				if (t.y() < m_bbx[gid].y_min) m_bbx[gid].y_min = t.y();
			}

			for (int i = 0; i < m_bbx.size(); i++){
				//get the corners and center of each group in the rectified status
				m_bbx[i].width = m_bbx[i].x_max - m_bbx[i].x_min;
				m_bbx[i].height = m_bbx[i].y_max - m_bbx[i].y_min;
				double minLen = std::min(m_bbx[i].width, m_bbx[i].height);
				if (minLen < 0.1){
					/*m_bbx[i].l_t_corner = QPointF(m_bbx[i].x_min - m_iteration*0.2, m_bbx[i].y_min - m_scene->m_tree_maxlen*0.0);
					m_bbx[i].r_t_corner = QPointF(m_bbx[i].x_max + m_iteration*0.2, m_bbx[i].y_min - m_scene->m_tree_maxlen*0.0);
					m_bbx[i].r_b_corner = QPointF(m_bbx[i].x_max + m_iteration*0.2, m_bbx[i].y_max + m_scene->m_tree_maxlen*0.0);
					m_bbx[i].l_b_corner = QPointF(m_bbx[i].x_min - m_iteration*0.2, m_bbx[i].y_max + m_scene->m_tree_maxlen*0.0);
					m_bbx[i].center = QPointF((m_bbx[i].x_min + m_bbx[i].x_max)*0.5, (m_bbx[i].y_min + m_bbx[i].y_max)*0.5);
					m_bbx[i].width = m_bbx[i].x_max - m_bbx[i].x_min + m_iteration*0.4;
					m_bbx[i].height = m_bbx[i].y_max - m_bbx[i].y_min + 0.0;*/
					m_bbx[i].l_t_corner = QPointF(m_bbx[i].x_min - 0.25, m_bbx[i].y_min - m_scene->m_tree_maxlen*0.0);
					m_bbx[i].r_t_corner = QPointF(m_bbx[i].x_max + 0.25, m_bbx[i].y_min - m_scene->m_tree_maxlen*0.0);
					m_bbx[i].r_b_corner = QPointF(m_bbx[i].x_max + 0.25, m_bbx[i].y_max + m_scene->m_tree_maxlen*0.0);
					m_bbx[i].l_b_corner = QPointF(m_bbx[i].x_min - 0.25, m_bbx[i].y_max + m_scene->m_tree_maxlen*0.0);
					m_bbx[i].center = QPointF((m_bbx[i].x_min + m_bbx[i].x_max)*0.5, (m_bbx[i].y_min + m_bbx[i].y_max)*0.5);
					m_bbx[i].width = m_bbx[i].x_max - m_bbx[i].x_min + 0.5;
					m_bbx[i].height = m_bbx[i].y_max - m_bbx[i].y_min + 0.0;
				}
				else{
					m_bbx[i].l_t_corner = QPointF(m_bbx[i].x_min, m_bbx[i].y_min);
					m_bbx[i].r_t_corner = QPointF(m_bbx[i].x_max, m_bbx[i].y_min);
					m_bbx[i].r_b_corner = QPointF(m_bbx[i].x_max, m_bbx[i].y_max);
					m_bbx[i].l_b_corner = QPointF(m_bbx[i].x_min, m_bbx[i].y_max);
					m_bbx[i].center = QPointF((m_bbx[i].x_min + m_bbx[i].x_max)*0.5, (m_bbx[i].y_min + m_bbx[i].y_max)*0.5);
					m_bbx[i].width = m_bbx[i].x_max - m_bbx[i].x_min;
					m_bbx[i].height = m_bbx[i].y_max - m_bbx[i].y_min;
				}

				//rotate the group back to the original direction
				m_bbx[i].l_t_corner = my_rotate2(m_bbx[i].l_t_corner, m_bbx[i].angleFromY);
				m_bbx[i].r_t_corner = my_rotate2(m_bbx[i].r_t_corner, m_bbx[i].angleFromY);
				m_bbx[i].r_b_corner = my_rotate2(m_bbx[i].r_b_corner, m_bbx[i].angleFromY);
				m_bbx[i].l_b_corner = my_rotate2(m_bbx[i].l_b_corner, m_bbx[i].angleFromY);
				m_bbx[i].center = my_rotate2(m_bbx[i].center, m_bbx[i].angleFromY);

				m_bbx[i].l_t_corner -= m_scene->m_tree_center;
				m_bbx[i].r_t_corner -= m_scene->m_tree_center;
				m_bbx[i].r_b_corner -= m_scene->m_tree_center;
				m_bbx[i].l_b_corner -= m_scene->m_tree_center;
				m_bbx[i].center -= m_scene->m_tree_center;

				double x_interval = m_scene->m_tree_width*0.5 / m_xSteps_all;
				double y_interval = m_scene->m_tree_height*0.5 / m_ySteps_all;

				QPointF new_center(m_xStep*x_interval, m_yStep*y_interval);

				m_bbx[i].l_t_corner += new_center;
				m_bbx[i].r_t_corner += new_center;
				m_bbx[i].r_b_corner += new_center;
				m_bbx[i].l_b_corner += new_center;
				m_bbx[i].center += new_center;

				m_bbx[i].l_t_corner = my_transform(m_bbx[i].l_t_corner, widget_center, scale_factor_, m_rotate_angle);
				m_bbx[i].r_t_corner = my_transform(m_bbx[i].r_t_corner, widget_center, scale_factor_, m_rotate_angle);
				m_bbx[i].r_b_corner = my_transform(m_bbx[i].r_b_corner, widget_center, scale_factor_, m_rotate_angle);
				m_bbx[i].l_b_corner = my_transform(m_bbx[i].l_b_corner, widget_center, scale_factor_, m_rotate_angle);
				m_bbx[i].center = my_transform(m_bbx[i].center, widget_center, scale_factor_, m_rotate_angle);
				m_bbx[i].width *= scale_factor_;
				m_bbx[i].height *= scale_factor_;

				//
				m_bbx[i].x_max = std::max(std::max(m_bbx[i].r_b_corner.x(), m_bbx[i].r_t_corner.x()), std::max(m_bbx[i].l_b_corner.x(), m_bbx[i].l_t_corner.x()));
				m_bbx[i].x_min = std::min(std::min(m_bbx[i].r_b_corner.x(), m_bbx[i].r_t_corner.x()), std::min(m_bbx[i].l_b_corner.x(), m_bbx[i].l_t_corner.x()));
				m_bbx[i].y_max = std::max(std::max(m_bbx[i].r_b_corner.y(), m_bbx[i].r_t_corner.y()), std::max(m_bbx[i].l_b_corner.y(), m_bbx[i].l_t_corner.y()));
				m_bbx[i].y_min = std::min(std::min(m_bbx[i].r_b_corner.y(), m_bbx[i].r_t_corner.y()), std::min(m_bbx[i].l_b_corner.y(), m_bbx[i].l_t_corner.y()));
			}

			std::stringstream strStream_id;
			strStream_id << setw(6) << setfill('0') << m_image_id;
			std::string s_id = strStream_id.str();
			std::string tree_name = "E:/work_SZU_ipml2d/DeepTree/code_projects/IPML-2d/outputFRCNN-tt/JPEGImages/ps7-" + s_id + ".jpg"; //used 
			//std::string tree_name = "D:/SVN_Projects/IPML2d/code/out/JPEGImages/s1-" + s_id + ".jpg"; // my laptop path
			const char *save_fname = tree_name.c_str();

			std::string pdf_name = "E:/work_SZU_ipml2d/DeepTree/code_projects/IPML-2d/outputFRCNN-tt/PDFImages/ps7-" + s_id + ".pdf"; // used
			//std::string pdf_name = "D:/SVN_Projects/IPML2d/code/out/PDFImages/complex1-" + s_id + ".pdf"; // my laptop path
			const char *save_pfname = pdf_name.c_str();

			std::string patternf_name = "E:/work_SZU_ipml2d/DeepTree/code_projects/IPML-2d/outputFRCNN-tt/PatternImages/ps7-" + s_id + ".jpg"; //used
			//std::string patternf_name = "D:/SVN_Projects/IPML2d/code/out/PatternImages/complex1-" + s_id + ".jpg"; // my laptop path
			const char *save_patternfname = patternf_name.c_str();

			//Jianwei modified it in 2020.02.17: don't draw the pdf
			//// Setup printer
			//QPrinter printer(QPrinter::PrinterResolution);
			//printer.setOutputFormat(QPrinter::PdfFormat);
			//printer.setOutputFileName(save_pfname);
			//printer.setFullPage(true);
			//printer.setPageSize(QPrinter::Custom);
			//printer.setPaperSize(QSizeF(512, 512), QPrinter::Millimeter);
			//printer.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
			//// Get the painting context from the printer
			//QPainter painter_pdf(&printer); // Destructing painter writes the pdf file
			//double xscale = printer.pageRect().width() / double(width());
			//double yscale = printer.pageRect().height() / double(height());
			//double scale = qMin(xscale, yscale);
			//painter_pdf.translate(printer.paperRect().x() + printer.pageRect().width() / 2, printer.paperRect().y() + printer.pageRect().height() / 2);
			//painter_pdf.scale(scale, scale);
			//painter_pdf.translate(-width() / 2, -height() / 2);

			ofstream samples_save_file;
			if (m_showSampledPoints){
				std::string samples_name = "E:/work_SZU_ipml2d/DeepTree/code_projects/IPML-2d/outputFRCNN-tt/PDFImages/ps7-" + strStream_id.str() + ".txt";
				samples_save_file.open(samples_name);
				double tree_diagonal_len = m_scene->m_tree_diagonal*scale_factor_;
				samples_save_file << tree_diagonal_len << " " << tree_diagonal_len << std::endl;
			}

			for (int i = 0; i < tree2d.size(); i++){
				t_line line = tree2d[i];
				QPointF s(line.p_start.X(), -line.p_start.Y());
				QPointF t(line.p_end.X(), -line.p_end.Y());
				s -= m_scene->m_tree_center;
				t -= m_scene->m_tree_center;

				double x_interval = m_scene->m_tree_width*0.5 / m_xSteps_all;
				double y_interval = m_scene->m_tree_height*0.5 / m_ySteps_all;

				QPointF new_center(m_xStep*x_interval, m_yStep*y_interval);
				s += new_center;
				t += new_center;

				s = my_transform(s, widget_center, scale_factor_, m_rotate_angle);
				t = my_transform(t, widget_center, scale_factor_, m_rotate_angle);

				//draw jpg
				painter2.save();
				float w;
				if (line.strahler_number == 0) {
					w = line.width;
					//painter.setPen(QPen(colors[cdx], m_line_thickness*w));
					painter2.setPen(QPen(Qt::black, m_line_thickness*w));
				}
				else {
					w = width_factor*(line.strahler_number + 1);
					painter2.setPen(QPen(strahlerNumberColors[line.strahler_number], m_line_thickness*w));
				}
				//painter2.translate(widget_center.x(), widget_center.y());
				//painter2.scale(scale_factor, scale_factor);
				//painter2.rotate(m_rotate_angle);
				painter2.drawLine(s, t);
				painter2.restore();
				
				// Draw pdf: you should open it if you want to save to pdf
				/*painter_pdf.save();
				//painter_pdf.setPen(QPen(Qt::black, m_line_thickness));
				if (line.strahler_number == 0) {
					w = line.width;
					//painter.setPen(QPen(colors[cdx], m_line_thickness*w));
					painter_pdf.setPen(QPen(Qt::black, m_line_thickness*w));
				}
				else {
					w = width_factor*(line.strahler_number + 1);
					painter_pdf.setPen(QPen(strahlerNumberColors[line.strahler_number], m_line_thickness*w));
				}
				painter_pdf.drawLine(s, t);*/

				//save sampled points
				/*if (m_showSampledPoints){
					double line_len_input = (line.p_start - line.p_end).Length();
					double line_len = (line.p_start - line.p_end).Length()*scale_factor_;
					int sampled_points = std::max(2, int(line_len / 3.0));
					double step = line_len_input / (sampled_points - 1);
					for (int j = 0; j < sampled_points; j++){
						R2Vector new_point = line.p_start + line.direction*(step*j);
						QPointF np(new_point.X(), -new_point.Y());
						np -= m_scene->m_tree_center;
						np += new_center;
						np = my_transform(np, widget_center, scale_factor_, m_rotate_angle);
						samples_save_file << np.x() << " " << np.y() << std::endl;
					}
				}*/

				if (m_showSampledPoints){
					double line_len_input = (line.p_start - line.p_end).Length();
					double line_len = (line.p_start - line.p_end).Length()*scale_factor_;
					int sampled_points = std::max(4, int(line_len / 2.0));
					//int sampled_points = 6;
					double step = line_len_input / (sampled_points - 1);
					QPointF last_point;

					//const siv::PerlinNoise perlin(seed);
					const double fx = sampled_points / frequency;

					for (int j = 0; j < sampled_points; j++){
						R2Vector new_point = line.p_start + line.direction*(step*j);

						R2Vector new_point_noise;
						if (j == 0 || j == sampled_points - 1){
							new_point_noise = new_point;
						}
						else{
							double angle_noise = (rand() % (15 * 2)) - 15;
							QPointF rotate_dir = my_rotate(QPointF(line.direction.X(), line.direction.Y()), 90 + angle_noise);
							R2Vector orthogonal_dir(rotate_dir.x(), rotate_dir.y());
							orthogonal_dir.Normalize();
							//double noise_scale = 7.0*step;
							//double rand_val = (randInt(0, 4) / 2.0 - 1.0)*noise_scale;
							double pp = perlin.octaveNoise0_1(new_point_noise.X(), new_point_noise.Y(), octaves);
							//double pp = perlin.octaveNoise0_1(j/fx, octaves);
							double rand_val = (pp - noise_base)*noise_scale*step;
							new_point_noise = new_point + rand_val*orthogonal_dir;
						}

						QPointF np(new_point_noise.X(), -new_point_noise.Y());
						np -= m_scene->m_tree_center;
						np = my_transform(np, widget_center, scale_factor_, m_rotate_angle);

						/*if (j != 0){
							painter_pdf.drawLine(last_point, np);  //Jianwei modified it in 2020.02.17: don't draw the pdf
						}*/
						last_point = np;
					}
				}

				////for old RtinaNet's data 
				//int gid = line.group_id;
				//if (s.x() > m_bbx[gid].x_max) m_bbx[gid].x_max = s.x();
				//if (s.x() < m_bbx[gid].x_min) m_bbx[gid].x_min = s.x();
				//if (s.y() > m_bbx[gid].y_max) m_bbx[gid].y_max = s.y(); //we reverse the y value to make the coordinate system same to the QWidget
				//if (s.y() < m_bbx[gid].y_min) m_bbx[gid].y_min = s.y();
				//if (t.x() > m_bbx[gid].x_max) m_bbx[gid].x_max = t.x();
				//if (t.x() < m_bbx[gid].x_min) m_bbx[gid].x_min = t.x();
				//if (t.y() > m_bbx[gid].y_max) m_bbx[gid].y_max = t.y();
				//if (t.y() < m_bbx[gid].y_min) m_bbx[gid].y_min = t.y();
			}
			if (m_showSampledPoints){
				samples_save_file.close();
			}

			//painter_pdf.restore();  //Jianwei modified it in 2020.02.17: don't draw the pdf

			//Jianwei modified it in 2020.02.17: don't draw the pdf
			QImage image_pattern(width(), height(), QImage::Format_RGB32);
			image_pattern.fill(Qt::white);
			// draw jpg image
			QPainter painter_pattern;
			painter_pattern.begin(&image_pattern);
			painter_pattern.setRenderHint(QPainter::Antialiasing, true);
			//// draw pdf image
			//QPrinter printer_pdf(QPrinter::PrinterResolution);
			//printer_pdf.setOutputFormat(QPrinter::PdfFormat);
			//printer_pdf.setOutputFileName(save_patternfname);
			//printer_pdf.setFullPage(true);
			//printer_pdf.setPageSize(QPrinter::Custom);
			//printer_pdf.setPaperSize(QSizeF(512, 512), QPrinter::Millimeter);
			//printer_pdf.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
			//// Get the painting context from the printer
			//QPainter painter_pattern(&printer_pdf); // Destructing painter writes the pdf file
			//double xscale2 = printer_pdf.pageRect().width() / double(width());
			//double yscale2 = printer_pdf.pageRect().height() / double(height());
			//double scale2 = qMin(xscale2, yscale2);
			//painter_pattern.translate(printer_pdf.paperRect().x() + printer_pdf.pageRect().width() / 2, printer_pdf.paperRect().y() + printer_pdf.pageRect().height() / 2);
			//painter_pattern.scale(scale2, scale2);
			//painter_pattern.translate(-width() / 2, -height() / 2);

			//draw bounding box
			for (int j = 0; j < m_bbx.size(); j++){
				Bounding_box bbx = m_bbx[j];

				//double randomValue1 = randInt(0, 100) / 50;
				//double randomValue2 = randInt(0, 100) / 50;
				//QPoint left_bottom_corner((int)(bbx.x_min - randomValue1), (int)(bbx.y_min - randomValue1)), right_top_corner((int)(bbx.x_max + randomValue2), (int)(bbx.y_max + randomValue2));
				//QPoint left_bottom_corner((int)bbx.x_min, (int)bbx.y_min), right_top_corner((int)bbx.x_max, (int)bbx.y_max);

				//here painer2 is used to draw bounding box
				painter2.save();
				//int cdx = j%10;
				int cdx = bbx.class_id;
				painter2.setPen(QPen(labelColors2[cdx], 3));
				painter2.setBrush(Qt::NoBrush);

				//Jianwei modified it in 2020.02.17: don't draw the pdf
				///*painter_pdf.save();
				//painter_pdf.setPen(QPen(labelColors2[cdx], 3)); 
				//painter_pdf.setBrush(Qt::NoBrush);*/

				//for R2CNN
				if (is_in_imageSpace(m_bbx[j].l_t_corner, width(), height()) && is_in_imageSpace(m_bbx[j].r_t_corner, width(), height())
					&& is_in_imageSpace(m_bbx[j].r_b_corner, width(), height()) && is_in_imageSpace(m_bbx[j].l_b_corner, width(), height())){

					/*painter2.drawLine(m_bbx[j].l_t_corner, m_bbx[j].r_t_corner);
					painter2.drawLine(m_bbx[j].r_t_corner, m_bbx[j].r_b_corner);
					painter2.drawLine(m_bbx[j].r_b_corner, m_bbx[j].l_b_corner);
					painter2.drawLine(m_bbx[j].l_b_corner, m_bbx[j].l_t_corner);*/

					/*painter_pdf.drawLine(m_bbx[j].l_t_corner, m_bbx[j].r_t_corner);
					painter_pdf.drawLine(m_bbx[j].r_t_corner, m_bbx[j].r_b_corner);
					painter_pdf.drawLine(m_bbx[j].r_b_corner, m_bbx[j].l_b_corner);
					painter_pdf.drawLine(m_bbx[j].l_b_corner, m_bbx[j].l_t_corner);*/

					double box_h = m_bbx[j].height;
					/*pattern_svgs_inkBranch[8]  pattern_svgs_flowerBranch[4]  pattern_svgs_snowflake[18] pattern_svgs_leaf[9] pattern_svgs_flower[16]2
					//pattern_svgs_grass[20] // 0-3 4-7 8-10 11-19   pattern_svgs_fruit[15], branch_parts_names[38] sketch_lines_names[20]
					// CG_parts_names[13] sketch_lines_middle[5] pattern_svgs_stone[8] pattern_svgs_symbol[12] pattern_svgs_silhouettes[10]*/
					int pattern_id;
					pattern_id = randInt(0, 3);
					/*if (m_bbx[j].group_id < 20){
					pattern_id = randInt(0, 3);
					}*/

					/*if (m_bbx[j].branch_status == 0) {
						pattern_id = 0;
					}
					else if (m_bbx[j].branch_status == 1) {
						pattern_id = 1;
					}
					else if (m_bbx[j].branch_status == 2) {
						pattern_id = 2;
					}*/

					pattern_id = (m_bbx[j].iteration-1) % 3;

					////load from .jpg or .png files
					//QImage pattern_img;
					//pattern_img.load(sketch_lines_names[pattern_id]);
					//QRectF img_src(0, 0, pattern_img.width(), pattern_img.height());
					//double img_max_len = std::max(pattern_img.width(), pattern_img.height());
					////double img_max_len = pattern_img.height();
					//double scale_ratio = img_max_len / (box_h*1.0); //for lines
					////double scale_ratio = img_max_len / (box_h*0.8); //for patterns
					//double pattern_w = pattern_img.width() / scale_ratio;
					//double pattern_h = pattern_img.height() / scale_ratio;

					//load from .svg files
					QSvgRenderer svg(pattern_svgs_330[pattern_id], this);
					QSize svg_size = svg.defaultSize();
					double img_max_len = std::max(svg_size.width(), svg_size.height());
					//double img_max_len = svg_size.height();
					//double scale_ratio = img_max_len / (box_h*0.9);
					double scale_ratio = img_max_len / (box_h*0.9);
					double pattern_w = svg_size.width() / scale_ratio;
					double pattern_h = svg_size.height() / scale_ratio;

					QPoint ori_ltc((0.0 - pattern_w*0.5), (0.0 - pattern_h*0.5));
					QPoint ori_rbc((0.0 + pattern_w*0.5), (0.0 + pattern_h*0.5));
					QRectF img_target(ori_ltc, ori_rbc);

					//painter2.save();
					//painter2.translate(m_bbx[j].center.x(), m_bbx[j].center.y());
					//double rotated_angle = 180 - m_bbx[j].angleFromY*180.0 / M_PI;
					//painter2.rotate(rotated_angle);
					////painter2.drawImage(img_target, pattern_img, img_src);
					//svg.render(&painter2, img_target);
					//painter2.restore();

					//int indicator = j % 2;
					int indicator = 0;
					if (m_bbx[j].branch_status == 1) {
						indicator = 1;
					}
					double rotated_angle;

					painter_pattern.save();
					painter_pattern.translate(m_bbx[j].center.x(), m_bbx[j].center.y());
					//painter_pattern.rotate(180-m_rotate_angle);
					if (indicator == 0){
						rotated_angle = 180 - m_bbx[j].angleFromY*180.0 / M_PI;
					}
					else{
						rotated_angle = 180 - m_bbx[j].angleFromY*180.0 / M_PI;
					}
					painter_pattern.rotate(rotated_angle);
					//painter_pattern.drawImage(img_target, pattern_img, img_src);
					svg.render(&painter_pattern, img_target);
					painter_pattern.restore();

					////following is used for pdf 
					//painter_pdf.save();
					//painter_pdf.translate(m_bbx[j].center.x(), m_bbx[j].center.y());
					//if (indicator == 0) {
					//	rotated_angle = 360 - m_bbx[j].angleFromY*180.0 / M_PI;
					//}
					//else {
					//	rotated_angle = 360 - m_bbx[j].angleFromY*180.0 / M_PI;
					//}
					//painter_pdf.rotate(rotated_angle);
					////painter2.drawImage(img_target, pattern_img, img_src);
					//svg.render(&painter_pdf, img_target);
					//painter_pdf.restore();

					//now we modify the boundingbox according to the pattern 
					m_bbx[j].l_t_corner = QPointF(m_bbx[j].center.x() - pattern_w*0.5, m_bbx[j].center.y() - pattern_h*0.5) - m_bbx[j].center;
					m_bbx[j].r_t_corner = QPointF(m_bbx[j].center.x() + pattern_w*0.5, m_bbx[j].center.y() - pattern_h*0.5) - m_bbx[j].center;
					m_bbx[j].r_b_corner = QPointF(m_bbx[j].center.x() + pattern_w*0.5, m_bbx[j].center.y() + pattern_h*0.5) - m_bbx[j].center;
					m_bbx[j].l_b_corner = QPointF(m_bbx[j].center.x() - pattern_w*0.5, m_bbx[j].center.y() + pattern_h*0.5) - m_bbx[j].center;
					m_bbx[j].l_t_corner = my_rotate2(m_bbx[j].l_t_corner, m_rotate_angle*M_PI / 180.0);
					m_bbx[j].r_t_corner = my_rotate2(m_bbx[j].r_t_corner, m_rotate_angle*M_PI / 180.0);
					m_bbx[j].r_b_corner = my_rotate2(m_bbx[j].r_b_corner, m_rotate_angle*M_PI / 180.0);
					m_bbx[j].l_b_corner = my_rotate2(m_bbx[j].l_b_corner, m_rotate_angle*M_PI / 180.0);
					m_bbx[j].l_t_corner = my_rotate2(m_bbx[j].l_t_corner, m_bbx[j].angleFromY) + m_bbx[j].center;
					m_bbx[j].r_t_corner = my_rotate2(m_bbx[j].r_t_corner, m_bbx[j].angleFromY) + m_bbx[j].center;
					m_bbx[j].r_b_corner = my_rotate2(m_bbx[j].r_b_corner, m_bbx[j].angleFromY) + m_bbx[j].center;
					m_bbx[j].l_b_corner = my_rotate2(m_bbx[j].l_b_corner, m_bbx[j].angleFromY) + m_bbx[j].center;
					m_bbx[j].width = pattern_w;
					m_bbx[j].height = pattern_h;

					//m_bbx[j].class_id = pattern_id + 1;

					/*painter_pattern.drawLine(m_bbx[j].l_t_corner, m_bbx[j].r_t_corner);
					painter_pattern.drawLine(m_bbx[j].r_t_corner, m_bbx[j].r_b_corner);
					painter_pattern.drawLine(m_bbx[j].r_b_corner, m_bbx[j].l_b_corner);
					painter_pattern.drawLine(m_bbx[j].l_b_corner, m_bbx[j].l_t_corner);*/
				}
                //end: Jianwei modified it in 2020.02.17: don't draw the pdf

				////for RetinaNet
				//QPoint ltc((m_bbx[j].center.x() - m_bbx[j].width*0.5), (m_bbx[j].center.y() - m_bbx[j].height*0.5));
				//QPoint rtc((m_bbx[j].center.x() + m_bbx[j].width*0.5), (m_bbx[j].center.y() - m_bbx[j].height*0.5));
				//QPoint rbc((m_bbx[j].center.x() + m_bbx[j].width*0.5), (m_bbx[j].center.y() + m_bbx[j].height*0.5));
				//QPoint lbc((m_bbx[j].center.x() - m_bbx[j].width*0.5), (m_bbx[j].center.y() + m_bbx[j].height*0.5));
				//painter2.drawLine(ltc, rtc);
				//painter2.drawLine(rtc, rbc);
				//painter2.drawLine(rbc, lbc);
				//painter2.drawLine(lbc, ltc);

				painter2.restore();

				//painter_pdf.restore(); //Jianwei modified it in 2020.02.17: don't draw the pdf
			}

			//const char *save_fname = tree_name.c_str();
			image.save(save_fname);
			image_pattern.save(save_patternfname);

			painter2.end();
			//painter_pdf.end();
			painter_pattern.end();
		}
	}
	
	if (m_show_image){
		int sw = m_parse_img.width();
		int sh = m_parse_img.height();
		QRectF src(0, 0, sw, sh);
		QRectF target(BL, TR);
		painter.save();
		painter.drawImage(target, m_parse_img, src);

		QImage image2(width(), height(), QImage::Format_RGB32);
		image2.fill(Qt::white);
		QPainter painter2;
		painter2.begin(&image2);
		painter2.setRenderHint(QPainter::Antialiasing, true);
		painter2.drawImage(target, m_parse_img, src);

		//used for save bounding boxes
		std::string pdf_name = m_save_image_name;
		pdf_name.replace(pdf_name.end() - 3, pdf_name.end(), "pdf");
		const char *save_pfname = pdf_name.c_str();

		// Setup printer
		QPrinter printer(QPrinter::PrinterResolution);
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName(save_pfname);
		printer.setFullPage(true);
		printer.setPageSize(QPrinter::Custom);
		printer.setPaperSize(QSizeF(512, 512), QPrinter::Millimeter);
		printer.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
		// Get the painting context from the printer
		QPainter painter_pdf(&printer); // Destructing painter writes the pdf file
		double xscale = printer.pageRect().width() / double(width());
		double yscale = printer.pageRect().height() / double(height());
		double scale = qMin(xscale, yscale);
		painter_pdf.translate(printer.paperRect().x() + printer.pageRect().width() / 2, printer.paperRect().y() + printer.pageRect().height() / 2);
		painter_pdf.scale(scale, scale);
		painter_pdf.translate(-width() / 2, -height() / 2);
		// Draw the image
		QRectF pdf_src(0, 0, sw, sh);
		QRectF pdf_dst(BL, TR);
		//painter_pdf.drawImage(pdf_dst, m_parse_img, pdf_src);

		//used for save graph
		std::string pdf_graph_name = m_save_image_name;
		pdf_graph_name.replace(pdf_graph_name.end() - 4, pdf_graph_name.end(), "._graph.pdf");
		const char *save_pfname_graph = pdf_graph_name.c_str();
		// Setup printer
		QPrinter printer_graph(QPrinter::PrinterResolution);
		printer_graph.setOutputFormat(QPrinter::PdfFormat);
		printer_graph.setOutputFileName(save_pfname_graph);
		printer_graph.setFullPage(true);
		printer_graph.setPageSize(QPrinter::Custom);
		printer_graph.setPaperSize(QSizeF(512, 512), QPrinter::Millimeter);
		printer_graph.setPageMargins(0, 0, 0, 0, QPrinter::Millimeter);
		// Get the painting context from the printer
		QPainter painter_pdf_graph(&printer_graph); // Destructing painter writes the pdf file
		double xscale_graph = printer_graph.pageRect().width() / double(width());
		double yscale_graph = printer_graph.pageRect().height() / double(height());
		double scale_graph = qMin(xscale_graph, yscale_graph);
		painter_pdf_graph.translate(printer_graph.paperRect().x() + printer_graph.pageRect().width() / 2, printer_graph.paperRect().y() + printer_graph.pageRect().height() / 2);
		painter_pdf_graph.scale(scale_graph, scale_graph);
		painter_pdf_graph.translate(-width() / 2, -height() / 2);
		// Draw the image
		QRectF pdf_src_graph(0, 0, sw, sh);
		QRectF pdf_dst_graph(BL, TR);
		//painter_pdf.drawImage(pdf_dst, m_parse_img, pdf_src);

		//draw the detected bounding boxes
		for (int k = 0; k < m_bbx_parse.size(); k++){
			if (m_bbxID != -1){
				if (m_bbxID != k){
					continue;
				}
			}
			
			Bounding_box_parse cur_bbx = m_bbx_parse[k];
			
			QPoint ltc2(m_bbx_parse[k].l_t_corner.X(), m_bbx_parse[k].l_t_corner.Y());
			QPoint rtc2(m_bbx_parse[k].r_t_corner.X(), m_bbx_parse[k].r_t_corner.Y());
			QPoint rbc2(m_bbx_parse[k].r_b_corner.X(), m_bbx_parse[k].r_b_corner.Y());
			QPoint lbc2(m_bbx_parse[k].l_b_corner.X(), m_bbx_parse[k].l_b_corner.Y());

			//int cdx = k % 10;
			int cid = cur_bbx.label_id;
			//int cid = k%9;
			//painter.setPen(QPen(colors[cid], 3));
			painter.setPen(QPen(labelColors2[cid], 3));
			painter.setBrush(Qt::NoBrush);
			painter.drawLine(ltc2, rtc2);
			painter.drawLine(rtc2, rbc2);
			painter.drawLine(rbc2, lbc2);
			painter.drawLine(lbc2, ltc2);

			painter2.setPen(QPen(labelColors2[cid], 3));
			painter2.setBrush(Qt::NoBrush);
			painter2.drawLine(ltc2, rtc2);
			painter2.drawLine(rtc2, rbc2);
			painter2.drawLine(rbc2, lbc2);
			painter2.drawLine(lbc2, ltc2);

			//painter_pdf.save();
			painter_pdf.setPen(QPen(labelColors2[cid], 3));
			painter_pdf.drawLine(ltc2, rtc2);
			painter_pdf.drawLine(rtc2, rbc2);
			painter_pdf.drawLine(rbc2, lbc2);
			painter_pdf.drawLine(lbc2, ltc2);
			//painter_pdf.restore();

		}

		//draw the graph
		// traverse the nodes;
		//get the property map for vertex indices
		//std::pair<edge_iterator, edge_iterator> ei = edges(m_graph);
		typedef boost::property_map<UndirectedGraph, boost::vertex_index_t>::type IndexMap;
		IndexMap index = get(boost::vertex_index, m_graph);
		boost::graph_traits<UndirectedGraph>::edge_iterator ei, ei_end;
		painter_pdf_graph.save();
		for (tie(ei, ei_end) = edges(m_graph); ei != ei_end; ++ei){
			int bbx_idx_1 = index[boost::source(*ei, m_graph)];
			int bbx_idx_2 = index[boost::target(*ei, m_graph)];

			QPoint bbx_center_1(m_bbx_parse[bbx_idx_1].center_position.X(), m_bbx_parse[bbx_idx_1].center_position.Y());
			QPoint bbx_center_2(m_bbx_parse[bbx_idx_2].center_position.X(), m_bbx_parse[bbx_idx_2].center_position.Y());
			painter.setPen(QPen(QColor::fromRgb(106, 190, 131), 4));
			painter.setBrush(Qt::NoBrush);
			painter.drawLine(bbx_center_1, bbx_center_2);

			painter_pdf_graph.setPen(QPen(QColor::fromRgb(106, 190, 131), 4)); //(106, 190, 131) (65, 146, 75)
			painter_pdf_graph.setBrush(Qt::NoBrush);
			painter_pdf_graph.drawLine(bbx_center_1, bbx_center_2);
		}
		

		typedef boost::graph_traits<UndirectedGraph>::vertex_iterator vertex_iter;
		std::pair<vertex_iter, vertex_iter> vp;
		for (vp = vertices(m_graph); vp.first != vp.second; ++vp.first){
			int bbx_idx = index[*vp.first];
			Bounding_box_parse cur_bbx = m_bbx_parse[bbx_idx];

			painter.save();
			QPointF bbx_center(cur_bbx.center_position.X(), cur_bbx.center_position.Y());
			painter.setPen(QPen(QColor::fromRgb(221, 119, 119), 2));
			//painter.setBrush(Qt::NoBrush);
			painter.setBrush(QBrush(QColor::fromRgb(221, 119, 119)));
			painter.drawEllipse(bbx_center, 6, 6);
			
			QFont font = painter.font();
			font.setPointSize(16);
			painter.setFont(font);
			painter.setPen(Qt::darkBlue);
			painter.drawText(bbx_center.x() + 4.0, bbx_center.y() + 4.0, QString::number(bbx_idx));
			painter.restore();

			painter_pdf_graph.setPen(QPen(QColor::fromRgb(221, 119, 119), 2));
			painter_pdf_graph.setBrush(QBrush(QColor::fromRgb(221, 119, 119)));
			painter_pdf_graph.drawEllipse(bbx_center, 6, 6);
		}

		painter.restore();
		painter_pdf_graph.restore();

		const char *save_img_fname = m_save_image_name.c_str();
		image2.save(save_img_fname);

		painter2.end();
		painter_pdf.end();
		painter_pdf_graph.end();
	}
	painter.end();
}

/*
Functions for rendering training and testing images 
*/

//for Faster-RCNN data format: use one specific rule
void GlViewer::render_images_all_FRCNN(){ //for training

	m_render = true;
	qDebug() << "Start rendering image...";

	m_image_id = 0;
	string class_labels[8] = { "b10", "b20", "b30", "b40", "b50", "b60", "b70", "b80" };

	for (int iter = 1; iter <= 4; iter++){ // growth iterations
		m_iteration = iter;
		//for (int deg = 15; deg <= 50; deg += 5){ // for branch case-8
		for (int deg = 35; deg <= 75; deg += 10){ // branching angles
		//double deg = 60;
		m_scene->lSystem()->set_iterations(iter);
		m_scene->lSystem()->set_branchingAngle(double(deg));
		m_scene->draw_rules();

		//int x_steps = 2, y_steps = 2; //training
		int x_steps = 1, y_steps = 1; //test

		bool height_longer = (m_scene->m_tree_height > m_scene->m_tree_width) ? true : false;
		if (height_longer && m_scene->m_tree_height / m_scene->m_tree_width > 1.5){
			double ratio = m_scene->m_tree_height / m_scene->m_tree_width;
			y_steps = x_steps * ratio;
		}
		else if (height_longer && m_scene->m_tree_width / m_scene->m_tree_height > 1.5){
			double ratio = m_scene->m_tree_width / m_scene->m_tree_height;
			x_steps = y_steps*ratio;
		}

		x_steps = 1;
		y_steps = 1;

		m_xSteps_all = std::max(x_steps * 2, 1);
		m_ySteps_all = std::max(y_steps * 2, 1);
		qDebug() << m_xSteps_all << " -steps- " << m_ySteps_all;

		for (int i = -x_steps; i <= x_steps; i++){ // translate along X aix
			m_xStep = i;
			for (int j = -y_steps; j <= y_steps; j++){ //translate along Y aix
				m_yStep = j;
				/*scale_factor_ = m_scene->m_tree_scale_factor;
				if (scale_factor_ > 28.0){
					scale_factor_ = 28.0;
				}*/
				//scale_factor_ = 15.1704; // for case-1
				//scale_factor_ = 17; // for case-2-80 degree, no scale
				//scale_factor_ = 32; // for case-2-70 degree, scale
				//scale_factor_ = 30.1; // for case-2-45 degree, no scale
				//scale_factor_ = 31; // for case-2-60 degree, scale
				//scale_factor_ = 9.07; // for branch5-iter4  
				double s_begin = 0.6, s_end = 1.0, s_step = 0.1;
				if (iter == 1){
					s_begin = 0.3; s_end = 0.7; s_step = 0.1;
				}
				else if (iter == 2){
					s_begin = 0.4; s_end = 0.8; s_step = 0.1;
				}
				else if (iter == 4){
					s_begin = 1.0; s_end = 1.0; s_step = 0.1;
				}
				/*else{
					s_begin = 0.6; s_end = 1.0; s_step = 0.2;
				}*/
				for (double sf = s_begin; sf <= s_end; sf += s_step){ // scale 
				scale_factor_ = m_scene->m_tree_scale_factor*sf;
				m_rotate_angle = 0.0;
				while (m_rotate_angle < 360){ //rotate
					repaint();
					m_rotate_angle += 60; //10 for training 60 for testing

					std::stringstream strStream_id;
					strStream_id << setw(6) << setfill('0') << m_image_id;

					std::string annotations_name = "E:/work_SZU_ipml2d/DeepTree/code_projects/IPML-2d/outputFRCNN-tt/txts/p1-" + strStream_id.str() + ".txt";
					ofstream file;
					file.open(annotations_name);
			
					//file << "imagesource:GoogleEarth" << std::endl;
					//file << " gsd : null "<< std::endl;

					for (int k = 0; k < m_bbx.size(); k++){
						Bounding_box bbx = m_bbx[k];
						int xmin = (int)bbx.x_min, xmax = (int)bbx.x_max, ymin = (int)bbx.y_min, ymax = (int)bbx.y_max;
						//if (xmin > 0 && ymin >0 && xmin < xmax && ymin < ymax && xmax < width() && ymax < height()){
						if (is_in_imageSpace(bbx.l_t_corner, width(), height()) && is_in_imageSpace(bbx.r_t_corner, width(), height())
							&& is_in_imageSpace(bbx.r_b_corner, width(), height()) && is_in_imageSpace(bbx.l_b_corner, width(), height())){
							//file << s_id << "," << xmin << "," << ymin << "," << xmax << "," << ymax << ",branch" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
							int idx = deg / 10;
							//file << bbx.l_t_corner.x() << "," << bbx.l_t_corner.y() << "," << bbx.r_t_corner.x() << "," << bbx.r_t_corner.y() << "," << 
							//	bbx.r_b_corner.x() << "," << bbx.r_b_corner.y() << "," << bbx.l_b_corner.x() << "," << bbx.l_b_corner.y() << "," <<
							//	class_labels[idx - 1] << ",0" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
							file << bbx.l_t_corner.x() << "," << bbx.l_t_corner.y() << "," << bbx.r_t_corner.x() << "," << bbx.r_t_corner.y() << "," <<
								bbx.r_b_corner.x() << "," << bbx.r_b_corner.y() << "," << bbx.l_b_corner.x() << "," << bbx.l_b_corner.y() << "," <<
								"b60,0" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
						}
					}
					file.close();
					m_image_id++;
				}
				}
			}
		}
		}
	}

	qDebug() << "End rendering image...";
	m_render = false;
}

void GlViewer::render_images_all_FRCNN_test(){ //for testing

	m_render = true;
	qDebug() << "Start rendering image...";

	m_image_id = 0;
	string class_labels[8] = { "b10", "b20", "b30", "b40", "b50", "b60", "b70", "b80" };

	for (int iter = 2; iter <= 7; iter++){ // growth iterations
	//for (int iter = 10; iter <= 30; iter++){ // growth iterations, for b19
		m_iteration = iter;
		//for (int deg = 10; deg <= 50; deg += 10){ // for branch case-8
		//for (int deg = 15; deg <= 40; deg += 5){ // for branch case-8
		for (int deg = 20; deg <= 80; deg += 10){ // branching angles
		//for (int deg = 8; deg <= 20; deg += 4){ // for branch case-13
			//double deg2 = 78.5;
			m_scene->lSystem()->set_iterations(iter);
			m_scene->lSystem()->set_branchingAngle(double(deg));
			m_scene->draw_rules();

			double averge_branching_angle = m_scene->lSystem()->get_average_branch_angle();
			double average_scalar = m_scene->lSystem()->get_average_scalar();
			AssociativeArray used_grammar = m_scene->lSystem()->get_used_grammar();

			//int x_steps = 2, y_steps = 2; //training
			int x_steps = 1, y_steps = 1; //test

			bool height_longer = (m_scene->m_tree_height > m_scene->m_tree_width) ? true : false;
			if (height_longer && m_scene->m_tree_height / m_scene->m_tree_width > 1.5){
				double ratio = m_scene->m_tree_height / m_scene->m_tree_width;
				y_steps = x_steps * ratio;
			}
			else if (height_longer && m_scene->m_tree_width / m_scene->m_tree_height > 1.5){
				double ratio = m_scene->m_tree_width / m_scene->m_tree_height;
				x_steps = y_steps*ratio;
			}

			x_steps = 1;
			y_steps = 1;

			m_xSteps_all = std::max(x_steps * 2, 1);
			m_ySteps_all = std::max(y_steps * 2, 1);
			qDebug() << m_xSteps_all << " -steps- " << m_ySteps_all;
			m_xStep = 0;
			m_yStep = 0;
			//for (int i = -x_steps; i <= x_steps; i++){ // translate along X aix
			//m_xStep = i;
			//for (int j = -y_steps; j <= y_steps; j++){ //translate along Y aix
			//m_yStep = j;  
			double s_begin = 0.6, s_end = 1.0, s_step = 0.1; //for b2
			/*if (iter >= 6){
				s_begin = 0.6; s_end = 1.0; s_step = 0.2;
			}*/
			for (double sf = s_begin; sf <= s_end; sf += s_step){ // scale 
				scale_factor_ = m_scene->m_tree_scale_factor*sf;
				m_rotate_angle = 0.0;
				while (m_rotate_angle < 360){ //rotate
					repaint();
					m_rotate_angle += 360; //10 for training 60 for testing

					std::stringstream strStream_id;
					strStream_id << setw(6) << setfill('0') << m_image_id;

					// we output the original grammar information
					std::string grammar_info_name = "E:/work_SZU_ipml2d/DeepTree/code_projects/IPML-2d/outputFRCNN-tt/grammar_info/s1-" + strStream_id.str() + ".txt";
					ofstream grammar_info_file;
					grammar_info_file.open(grammar_info_name);
					int grammar_len = 0;
					grammar_info_file <<"The original grammar: "<< std::endl;
					AssociativeArray::const_iterator iter;
					for (iter = used_grammar.begin(); iter != used_grammar.end(); ++iter)
					{
						string key = iter->first;
						vector<string> value = iter->second;
						for (int i = 0; i < value.size(); i++){
							grammar_info_file << key << "=" << value[i] << endl;
							grammar_len += key.length();
							for (int k = 0; k < value[i].length(); k++){
								if (value[i].at(k) == 'F' || value[i].at(k) == 'X' || value[i].at(k) == 'A' || value[i].at(k) == '+' ||
									value[i].at(k) == '-' || value[i].at(k) == '[' || value[i].at(k) == ']'){
									grammar_len += 1;
								}
							}
						}
					}
					grammar_info_file << std::endl;
					grammar_info_file << "Statistics of the grammar:" << std::endl;
					grammar_info_file << "    (1) grammar length : " << grammar_len << endl;
					grammar_info_file << "    (2) branching angle: " << averge_branching_angle << std::endl;
					grammar_info_file << "    (3) scaling  fator : " << average_scalar << std::endl;
					grammar_info_file.close();

					std::string annotations_name = "E:/work_SZU_ipml2d/DeepTree/code_projects/IPML-2d/outputFRCNN-tt/txts/s1-" + strStream_id.str() + ".txt";
					ofstream file;
					file.open(annotations_name);

					//file << "imagesource:GoogleEarth" << std::endl;
					//file << " gsd : null "<< std::endl;

					for (int k = 0; k < m_bbx.size(); k++){
						Bounding_box bbx = m_bbx[k];
						int xmin = (int)bbx.x_min, xmax = (int)bbx.x_max, ymin = (int)bbx.y_min, ymax = (int)bbx.y_max;
						//if (xmin > 0 && ymin >0 && xmin < xmax && ymin < ymax && xmax < width() && ymax < height()){
						if (is_in_imageSpace(bbx.l_t_corner, width(), height()) && is_in_imageSpace(bbx.r_t_corner, width(), height())
							&& is_in_imageSpace(bbx.r_b_corner, width(), height()) && is_in_imageSpace(bbx.l_b_corner, width(), height())){
							//file << s_id << "," << xmin << "," << ymin << "," << xmax << "," << ymax << ",branch" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
							int idx = deg / 10;
							//file << bbx.l_t_corner.x() << "," << bbx.l_t_corner.y() << "," << bbx.r_t_corner.x() << "," << bbx.r_t_corner.y() << "," << 
							//	bbx.r_b_corner.x() << "," << bbx.r_b_corner.y() << "," << bbx.l_b_corner.x() << "," << bbx.l_b_corner.y() << "," <<
							//	class_labels[idx - 1] << ",0" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
							file << bbx.l_t_corner.x() << "," << bbx.l_t_corner.y() << "," << bbx.r_t_corner.x() << "," << bbx.r_t_corner.y() << "," <<
								bbx.r_b_corner.x() << "," << bbx.r_b_corner.y() << "," << bbx.l_b_corner.x() << "," << bbx.l_b_corner.y() << "," <<
								"b60,0" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
						}
					}
					file.close();
					m_image_id++;
				}
			}
			//}
			//}
		}
	}

	qDebug() << "End rendering image...";
	m_render = false;
}

//for Faster-RCNN data format: use random rules
void GlViewer::render_images_all_FRCNN_random(){ //for training

	m_render = true;
	qDebug() << "Start rendering image...";

	std::srand(unsigned(std::time(0)));

	m_image_id = 0;
	for (int kk = 0; kk < 4; kk++){
		for (int iter = 3; iter <= 5; iter++){ // growth iterations
			//m_scene->lSystem()->select_rules_random();
			m_iteration = iter;
			int deg_begin = 40, deg_end = 70, deg_step = 10;
			if (iter == 2){
				deg_begin = 45, deg_end = 60, deg_step = 15;
			}
			for (int deg = 30; deg <= (50 + kk * 10); deg += (5 + kk * 5)){ // branching angles
			//for (int deg = 40; deg <= 70; deg += 10){ // branching angles
			//for (int deg = deg_begin; deg <= deg_end; deg += deg_step){ // branching angles, for b2
				//double deg = 60;
				m_scene->lSystem()->set_iterations(iter);
				m_scene->lSystem()->set_branchingAngle(double(deg));
				m_scene->draw_rules_random();
				QThread::msleep(100);

				//int x_steps = 2, y_steps = 2; //training
				int x_steps = 1, y_steps = 1; //test

				bool height_longer = (m_scene->m_tree_height > m_scene->m_tree_width) ? true : false;
				if (height_longer && m_scene->m_tree_height / m_scene->m_tree_width > 1.5){
					double ratio = m_scene->m_tree_height / m_scene->m_tree_width;
					y_steps = x_steps * ratio;
				}
				else if (height_longer && m_scene->m_tree_width / m_scene->m_tree_height > 1.5){
					double ratio = m_scene->m_tree_width / m_scene->m_tree_height;
					x_steps = y_steps*ratio;
				}

				x_steps = 1;
				y_steps = 1;

				m_xSteps_all = std::max(x_steps * 2, 1);
				m_ySteps_all = std::max(y_steps * 2, 1);

				qDebug() << m_xSteps_all << " -steps- " << m_ySteps_all;

				for (int i = -x_steps; i <= x_steps; i++){ // translate along X aix
					m_xStep = i;
					for (int j = -y_steps; j <= y_steps; j++){ //translate along Y aix
						m_yStep = j;
						/*scale_factor_ = m_scene->m_tree_scale_factor;
						if (scale_factor_ > 30.0){
						scale_factor_ = 30.0;
						}*/
						//scale_factor_ = 31; // for case-2-60 degree, scale
						//double s_begin = 0.35, s_end = 0.75, s_step = 0.1;
						//double s_begin = 0.2, s_end = 0.7, s_step = 0.1; //for b2
						double s_begin = 0.4, s_end = 0.7, s_step = 0.1; //for noise test
						//double s_begin = 0.6, s_end = 1.0, s_step = 0.1; // for branch parts
						if (iter == 2){
							s_begin = 0.6; s_end = 0.9; s_step = 0.1;
							//s_begin = 0.7; s_end = 0.9; s_step = 0.1; //for noise test
							//s_begin = 0.8; s_end = 1.1; s_step = 0.1; //for noise test
						}
						if (iter == 4){
							s_begin = 0.4; s_end = 0.8; s_step = 0.1;
							//s_begin = 0.6; s_end = 0.8; s_step = 0.1; //for noise test
							//s_begin = 0.7, s_end = 1.1, s_step = 0.1; // for branch parts
						}
						if (iter >= 5){
							s_begin = 0.8; s_end = 1.0; s_step = 0.1; //for noise test
						}
						double tree_maxlen = (m_scene->m_tree_height > m_scene->m_tree_width) ? m_scene->m_tree_height : m_scene->m_tree_width;
						double tree_minlen = (m_scene->m_tree_height < m_scene->m_tree_width) ? m_scene->m_tree_height : m_scene->m_tree_width;
						double ratiot = tree_maxlen / tree_minlen;
						if (ratiot > 5.0){
							s_begin = 0.6; s_end = 1.0; s_step = 0.2;
							//s_begin = 1.0; s_end = 1.0; s_step = 0.2; // for b1
						}
						//for (double sf = 0.35; sf <= 0.75; sf += 0.1){ // scale
						for (double sf = s_begin; sf <= s_end; sf += s_step){ // scale 
							scale_factor_ = m_scene->m_tree_scale_factor*sf;
							m_rotate_angle = 0.0;
							while (m_rotate_angle < 360){ //rotate
								repaint();
								m_rotate_angle += 30; //30 for training 60 for testing

								std::stringstream strStream_id;
								strStream_id << setw(6) << setfill('0') << m_image_id;

								//std::string annotations_name = "E:/work_SZU_ipml2d/DeepTree/code_projects/IPML-2d/outputFRCNN-tt/txts/s1-" + strStream_id.str() + ".txt";
								std::string annotations_name = "D:/SVN_Projects/IPML2d/code/out/txts/s1-" + strStream_id.str() + ".txt"; // my laptop path
								ofstream file;
								file.open(annotations_name);

								for (int k = 0; k < m_bbx.size(); k++){
									Bounding_box bbx = m_bbx[k];
									if (is_in_imageSpace(bbx.l_t_corner, width(), height()) && is_in_imageSpace(bbx.r_t_corner, width(), height())
										&& is_in_imageSpace(bbx.r_b_corner, width(), height()) && is_in_imageSpace(bbx.l_b_corner, width(), height())){
										int idx = deg / 10;
										//file << bbx.l_t_corner.x() << "," << bbx.l_t_corner.y() << "," << bbx.r_t_corner.x() << "," << bbx.r_t_corner.y() << "," <<
										//	bbx.r_b_corner.x() << "," << bbx.r_b_corner.y() << "," << bbx.l_b_corner.x() << "," << bbx.l_b_corner.y() << "," <<
										//	class_labels[idx - 1] << ",0" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
										//file << bbx.l_t_corner.x() << "," << bbx.l_t_corner.y() << "," << bbx.r_t_corner.x() << "," << bbx.r_t_corner.y() << "," <<
										//	bbx.r_b_corner.x() << "," << bbx.r_b_corner.y() << "," << bbx.l_b_corner.x() << "," << bbx.l_b_corner.y() << "," <<
										//	"b60,0" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
										file << bbx.l_t_corner.x() << "," << bbx.l_t_corner.y() << "," << bbx.r_t_corner.x() << "," << bbx.r_t_corner.y() << "," <<
											bbx.r_b_corner.x() << "," << bbx.r_b_corner.y() << "," << bbx.l_b_corner.x() << "," << bbx.l_b_corner.y() << "," <<
											brachClassId[bbx.class_id] << ",0" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
									}
								}
								file.close();
								m_image_id++;
							}
						}
					}
				}
			}
		}
	}

	qDebug() << "End rendering image...";
	m_render = false;
}

void GlViewer::render_images_all_FRCNN_random_test(){

	m_render = true;
	qDebug() << "Start rendering image...";

	std::srand(unsigned(std::time(0)));

	m_image_id = 0;
	for (int kk = 0; kk < 20; kk++){
		for (int iter = 2; iter <= 5; iter++){ // growth iterations
			//m_scene->lSystem()->select_rules_random();
			m_iteration = iter;
			int deg_begin = 30, deg_end = 80, deg_step = 10;
			/*if (iter == 2){
				deg_begin = 45, deg_end = 60, deg_step = 15;
			}*/
			//for (int deg = 30; deg <= (50 + kk * 10); deg += (5 + kk * 5)){ // branching angles
			//for (int deg = 40; deg <= 70; deg += 10){ // branching angles
			for (int deg = deg_begin; deg <= deg_end; deg += deg_step){ // branching angles, for b2
				//double deg = 60;
				m_scene->lSystem()->set_iterations(iter);
				m_scene->lSystem()->set_branchingAngle(double(deg));
				m_scene->draw_rules_random();
				QThread::msleep(100);

				double averge_branching_angle = m_scene->lSystem()->get_average_branch_angle();
				double average_scalar = m_scene->lSystem()->get_average_scalar();
				AssociativeArray used_grammar = m_scene->lSystem()->get_used_grammar();

				//int x_steps = 2, y_steps = 2; //training
				int x_steps = 1, y_steps = 1; //test

				bool height_longer = (m_scene->m_tree_height > m_scene->m_tree_width) ? true : false;
				if (height_longer && m_scene->m_tree_height / m_scene->m_tree_width > 1.5){
					double ratio = m_scene->m_tree_height / m_scene->m_tree_width;
					y_steps = x_steps * ratio;
				}
				else if (height_longer && m_scene->m_tree_width / m_scene->m_tree_height > 1.5){
					double ratio = m_scene->m_tree_width / m_scene->m_tree_height;
					x_steps = y_steps*ratio;
				}

				x_steps = 1;
				y_steps = 1;

				m_xSteps_all = std::max(x_steps * 2, 1);
				m_ySteps_all = std::max(y_steps * 2, 1);

				qDebug() << m_xSteps_all << " -steps- " << m_ySteps_all;

				//for (int i = -x_steps; i <= x_steps; i++){ // translate along X aix
					m_xStep = 0;
					//for (int j = -y_steps; j <= y_steps; j++){ //translate along Y aix
						m_yStep = 0;
						/*scale_factor_ = m_scene->m_tree_scale_factor;
						if (scale_factor_ > 30.0){
						scale_factor_ = 30.0;
						}*/
						//scale_factor_ = 31; // for case-2-60 degree, scale
						//double s_begin = 0.35, s_end = 0.75, s_step = 0.1;
						//double s_begin = 0.2, s_end = 0.7, s_step = 0.1; //for b2
						double s_begin = 0.6, s_end = 0.7, s_step = 0.1; //for noise test
						//double s_begin = 0.6, s_end = 1.0, s_step = 0.1; // for branch parts
						//if (iter == 2){
						//	s_begin = 0.6; s_end = 0.9; s_step = 0.1;
						//}
						//if (iter == 4){
						//	s_begin = 0.7, s_end = 1.1, s_step = 0.1; // for branch parts
						//}
						//if (iter >= 5){
						//	s_begin = 0.8; s_end = 1.0; s_step = 0.1; //for noise test
						//}
						double tree_maxlen = (m_scene->m_tree_height > m_scene->m_tree_width) ? m_scene->m_tree_height : m_scene->m_tree_width;
						double tree_minlen = (m_scene->m_tree_height < m_scene->m_tree_width) ? m_scene->m_tree_height : m_scene->m_tree_width;
						double ratiot = tree_maxlen / tree_minlen;
						if (ratiot > 5.0){
							//s_begin = 0.6; s_end = 1.0; s_step = 0.2;
							s_begin = 1.0; s_end = 1.0; s_step = 0.2; // for b1
						}
						//for (double sf = 0.35; sf <= 0.75; sf += 0.1){ // scale
						for (double sf = s_begin; sf <= s_end; sf += s_step){ // scale 
							scale_factor_ = m_scene->m_tree_scale_factor*sf;
							m_rotate_angle = 0.0;
							while (m_rotate_angle < 360){ //rotate
								repaint();
								m_rotate_angle += 360; //30 for training 60 for testing

								std::stringstream strStream_id;
								strStream_id << setw(6) << setfill('0') << m_image_id;

								// we output the original grammar information
								std::string grammar_info_name = "E:/work_SZU_ipml2d/DeepTree/code_projects/IPML-2d/outputFRCNN-tt/grammar_info/ps6-" + strStream_id.str() + ".txt";
								ofstream grammar_info_file;
								grammar_info_file.open(grammar_info_name);
								int grammar_len = 0;
								grammar_info_file << "The original grammar: " << std::endl;
								AssociativeArray::const_iterator iter;
								for (iter = used_grammar.begin(); iter != used_grammar.end(); ++iter)
								{
									string key = iter->first;
									vector<string> value = iter->second;
									for (int i = 0; i < value.size(); i++){
										grammar_info_file << key << "=" << value[i] << endl;
										grammar_len += key.length();
										for (int k = 0; k < value[i].length(); k++){
											if (value[i].at(k) == 'F' || value[i].at(k) == 'X' || value[i].at(k) == 'A' || value[i].at(k) == '+' ||
												value[i].at(k) == '-' || value[i].at(k) == '[' || value[i].at(k) == ']'){
												grammar_len += 1;
											}
										}
									}
								}
								grammar_info_file << std::endl;
								grammar_info_file << "Statistics of the grammar:" << std::endl;
								grammar_info_file << "    (1) grammar length : " << grammar_len << endl;
								grammar_info_file << "    (2) branching angle: " << averge_branching_angle << std::endl;
								grammar_info_file << "    (3) scaling  fator : " << average_scalar << std::endl;
								grammar_info_file.close();

								std::string annotations_name = "E:/work_SZU_ipml2d/DeepTree/code_projects/IPML-2d/outputFRCNN-tt/txts/ps7-" + strStream_id.str() + ".txt";
								ofstream file;
								file.open(annotations_name);

								for (int k = 0; k < m_bbx.size(); k++){
									Bounding_box bbx = m_bbx[k];
									if (is_in_imageSpace(bbx.l_t_corner, width(), height()) && is_in_imageSpace(bbx.r_t_corner, width(), height())
										&& is_in_imageSpace(bbx.r_b_corner, width(), height()) && is_in_imageSpace(bbx.l_b_corner, width(), height())){
										int idx = deg / 10;
										//file << bbx.l_t_corner.x() << "," << bbx.l_t_corner.y() << "," << bbx.r_t_corner.x() << "," << bbx.r_t_corner.y() << "," <<
										//	bbx.r_b_corner.x() << "," << bbx.r_b_corner.y() << "," << bbx.l_b_corner.x() << "," << bbx.l_b_corner.y() << "," <<
										//	class_labels[idx - 1] << ",0" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
										//file << bbx.l_t_corner.x() << "," << bbx.l_t_corner.y() << "," << bbx.r_t_corner.x() << "," << bbx.r_t_corner.y() << "," <<
										//	bbx.r_b_corner.x() << "," << bbx.r_b_corner.y() << "," << bbx.l_b_corner.x() << "," << bbx.l_b_corner.y() << "," <<
										//	"b60,0" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
										file << bbx.l_t_corner.x() << "," << bbx.l_t_corner.y() << "," << bbx.r_t_corner.x() << "," << bbx.r_t_corner.y() << "," <<
											bbx.r_b_corner.x() << "," << bbx.r_b_corner.y() << "," << bbx.l_b_corner.x() << "," << bbx.l_b_corner.y() << "," <<
											brachClassId[bbx.class_id] << ",0" << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
									}
								}
								file.close();
								m_image_id++;
							}
						}
					//}
				//}
			}
		}
	}

	qDebug() << "End rendering image...";
	m_render = false;
}

void GlViewer::render_current_image() { //only render current window

	m_render = true;
	qDebug() << "Start rendering image...";

	m_image_id = 0;

	m_scene->lSystem()->set_iterations(m_scene->lSystem()->iterations_);
	m_scene->lSystem()->set_branchingAngle(m_scene->lSystem()->branchingAngle_);
	if (m_scene->m_random_rules) {
		m_scene->draw_rules_random();
	}
	else {
		m_scene->draw_rules();
	}

	//int x_steps = 1, y_steps = 1; //test
	m_xSteps_all = std::max(2, 1);
	m_ySteps_all = std::max(2, 1);
	m_xStep = 0;
	m_yStep = 0;
	double s_begin = 0.6, s_end = 1.0, s_step = 0.1; // for branch parts
	m_image_id = 0;
	for (int iter = 2; iter <= 10; iter++) { // growth iterations
		m_iteration = iter;
		m_scene->lSystem()->set_iterations(iter);
		m_scene->draw_rules();
		double sf = 1.0;
		//for (double sf = s_begin; sf <= s_end; sf += s_step) { // scale 
		std::stringstream strStream_id;
		strStream_id << setw(6) << setfill('0') << m_image_id;

		scale_factor_ = m_scene->m_tree_scale_factor*sf;
		m_rotate_angle = 0.0;
		repaint();
		m_image_id++;
		//}
	}

	qDebug() << "End rendering image...";
	m_render = false;
}

void GlViewer::render_images_all_RetinaNet() { //for RetinaNet data format, we don't use it in the paper

	m_render = true;
	qDebug() << "Start rendering image...";

	m_image_id = 0;
	string class_labels[8] = { "b10", "b20", "b30", "b40", "b50", "b60", "b70", "b80" };

	for (int iter = 1; iter <= 3; iter++) { // growth iterations
											//for (int deg = 20; deg <= 80; deg += 10){ // branching angles
		double deg = 60;
		m_scene->lSystem()->set_iterations(iter);
		m_scene->lSystem()->set_branchingAngle(double(deg));
		m_scene->draw_rules();

		int x_steps = 2, y_steps = 2; //training
									  //int x_steps = 1, y_steps = 1; //test

		bool height_longer = (m_scene->m_tree_height > m_scene->m_tree_width) ? true : false;
		if (height_longer && m_scene->m_tree_height / m_scene->m_tree_width > 1.5) {
			double ratio = m_scene->m_tree_height / m_scene->m_tree_width;
			y_steps = x_steps * ratio;
		}
		else if (height_longer && m_scene->m_tree_width / m_scene->m_tree_height > 1.5) {
			double ratio = m_scene->m_tree_width / m_scene->m_tree_height;
			x_steps = y_steps*ratio;
		}

		m_xSteps_all = x_steps * 2;
		m_ySteps_all = y_steps * 2;
		qDebug() << m_xSteps_all << " -steps- " << m_ySteps_all;

		std::string annotations_name = "../outputRetinaNew-tt/annotations.csv";
		ofstream file;
		file.open(annotations_name, ios::app);

		for (int i = -x_steps; i <= x_steps; i++) { // translate along X aix
			m_xStep = i;
			for (int j = -y_steps; j <= y_steps; j++) { //translate along Y aix
				m_yStep = j;
				//scale_factor_ = m_scene->m_tree_scale_factor;
				scale_factor_ = 15.1704;
				//for (double sf = 0.5; sf <= 1.0; sf += 0.1){ // scale 
				//scale_factor_ = m_scene->m_tree_scale_factor*sf;
				m_rotate_angle = 0.0;
				while (m_rotate_angle < 360) { //rotate
					repaint();

					std::stringstream strStream_id;
					strStream_id << setw(6) << setfill('0') << m_image_id;
					//std::string s_id = "JPEGImages/" + strStream_id.str() + ".jpg";
					std::string s_id = strStream_id.str() + ".jpg";

					for (int k = 0; k < m_bbx.size(); k++) {
						Bounding_box bbx = m_bbx[k];
						int xmin = (int)bbx.x_min, xmax = (int)bbx.x_max, ymin = (int)bbx.y_min, ymax = (int)bbx.y_max;
						int xmin_r = (int)(bbx.center.x() - bbx.width*0.5), xmax_r = (int)(bbx.center.x() + bbx.width*0.5);
						int ymin_r = (int)(bbx.center.y() - bbx.height*0.5), ymax_r = (int)(bbx.center.y() + bbx.height*0.5);
						double angle = bbx.angleFromY + m_rotate_angle*M_PI / 180.0;
						//double angle = bbx.angleFromY;
						if (angle >= M_PI) {
							angle -= 2 * M_PI;
						}
						if (angle < -M_PI) {
							angle += 2 * M_PI;
						}
						if (xmin > 0 && ymin >0 && xmin < xmax && ymin < ymax && xmax < width() && ymax < height()) {
							if (xmin_r > 0 && ymin_r > 0 && xmin_r < xmax_r && ymin_r < ymax_r && xmax_r < width() && ymax_r < height()) {
								int idx = deg / 10;
								//file << s_id << "," << xmin << "," << ymin << "," << xmax << "," << ymax << "," << angle << "," << class_labels[idx - 1] << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
								file << s_id << "," << xmin_r << "," << ymin_r << "," << xmax_r << "," << ymax_r << "," << angle << "," << class_labels[idx - 1] << std::endl; //path/to/image.jpg, x1, y1, x2, y2, class_name
							}
						}
					}
					m_image_id++;
					m_rotate_angle += 60; //10 for training 60 for testing
				}
				//}
			}
		}
		file.close();
		//}
	}

	qDebug() << "End rendering image...";
	m_render = false;
}



/////////////////////////////////Grammar inference/////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////

inline int get_calss_id(std::string name){
	int id = 0;
	if (name == "b60"){
		id = 0;
	}
	else if (name == "b1"){
		id = 1;
	}
	else if (name == "b2"){
		id = 2;
	}
	else if (name == "b3"){
		id = 3;
	}
	else if (name == "b4"){
		id = 4;
	}
	else if (name == "b5"){
		id = 5;
	}
	else if (name == "b6"){
		id = 6;
	}
	else if (name == "b7"){
		id = 7;
	}
	else if (name == "b8"){
		id = 8;
	}
	return id;
}

//load image and corresponding detected bboxs
void GlViewer::parsing_image(const QString& filename){
	m_parse_img.load(filename);

	m_save_image_name = filename.toStdString();
	m_save_image_name.replace(m_save_image_name.end() - 4, m_save_image_name.end(), "_bbx.jpg");

	m_bbx_parse.clear();
	m_min_len = 1000;
	std::string bbox_name = filename.toStdString();
	bbox_name.replace(bbox_name.end() - 3, bbox_name.end(), "txt");
	std::ifstream in(bbox_name.c_str());
	string class_name;
	float score, x_c, y_c, w, h, angle;
	while (in) {
		in >> class_name >> score >> x_c >> y_c >> w >> h >> angle;
		if (in){
			Bounding_box_parse bbx;
			bbx.center_position = R2Vector(x_c, y_c);
			bbx.label_id = get_calss_id(class_name);
			//here we make two strong assumptions about the rotation
			if (w > h){
				bbx.width = h;
				bbx.height = w; // +8;
				bbx.angleFromY = -angle + 90; //in image coordinates, this angle is from Y-axis to the direction of this bbx  
				bbx.angleFromY_opp = -(90 + angle);
			}
			else{
				bbx.width = w;
				bbx.height = h; // +8;
				bbx.angleFromY = -angle - 180; //-90-(180-90+angle)
				bbx.angleFromY_opp = -angle;
			}

			m_min_len = std::min(m_min_len, bbx.height);

			QPointF ltc(-bbx.width*0.5, -bbx.height*0.5);
			QPointF rtc(bbx.width*0.5, -bbx.height*0.5);
			QPointF rbc(bbx.width*0.5, bbx.height*0.5);
			QPointF lbc(-bbx.width*0.5, bbx.height*0.5);
			QPointF dir(0, 1);

			ltc = my_rotate(ltc, bbx.angleFromY);
			rtc = my_rotate(rtc, bbx.angleFromY);
			rbc = my_rotate(rbc, bbx.angleFromY);
			lbc = my_rotate(lbc, bbx.angleFromY);
			dir = my_rotate(dir, bbx.angleFromY);

			bbx.l_t_corner = R2Vector(bbx.center_position.X() + ltc.x(), bbx.center_position.Y() + ltc.y());
			bbx.r_t_corner = R2Vector(bbx.center_position.X() + rtc.x(), bbx.center_position.Y() + rtc.y());
			bbx.r_b_corner = R2Vector(bbx.center_position.X() + rbc.x(), bbx.center_position.Y() + rbc.y());
			bbx.l_b_corner = R2Vector(bbx.center_position.X() + lbc.x(), bbx.center_position.Y() + lbc.y());
			bbx.direction = R2Vector(dir.x(), dir.y());
			bbx.direction_opp = R2Vector(-dir.x(), -dir.y());

			bbx.center_position_LC = R2Vector(bbx.center_position.X(), -bbx.center_position.Y());
			bbx.l_t_corner_LC = R2Vector(bbx.l_t_corner.X(), -bbx.l_t_corner.Y());
			bbx.r_t_corner_LC = R2Vector(bbx.r_t_corner.X(), -bbx.r_t_corner.Y());
			bbx.r_b_corner_LC = R2Vector(bbx.r_b_corner.X(), -bbx.r_b_corner.Y());
			bbx.l_b_corner_LC = R2Vector(bbx.l_b_corner.X(), -bbx.l_b_corner.Y());
			bbx.direction_LC = R2Vector(bbx.direction.X(), -bbx.direction.Y());
			bbx.direction_LC_opp = R2Vector(bbx.direction_opp.X(), -bbx.direction_opp.Y());
			//bbx.angleFromY_LC = std::min(0.0, std::abs(RAD2DEG(angleFromY_LC(bbx.direction_LC))));
			bbx.angleFromY_LC = RAD2DEG(angleFromY_LC(bbx.direction_LC));
			bbx.angleFromY_LC_opp = RAD2DEG(angleFromY_LC(bbx.direction_LC_opp));

			m_bbx_parse.push_back(bbx);
		}
	};
	in.close();

	repaint();
}

struct pairwise_bbx{
	int i, j;
	double weight;
	pairwise_bbx(int x, int y, double dis) : weight(dis){
		i = (x < y) ? x : y;
		j = (x < y) ? y : x;
	}
	bool operator==(const pairwise_bbx b) const{
		return (i == b.i && j == b.j);
	}

	bool operator <(const pairwise_bbx& b) const{
		bool flag = (i < b.i) || (!(i<b.i) && (j<b.j));
		return flag;
	}

	bool operator () (const pairwise_bbx& b) const{
		return (i == b.i && j == b.j);
	}
};

//build the graph structure
void GlViewer::build_graph(){
	m_graph.clear();

	std::vector<int> nodes_list;
	std::vector<bool> visited;
	m_root_id = 0;
	double min_y = 100000;
	//build tree nodes
	std::vector<pairwise_bbx> adjenct_bbx;
	std::cout << "number of BBX: " << m_bbx_parse.size() << std::endl;
	for (int i = 0; i < m_bbx_parse.size(); i++){
		Bounding_box_parse cur_bbx = m_bbx_parse[i];

		//std::cout << "i: " << i << std::endl;
		for (int j = 0; j < m_bbx_parse.size(); j++){
			if (i == j) continue;
			//std::cout << "j: " << j << std::endl;
			bool intersect = check_bbox_intersect(m_bbx_parse[i], m_bbx_parse[j]);
			double pair_weight = compute_relative_distance(m_bbx_parse[i], m_bbx_parse[j]);

			//if (intersect){
			//	//boost::add_edge(i, j, 0.0, m_graph);
			//	pairwise_bbx pb(i, j, 0.0);
			//	if (std::find(adjenct_bbx.begin(), adjenct_bbx.end(), pb) == adjenct_bbx.end()){
			//		adjenct_bbx.push_back(pb);
			//	}
			//	std::cout << "True: " << pair_weight << std::endl;
			//}
			//else{
			//	std::cout << "False: " << pair_weight << std::endl;
			//}

			if (pair_weight < m_weight_thrs){
				//boost::add_edge(i, j, 0.0, m_graph);
				pairwise_bbx pb(i, j, pair_weight);
				if (std::find(adjenct_bbx.begin(), adjenct_bbx.end(), pb) == adjenct_bbx.end()){
					adjenct_bbx.push_back(pb);
				}
				//std::cout << "True: " << intersect << std::endl;
			}
			else{
				//std::cout << "False: " << intersect << std::endl;
			}

		}
		if (m_bbx_parse[i].center_position_LC.Y() < min_y){
			min_y = m_bbx_parse[i].center_position_LC.Y();
			m_root_id = i;
		}
		nodes_list.push_back(i);
		visited.push_back(false);
	}

	for (int i = 0; i<adjenct_bbx.size(); i++){
		pairwise_bbx cur_pb = adjenct_bbx[i];
		boost::add_edge(cur_pb.i, cur_pb.j, cur_pb.weight, m_graph);
	}

	std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<UndirectedGraph, vertex_t>(m_graph);
	udgcd::PrintPaths(std::cout, cycles);

	std::cout << "Build graph done!" << std::endl;

	/*std::pair<edge_iterator, edge_iterator> ei = edges(m_graph);
	std::cout << "Number of edges = " << num_edges(m_graph) << "\n";
	std::cout << "Edge list:\n";
	for (edge_iterator it = ei.first; it != ei.second; ++it)
	{
	std::cout << *it << std::endl;
	}
	UndirectedGraph::adjacency_iterator vit, vend;
	std::tie(vit, vend) = boost::adjacent_vertices(2, m_graph);
	std::copy(vit, vend,
	std::ostream_iterator<UndirectedGraph::vertex_descriptor>{std::cout, "\n"});
	std::cout << std::endl;*/

	//build_NaryTree();
}

void GlViewer::remove_cycles(){
	int iters = 0;
	bool has_cycles = false;
	do{
		//find cycles
		std::vector<std::vector<vertex_t>> cycles = udgcd::FindCycles<UndirectedGraph, vertex_t>(m_graph);
		if (cycles.size() == 0) break;
		std::cout << "Cycles number: " << cycles.size() << std::endl;
		udgcd::PrintPaths(std::cout, cycles);
		has_cycles = true;
		iters++;

		// Create property_map from edges to weights
		boost::property_map<UndirectedGraph, boost::edge_weight_t>::type weightmap = get(boost::edge_weight, m_graph);
		std::vector<vertex_t> p(boost::num_vertices(m_graph));
		std::vector<double> d(num_vertices(m_graph));
		boost::dijkstra_shortest_paths(m_graph, m_root_id, boost::predecessor_map(&p[0]).distance_map(&d[0]));

		//break cycles
		for (int i = 0; i < cycles.size(); i++){

			if (cycles[i].size() == 3){
				double min_distance = 10000;
				int indx;
				for (int k = 0; k < 3; k++){
					int goal = cycles[i][k];
					std::vector<int> nodes;
					double real_dis = 0;
					while (goal != m_root_id){
						nodes.push_back(goal);
						real_dis += d[goal];
						goal = p[goal];
					}

					if (min_distance > nodes.size()){
						//if (max_distance < real_dis){
						min_distance = nodes.size();
						//max_distance = real_dis;
						indx = k;
					}
				}
				if (indx == 0){
					//int left = cycles[i][1], right = cycles[i][2];
					boost::remove_edge(cycles[i][1], cycles[i][2], m_graph);
				}
				else if (indx == 1){
					boost::remove_edge(cycles[i][0], cycles[i][2], m_graph);
				}
				else{
					boost::remove_edge(cycles[i][0], cycles[i][1], m_graph);
				}

				continue;
			}

			std::vector<std::pair<int, int>> remove_edges;

			for (int j = 0; j < cycles[i].size(); j++){
				int b_left, b_middle, b_right;
				if (j == 0){
					b_left = cycles[i][cycles[i].size() - 1], b_middle = cycles[i][j], b_right = cycles[i][j + 1];
				}
				else{
					b_left = cycles[i][j - 1], b_middle = cycles[i][j], b_right = cycles[i][(j + 1) % cycles[i].size()];
				}
				R2Vector bm_to_bl = m_bbx_parse[b_left].center_position_LC - m_bbx_parse[b_middle].center_position_LC;
				R2Vector bm_to_br = m_bbx_parse[b_right].center_position_LC - m_bbx_parse[b_middle].center_position_LC;
				double dot_value = bm_to_bl.Dot(bm_to_br);
				double cos_value = dot_value / (bm_to_bl.Length() * bm_to_br.Length());
				double angle_1 = RAD2DEG(std::acos(cos_value));
				if (angle_1 < 90.0){
					dot_value = m_bbx_parse[b_middle].direction_LC.Dot(m_bbx_parse[b_left].direction_LC);
					cos_value = dot_value / (m_bbx_parse[b_middle].direction_LC.Length() * m_bbx_parse[b_left].direction_LC.Length());
					double angle_left = RAD2DEG(std::acos(cos_value));

					dot_value = m_bbx_parse[b_middle].direction_LC.Dot(m_bbx_parse[b_right].direction_LC);
					cos_value = dot_value / (m_bbx_parse[b_middle].direction_LC.Length() * m_bbx_parse[b_right].direction_LC.Length());
					double angle_right = RAD2DEG(std::acos(cos_value));

					if (angle_left < angle_right){
						remove_edges.push_back(std::make_pair(b_right, b_middle));
					}
					else{
						remove_edges.push_back(std::make_pair(b_left, b_middle));
					}
				}
			}

			if (remove_edges.size() > 0){
				double max_distance = -10000;
				int indx;

				for (int k = 0; k < remove_edges.size(); k++){
					int goal = remove_edges[k].second;
					std::vector<int> nodes;
					std::vector<float> distances;
					double real_dis = 0;
					while (goal != m_root_id){
						nodes.push_back(goal);
						distances.push_back(d[goal]);
						real_dis += d[goal];
						goal = p[goal];
					}

					if (max_distance < nodes.size()){
						//if (max_distance < real_dis){
						max_distance = nodes.size();
						//max_distance = real_dis;
						indx = k;
					}
				}
				boost::remove_edge(remove_edges[indx].first, remove_edges[indx].second, m_graph);
			}
		}

	} while (has_cycles || iters < 5);

	std::cout << "Break cycles done! Iters: " << iters << std::endl;
}

void GlViewer::extract_minimal_spanning_tree(){
	boost::property_map<UndirectedGraph, boost::edge_weight_t >::type weight = get(boost::edge_weight, m_graph);
	std::vector<edge_descriptor> spanning_tree;

	boost::kruskal_minimum_spanning_tree(m_graph, std::back_inserter(spanning_tree));

	UndirectedGraph new_graph;
	for (std::vector<edge_descriptor>::iterator ei = spanning_tree.begin();
		ei != spanning_tree.end(); ++ei)
	{
		boost::add_edge(source(*ei, m_graph), target(*ei, m_graph), weight[*ei], new_graph);
	}

	m_graph.clear();
	m_graph = new_graph;
}

bool inline Nary_GreaterSort(Nary_TreeNode* a, Nary_TreeNode* b) {
	return (a->bbx.angleFromParent>b->bbx.angleFromParent);
}
bool inline Nary_LessSort(Nary_TreeNode* a, Nary_TreeNode* b) {
	return (a->bbx.angleFromParent<b->bbx.angleFromParent);
}

// the main function for inference
void GlViewer::build_NaryTree(){

	std::vector<Nary_TreeNode*> nodes_list;
	std::vector<bool> visited;
	std::vector<int> parents;
	//build tree nodes
	for (int i = 0; i < m_bbx_parse.size(); i++){
		Bounding_box_parse cur_bbx = m_bbx_parse[i];
		Nary_TreeNode* cur_node = CreateNaryTreeNode(m_bbx_parse[i]);
		cur_node->bbx_index = i + 1;
		nodes_list.push_back(cur_node);
		visited.push_back(false);
	}
	//consturct the tree
	Nary_TreeNode* root_node = nodes_list[m_root_id];
	root_node->bbx.angleFromParent = root_node->bbx.angleFromY_LC;
	visited[m_root_id] = true;

	queue<Nary_TreeNode *> qt;
	qt.push(root_node);
	typedef boost::property_map<UndirectedGraph, boost::vertex_index_t>::type IndexMap;
	IndexMap index = get(boost::vertex_index, m_graph);

	while (!qt.empty()){
		Nary_TreeNode* cur_node = qt.front();
		qt.pop();
		if (cur_node){
			UndirectedGraph::adjacency_iterator vit, vend;
			std::vector<Nary_TreeNode*> children_list;
			for (std::tie(vit, vend) = boost::adjacent_vertices(cur_node->bbx_index - 1, m_graph); vit != vend; ++vit){
				int adj_idx = index[*vit];
				if (!visited[adj_idx]){
					double p1 = nodes_list[adj_idx]->bbx.angleFromY_LC - cur_node->bbx.angleFromY_LC;
					double p2 = nodes_list[adj_idx]->bbx.angleFromY_LC_opp - cur_node->bbx.angleFromY_LC;
					nodes_list[adj_idx]->bbx.angleFromParent = abs(p1) < abs(p2) ? p1 : p2;
					children_list.push_back(nodes_list[adj_idx]);
					visited[adj_idx] = true;
				}
			}

			std::sort(children_list.begin(), children_list.end(), Nary_GreaterSort);
			int main_branch_idx = -1; // we find which child should be the main branch
			double min_abs_anbgle = 10000;
			for (int i = 0; i < children_list.size(); i++){
				if (abs(children_list[i]->bbx.angleFromParent) < m_mianBranchAngle_thrs){
					if (abs(children_list[i]->bbx.angleFromParent) < min_abs_anbgle){
						min_abs_anbgle = abs(children_list[i]->bbx.angleFromParent);
						main_branch_idx = i;
					}
				}
			}

			for (int i = 0; i < children_list.size(); i++){
				if (i == main_branch_idx){
					children_list[i]->main_branch = true;
					children_list[i]->turn_indicator = "0";
				}
				else if (children_list[i]->bbx.angleFromParent > 0){
					children_list[i]->turn_indicator = "1";
				}
				else if (children_list[i]->bbx.angleFromParent < 0){
					children_list[i]->turn_indicator = "-1";
				}
				ConnectTreeNodes(cur_node, children_list[i]);
				qt.push(cur_node->children[i]);
			}
		}
	}

	/*std::cout << "Build tree done! The initial tree: " << std::endl;
	Nary_print_tree(root_node, 0);*/

	Nary_compute_strahler_number(root_node);
	bool output_strahler_number = true;

	//write expansion grammer
	std::cout << "Expansion grammar (input string) with parameters: " << std::endl;
	string expansion_grammer_withParas;
	//expansion_grammer_withParas = Nary_write_grammar(root_node, true);
	expansion_grammer_withParas = Nary_write_grammar_forPaper(root_node, true);
	std::cout << expansion_grammer_withParas << std::endl;

	std::cout << "Expansion grammar (input string) without parameters: " << std::endl;
	string expansion_grammer = Nary_write_grammar(root_node, false);
	std::cout << expansion_grammer << std::endl;
	std::cout << std::endl;

	//get the scale and branching angle parameters
	m_tree_node_size_ = 0;
	m_average_branch_angle_ = 0.0;
	m_tree_node_size_angle_ = 0;
	double scalar_para = Nary_get_scalar_para(root_node);
	scalar_para /= m_tree_node_size_;
	m_average_branch_angle_ /= m_tree_node_size_angle_;

	m_rules.clear();

	QTime timedebuge;
	timedebuge.start();

	if (m_optAlgorithm_ == "Our"){
		//inference using our method
		m_alphabet_pointer = 2;
		unordered_map<string, Nary_repetition_node> m;
		m_selected_repetitions.clear();
		m_selected_repetitions_nary_nodes.clear();
		m_used_symbols.clear();
		Nary_generate_conformal_grammar(root_node, m);
		// write the last rule
		ruleSuccessor rule = Nary_write_rules(root_node, true);
		m_rules[alphabet[1]].push_back(rule);
		std::cout << alphabet[1] << " -> " << rule.successor << std::endl;
		qDebug() << "Our Greedy Time: " << timedebuge.elapsed() / 1000.0 << "s";//输出计时
	}
	else if (m_optAlgorithm_ == "SA"){
		//inference using Simulated Annealing
		m_alphabet_pointer = 1;
		string testString = expansion_grammer;
		//string testString = "F[+F[+F[+F[+F][-F]][-F[+F][-F]]][-F[+F[+F][-F]][-F[+F][-F]]]][-F[+F[+F[+F][-F]][-F[+F][-F]]][-F[+F[+F][-F]][-F[+F][-F]]]]";
		Nary_generate_compact_grammar_simulatedAnnealing(alphabet[m_alphabet_pointer], testString);

		qDebug() << "SA Time: " << timedebuge.elapsed() / 1000.0 << "s";//输出计时
	}
	else if (m_optAlgorithm_ == "GA"){
		//inference using Genetic Algorithms
		m_alphabet_pointer = 1;
		string testString = expansion_grammer;
		Nary_generate_compact_grammar_geneticAlgorithms(alphabet[m_alphabet_pointer], testString);

		qDebug() << "GA Time: " << timedebuge.elapsed() / 1000.0 << "s";//输出计时
	}

	std::cout << std::endl;

	//print grammer
	std::cout << "The compact conformal grammar: " << std::endl;
	newAssociativeArray::const_iterator iter;
	int compact_grammar_len = 0;
	for (iter = m_rules.begin(); iter != m_rules.end(); ++iter)
	{
		string left = iter->first;
		vector<ruleSuccessor> sucs = iter->second;
		for (int i = 0; i < sucs.size(); i++){
			string right = sucs[i].successor;
			cout << left << " = " << right << endl;
			compact_grammar_len += left.length();
			compact_grammar_len += right.length();
		}
	}
	std::cout << std::endl;

	//merge similar rules
	grammar_induction();

	//write the last rule about the paramter
	std::ostringstream streamObj3;
	streamObj3 << std::fixed; // Set Fixed -Point Notation
	streamObj3 << std::setprecision(2); // Set precision to 2 digits
	streamObj3 << scalar_para; //Add double to stream
	//string succ = "<*(" + streamObj3.str() + ")F>";
	string succ = "F(*" + streamObj3.str() + ")";
	std::cout << "F->" << succ << std::endl;

	std::cout << std::endl;

	std::cout << "Statistics of the grammar inference:" << std::endl;
	std::cout << "    (1) expansion grammar length: " << expansion_grammer.length() << std::endl;
	std::cout << "    (2) conformal grammar length: " << compact_grammar_len << std::endl;
	std::cout << "    (3) final     grammar length: " << m_final_grammar_length_ << endl;
	std::cout << "    (4) average branching angle : " << m_average_branch_angle_ << std::endl;
	std::cout << "    (5) average scaling  fator  : " << scalar_para << std::endl;
}

bool GlViewer::check_bbox_intersect(Bounding_box_parse b1, Bounding_box_parse b2){
	bool is_intersect = false;

	// first, do a very fast check to remove most of the cases
	double distance = (b1.center_position_LC - b2.center_position_LC).Length();
	if (distance > 2.3 * b1.height && distance > 2.3 * b2.height){
		return false;
	}

	//second, hadnle the tangency case
	t_line b1_lines[4] = { t_line(b1.l_t_corner_LC, b1.r_t_corner_LC), t_line{b1.r_t_corner_LC, b1.r_b_corner_LC},
	                       t_line(b1.r_b_corner_LC, b1.l_b_corner_LC), t_line(b1.l_b_corner_LC, b1.l_t_corner_LC)};
	t_line b2_lines[4] = { t_line(b2.l_t_corner_LC, b2.r_t_corner_LC), t_line{b2.r_t_corner_LC, b2.r_b_corner_LC },
						   t_line(b2.r_b_corner_LC, b2.l_b_corner_LC), t_line(b2.l_b_corner_LC, b2.l_t_corner_LC) };
	for (int i = 0; i < 4; i++){
		t_line e1 = b1_lines[i]; 
		for (int j = 0; j < 4; j++){
			t_line e2 = b2_lines[j];
			if (((e1.p_start - e2.p_start).Length() < 0.15*b1.width && (e1.p_end - e2.p_end).Length() < 0.15*b1.width) ||
				((e1.p_start - e2.p_end).Length() < 0.15*b1.width && (e1.p_end - e2.p_start).Length() < 0.15*b1.width)){
				return true;
			}
			R2Vector mid1 = (e1.p_start + e1.p_end)*0.5;
			R2Vector mid2 = (e2.p_start + e2.p_end)*0.5;
			if ((mid1 - mid2).Length() < 0.15*b1.width){
				return true;
			}
		}
	}

	// third, do the exact test
	QPolygonF a, b;
	a << QPointF(b1.l_t_corner_LC.X(), b1.l_t_corner_LC.Y()) << QPointF(b1.r_t_corner_LC.X(), b1.r_t_corner_LC.Y())
		<< QPointF(b1.r_b_corner_LC.X(), b1.r_b_corner_LC.Y()) << QPointF(b1.l_b_corner_LC.X(), b1.l_b_corner_LC.Y());
	b << QPointF(b2.l_t_corner_LC.X(), b2.l_t_corner_LC.Y()) << QPointF(b2.r_t_corner_LC.X(), b2.r_t_corner_LC.Y())
		<< QPointF(b2.r_b_corner_LC.X(), b2.r_b_corner_LC.Y()) << QPointF(b2.l_b_corner_LC.X(), b2.l_b_corner_LC.Y());

	for (int polyi = 0; polyi < 2; ++polyi)
	{
		const QPolygonF& polygon = polyi == 0 ? a : b;

		for (int i1 = 0; i1 < polygon.size(); ++i1)
		{
			const int i2 = (i1 + 1) % polygon.size();

			const double normalx = polygon[i2].y() - polygon[i1].y();
			const double normaly = polygon[i1].x() - polygon[i2].x();

			double minA = std::numeric_limits<double>::max();
			double maxA = std::numeric_limits<double>::min();
			for (int ai = 0; ai < a.size(); ++ai)
			{
				const double projected = normalx * a[ai].x() +
					normaly * a[ai].y();
				if (projected < minA) minA = projected;
				if (projected > maxA) maxA = projected;
			}

			double minB = std::numeric_limits<double>::max();
			double maxB = std::numeric_limits<double>::min();
			for (int bi = 0; bi < b.size(); ++bi)
			{
				const double projected = normalx * b[bi].x() +
					normaly * b[bi].y();
				if (projected < minB) minB = projected;
				if (projected > maxB) maxB = projected;
			}

			if (maxA < minB || maxB < minA)
				return false;
		}
	}

	return true;
}

double GlViewer::compute_relative_distance(Bounding_box_parse b1, Bounding_box_parse b2){

	double pair_weight = 100;
	double max_height = (b1.height > b2.height) ? b1.height : b2.height;
	// first, do a very fast check to remove most of the cases
	double cc_distance = (b1.center_position_LC - b2.center_position_LC).Length();
	if (cc_distance > m_centerDis_thrs * max_height){
	//if (cc_distance > (1.0 * (b1.height + b2.height))){
		return pair_weight;
	}

	//second, do another check by using bbx corners to remove some cases
	bool intersect = check_bbox_intersect(b1, b2);
	if (!intersect){
		t_line b1_lines[4] = { t_line(b1.l_t_corner_LC, b1.r_t_corner_LC), t_line{ b1.r_t_corner_LC, b1.r_b_corner_LC },
			t_line(b1.r_b_corner_LC, b1.l_b_corner_LC), t_line(b1.l_b_corner_LC, b1.l_t_corner_LC) };
		t_line b2_lines[4] = { t_line(b2.l_t_corner_LC, b2.r_t_corner_LC), t_line{ b2.r_t_corner_LC, b2.r_b_corner_LC },
			t_line(b2.r_b_corner_LC, b2.l_b_corner_LC), t_line(b2.l_b_corner_LC, b2.l_t_corner_LC) };

		double min_dis = 10000;
		for (int i = 0; i < 4; i++){
			t_line e1 = b1_lines[i];
			for (int j = 0; j < 4; j++){
				t_line e2 = b2_lines[j];
				min_dis = std::min(min_dis, (e1.p_start - e2.p_start).Length());
				min_dis = std::min(min_dis, (e1.p_end - e2.p_end).Length());
				min_dis = std::min(min_dis, (e1.p_start - e2.p_end).Length());
				min_dis = std::min(min_dis, (e1.p_end - e2.p_start).Length());

				R2Vector mid1 = (e1.p_start + e1.p_end)*0.5;
				R2Vector mid2 = (e2.p_start + e2.p_end)*0.5;
				min_dis = std::min(min_dis, (mid1 - mid2).Length());
			}
		}
		if (min_dis > m_cornerDis_thrs*max_height){
			return pair_weight;
		}
	}

	////third, if the angle between two boxes are very large, we remove it
	//double dot_value = b1.direction_LC.Dot(b2.direction_LC);
	//double cos_value = dot_value / (b1.direction_LC.Length() * b2.direction_LC.Length());
	//double angle = RAD2DEG(std::acos(cos_value));
	//if (angle > 90.0){
	//	return pair_weight;
	//}
	
	// we compute the relative distance 
	R2Vector b2_to_b1 = (b1.center_position_LC - b2.center_position_LC);
	//b2_to_b1.Normalize();
	R2Vector b1_to_b2 = (b2.center_position_LC - b1.center_position_LC);
	//b1_to_b2.Normalize();
	R2Vector projection_vec_on_b1 = b2_to_b1;
	R2Vector projection_vec_on_b2 = b1_to_b2;
	projection_vec_on_b1.Project(b1.direction_LC);
	projection_vec_on_b2.Project(b2.direction_LC);
	double projection_dis_on_b1 = projection_vec_on_b1.Length();
	double projection_dis_on_b2 = projection_vec_on_b2.Length();

	pair_weight = std::abs((projection_dis_on_b1 + projection_dis_on_b2) - (b1.height + b2.height)) / (0.5*(b1.height + b2.height));

	return pair_weight;
}

//print tree
void GlViewer::Nary_print_tree(Nary_TreeNode *root, int spaces) {

	int loop;
	if (root != NULL) {

		for (loop = 1; loop <= spaces; loop++) {
			printf(" ");
		}

		printf("%d\n", root->bbx_index);
	}
	
	int main_branch_idx = -1;
	for (int i = 0; i < root->children.size(); i++){
		if (root->children[i]->main_branch){
			main_branch_idx = i;
			continue;
		}
		Nary_print_tree(root->children[i], spaces + 4);
	}
	if (main_branch_idx != -1){ 
		Nary_print_tree(root->children[main_branch_idx], spaces + 4);;
	}
}

string GlViewer::Nary_write_grammar(Nary_TreeNode *root, bool with_paras) {
	string str = "";
	if (root == NULL)
		return str;

	std::ostringstream streamObj1, streamObj2, streamObj3;

	streamObj3 << std::fixed; // Set Fixed -Point Notation
	streamObj3 << std::setprecision(1); // Set precision to 2 digits
	streamObj3 << abs(root->bbx.angleFromParent); //Add double to stream
	string turn_angle = streamObj3.str(); // Get string from output string stream

	streamObj1 << std::fixed; // Set Fixed -Point Notation
	streamObj1 << std::setprecision(1); // Set precision to 2 digits
	double scaler = root->bbx.height/m_min_len;
	streamObj1 << scaler; //Add double to stream
	string turn_scaler = streamObj1.str(); // Get string from output string stream

	streamObj2 << std::fixed; // Set Fixed -Point Notation
	streamObj2 << std::setprecision(1); // Set precision to 2 digits
	streamObj2 << abs(root->strahler_number); //Add double to stream
	string strahler_number = streamObj2.str(); // Get string from output string stream

	if (abs(root->bbx.angleFromParent) < m_mianBranchAngle_thrs){
		if (with_paras){
			//string prefix = "<*(" + turn_scaler + ")F>";
			string prefix = "<*(" + turn_scaler + ")&(" + strahler_number + ")F>";
			str += prefix;
		}
		else{
			str += "F";
		}
		
		int main_branch_idx = -1;
		for (int i = 0; i < root->children.size(); i++){
			if (root->children[i]->main_branch){
				main_branch_idx = i;
				continue;
			}
			str += Nary_write_grammar(root->children[i], with_paras);
		}
		if (main_branch_idx != -1){
			str += Nary_write_grammar(root->children[main_branch_idx], with_paras);
		}
	}
	else if (root->bbx.angleFromParent > 0){
		if (with_paras){
			//str += "[+(" + std::to_string(root->bbx.angleFromParent) + ")F";
			string prefix1 = "<*(" + turn_scaler + ")&(" + strahler_number + ")F>";
			string prfix = "[+(" + turn_angle + ")" + prefix1;
			str += prfix;
		}
		else{
			str += "[+F";
		}

		int main_branch_idx = -1;
		for (int i = 0; i < root->children.size(); i++){
			if (root->children[i]->main_branch){
				main_branch_idx = i;
				continue;
			}
			str += Nary_write_grammar(root->children[i], with_paras);
		}
		if (main_branch_idx != -1){
			str += Nary_write_grammar(root->children[main_branch_idx], with_paras);
		}
		str += "]";
		//std::cout << "[+F]";
	}
	else if (root->bbx.angleFromParent < 0){
		//std::cout << "[-(" << -root->bbx.angleFromParent << ")F]";
		//str += "[-(" + std::to_string(root->bbx.angleFromParent) + ")F";
		//str += "[-F";
		if (with_paras){
			string prefix1 = "<*(" + turn_scaler + ")&(" + strahler_number + ")F>";
			string prfix = "[-(" + turn_angle + ")" + prefix1;
			str += prfix;
		}
		else{
			str += "[-F";
		}

		int main_branch_idx = -1;
		for (int i = 0; i < root->children.size(); i++){
			if (root->children[i]->main_branch){
				main_branch_idx = i;
				continue;
			}
			str += Nary_write_grammar(root->children[i], with_paras);
		}
		if (main_branch_idx != -1){
			str += Nary_write_grammar(root->children[main_branch_idx], with_paras);
		}
		str += "]";
		//std::cout << "[-F]";
	}
	return str;
}

string GlViewer::Nary_write_grammar_forPaper(Nary_TreeNode *root, bool with_paras) {
	string str = "";
	if (root == NULL)
		return str;

	std::ostringstream streamObj1, streamObj2, streamObj3;

	streamObj3 << std::fixed; // Set Fixed -Point Notation
	streamObj3 << std::setprecision(1); // Set precision to 2 digits
	streamObj3 << abs(root->bbx.angleFromParent); //Add double to stream
	string turn_angle = streamObj3.str(); // Get string from output string stream

	streamObj1 << std::fixed; // Set Fixed -Point Notation
	streamObj1 << std::setprecision(1); // Set precision to 2 digits
	double scaler = root->bbx.height / m_min_len;
	streamObj1 << scaler; //Add double to stream
	string turn_scaler = streamObj1.str(); // Get string from output string stream

	streamObj2 << std::fixed; // Set Fixed -Point Notation
	streamObj2 << std::setprecision(1); // Set precision to 2 digits
	streamObj2 << abs(root->strahler_number); //Add double to stream
	string strahler_number = streamObj2.str(); // Get string from output string stream

	if (abs(root->bbx.angleFromParent) < m_mianBranchAngle_thrs){
		if (with_paras){
			string prefix = "F(" + turn_scaler + ")";
			str += prefix;
		}
		else{
			str += "F";
		}

		int main_branch_idx = -1;
		for (int i = 0; i < root->children.size(); i++){
			if (root->children[i]->main_branch){
				main_branch_idx = i;
				continue;
			}
			str += Nary_write_grammar_forPaper(root->children[i], with_paras);
		}
		if (main_branch_idx != -1){
			str += Nary_write_grammar_forPaper(root->children[main_branch_idx], with_paras);
		}
	}
	else if (root->bbx.angleFromParent > 0){
		if (with_paras){
			//str += "[+(" + std::to_string(root->bbx.angleFromParent) + ")F";
			string prefix1 = "F(" + turn_scaler + ")";
			string prfix = "[+(" + turn_angle + ")" + prefix1;
			str += prfix;
		}
		else{
			str += "[+F";
		}

		int main_branch_idx = -1;
		for (int i = 0; i < root->children.size(); i++){
			if (root->children[i]->main_branch){
				main_branch_idx = i;
				continue;
			}
			str += Nary_write_grammar_forPaper(root->children[i], with_paras);
		}
		if (main_branch_idx != -1){
			str += Nary_write_grammar_forPaper(root->children[main_branch_idx], with_paras);
		}
		str += "]";
		//std::cout << "[+F]";
	}
	else if (root->bbx.angleFromParent < 0){
		//std::cout << "[-(" << -root->bbx.angleFromParent << ")F]";
		//str += "[-(" + std::to_string(root->bbx.angleFromParent) + ")F";
		//str += "[-F";
		if (with_paras){
			string prefix1 = "F(" + turn_scaler + ")";
			string prfix = "[-(" + turn_angle + ")" + prefix1;
			str += prfix;
		}
		else{
			str += "[-F";
		}

		int main_branch_idx = -1;
		for (int i = 0; i < root->children.size(); i++){
			if (root->children[i]->main_branch){
				main_branch_idx = i;
				continue;
			}
			str += Nary_write_grammar_forPaper(root->children[i], with_paras);
		}
		if (main_branch_idx != -1){
			str += Nary_write_grammar_forPaper(root->children[main_branch_idx], with_paras);
		}
		str += "]";
		//std::cout << "[-F]";
	}
	return str;
}

double GlViewer::Nary_get_scalar_para(Nary_TreeNode *root){
	double scalar;

	if (root != NULL) {
		if (root->children.size() > 0) {
			double max_scale = -1000;
			for (int i = 0; i < root->children.size(); i++){
				double s = root->bbx.height / root->children[i]->bbx.height;
				max_scale = std::max(max_scale, s);
				if (!root->children[i]->main_branch){
					m_average_branch_angle_ += std::abs(root->children[i]->bbx.angleFromParent);
					m_tree_node_size_angle_++;
				}
			}
			scalar = std::max(max_scale, 1.0);
			m_tree_node_size_++;
		}
		else{
			return 0.0;
		}
	}
	else{
		return 0.0;
	}

	for (int i = 0; i < root->children.size(); i++){
		scalar += Nary_get_scalar_para(root->children[i]);
	}
	return scalar;
}

void GlViewer::Nary_generate_conformal_grammar(Nary_TreeNode *root, unordered_map<string, Nary_repetition_node>& m){

	if (root == NULL){
		return;
	}
	int max_iter = 5;
	int type_id = 0;
	double weight = 0.5;
	int real_iters = 0;
	for (int k = 0; k < max_iter; k++){
		//find all repetitions in current tree/sub-tree
		bool new_repetition = false;
		m.clear();
		Nary_find_repetitions(root, m);
		std::unordered_map<string, Nary_repetition_node>::iterator iter;
		for (iter = m.begin(); iter != m.end(); iter++){
			if (iter->second.oocur_time > 1 && iter->second.oocur_time > iter->second.last_groups_numer){
				new_repetition = true;
			}
		}
		//stop if cannot cluster anymore
		bool find_again = false;
		string str = Nary_select_prefer_repetition(m, weight, find_again);
		if (str == ""){
			//std::cout << "No selected repetition!" << std::endl;
			if (root->parent != NULL && !root->old_repetition){
				ruleSuccessor rule = Nary_write_rules(root, true);
				m_rules[alphabet[m_alphabet_pointer]].push_back(rule);
				//std::cout << alphabet[m_alphabet_pointer] << " -> " << rule.successor << endl;
				//m_alphabet_pointer++;
			}
			break;
		}
		m_selected_repetitions.insert(str);

		//update the cluster information according to current repetition
		Nary_repetition_node r_node = m[str];
		int last_groups_numer = r_node.last_groups_numer;
		if (!find_again){
			m_used_symbols[str] = alphabet[m_alphabet_pointer];
		}

		//int cluster_id = 0;
		for (int i = last_groups_numer; i < r_node.parent_node.size(); i++){
			r_node.parent_node[i]->old_repetition = find_again;
			m_selected_repetitions_nary_nodes.insert(r_node.parent_node[i]);
			if (i == 0){// we only genrate the grammar once for current repetition
				Nary_generate_conformal_grammar(r_node.parent_node[i], m);
				//if (find_again) m_alphabet_pointer--;
			}
			if (find_again){
				bool update = Nary_update_cluster_infomation(r_node.parent_node[i], i, m_used_symbols[str]); //update the node information;
			}
			else{
				bool update = Nary_update_cluster_infomation(r_node.parent_node[i], i, alphabet[m_alphabet_pointer]); //update the node information;
			}

			Nary_perform_clustring(r_node.parent_node[i], r_node.parent_node[i]->old_repetition); // perform real clustring on the sub-tree, i.e., collpase each repetition to one node 
		}
		if (!find_again) m_alphabet_pointer++;

		real_iters++;

		//std::cout << "***********After iteration " << k + 1 << "**************" << std::endl;
		//Nary_print_tree(root, 0);
	}

	//cout << "Our Greedy Iterations: " << real_iters << endl;
}

string GlViewer::Nary_find_repetitions(Nary_TreeNode* node, unordered_map<string, Nary_repetition_node>& m)
{
	if (!node)
		return "";

	string str = "(";
	//str += node->turn_indicator;
	int main_branch_idx = -1;
	for (int i = 0; i < node->children.size(); i++){
		if (node->children[i]->main_branch){
			main_branch_idx = i;
			continue;
		}
		if (node->cluster_level == node->children[i]->cluster_level){
			str += node->children[i]->turn_indicator;
			str += Nary_find_repetitions(node->children[i], m);
		}
		
	}
	if (main_branch_idx != -1 && node->cluster_level == node->children[main_branch_idx]->cluster_level){
		str += node->children[main_branch_idx]->turn_indicator;
		str += Nary_find_repetitions(node->children[main_branch_idx], m);
	}
	//str += to_string(node->data);
	str += ")";

	// Subtree already present (Note that we use 
	// unordered_map instead of unordered_set 
	// because we want to print multiple duplicates 
	// only once, consider example of 4 in above 
	// subtree, it should be printed only once. 
	//if (m[str].oocur_time >= 1 && str.length()>3)
	//	cout << node->bbx_index << " ";

	std::set<Nary_TreeNode *>::iterator siter;
	siter = m_selected_repetitions_nary_nodes.find(node);
	if (siter == m_selected_repetitions_nary_nodes.end()){
		m[str].oocur_time++;
		m[str].parent_node.push_back(node);
	}

	/*std::pair<std::unordered_set<TreeNode *>::iterator, bool> ret;
	ret = m[str].parent_node.insert(node);
	if (ret.second){
	m[str].oocur_time++;
	}*/

	return str;
}

string GlViewer::Nary_select_prefer_repetition(unordered_map<string, Nary_repetition_node>& m, double weight, bool& find){
	string select_str = "";
	std::unordered_map<string, Nary_repetition_node>::iterator iter;
	//now we only get the largest sub-tree, but we prefer the one has selected before
	int max_node_size = -1;

	std::vector<string> selected_strs;
	std::vector<int> node_sizes;
	int max_node_size_in_selected = -1;
	int max_node_pos_in_selected = 0;

	std::pair<std::set<string>::iterator, bool> ret;

	for (iter = m.begin(); iter != m.end(); iter++)
	{
		// get the node numbers of this sub-tree
		string str = iter->first;
		int num = 0;
		for (int j = 0; j < str.length(); j++){
			if (str.at(j) == '(' || str.at(j) == ')' || str.at(j) == '-'){
				num++;
			}
		}
		int group_node_size = str.length() - num + 1;
		iter->second.group_node_size = group_node_size;

		std::set<string>::iterator siter;
		siter = m_selected_repetitions.find(str);
		//ret = m_selected_repetitions.insert(str);

		/*
		2020.02.08: Jianwei changed the 'group_node_size >= 3' to 'group_node_size >= 2'
		*/
		if ((iter->second.oocur_time > 1 && group_node_size >= 3) || siter != m_selected_repetitions.end()){
			//if (iter->second.oocur_time > 1 && group_node_size >= 3){
			if (group_node_size > max_node_size){
				max_node_size = group_node_size;
				select_str = iter->first;
			}
			if (siter != m_selected_repetitions.end()){
				selected_strs.push_back(iter->first);
				node_sizes.push_back(group_node_size);
				if (group_node_size > max_node_size_in_selected){
					max_node_size_in_selected = group_node_size;
					max_node_pos_in_selected = selected_strs.size() - 1;
				}
			}
		}
	}

	if (selected_strs.empty()){
		find = false;
		return select_str;
	}
	else {
		if (max_node_size_in_selected == max_node_size){
			find = true;
			return select_str;
		}
		else{
			find = true;
			return selected_strs[max_node_pos_in_selected];
		}
	}

	return select_str;
}

ruleSuccessor GlViewer::Nary_write_rules(Nary_TreeNode *root, bool new_tree) {
	ruleSuccessor rule;
	string str = "";
	if (root == NULL)
		return rule;

	if (abs(root->bbx.angleFromParent) < m_mianBranchAngle_thrs){
		//str += "F";
		str += root->alphabet_symbol;
		int main_branch_idx = -1;
		for (int i = 0; i < root->children.size(); i++){
			if (root->children[i]->main_branch){
				main_branch_idx = i;
				continue;
			}
			str += Nary_write_rules(root->children[i], false).successor;
		}
		if (main_branch_idx != -1){
			str += Nary_write_rules(root->children[main_branch_idx], false).successor;
		}
	}
	else if (root->bbx.angleFromParent > 0){
		//str += "[+(" + std::to_string(root->bbx.angleFromParent) + ")F";
		//str += "[+F";
		if (new_tree){
			str += root->alphabet_symbol;
		}
		else{
			str += "[+" + root->alphabet_symbol;
		}
		int main_branch_idx = -1;
		for (int i = 0; i < root->children.size(); i++){
			if (root->children[i]->main_branch){
				main_branch_idx = i;
				continue;
			}
			str += Nary_write_rules(root->children[i], false).successor;
		}
		if (main_branch_idx != -1){
			str += Nary_write_rules(root->children[main_branch_idx], false).successor;
		}
		if (!new_tree){
			str += "]";
		}
		//std::cout << "[+F]";
	}
	else if (root->bbx.angleFromParent < 0){
		//std::cout << "[-(" << -root->bbx.angleFromParent << ")F]";
		//str += "[-(" + std::to_string(root->bbx.angleFromParent) + ")F";
		//str += "[-F";
		if (new_tree){
			str += root->alphabet_symbol;
		}
		else{
			str += "[-" + root->alphabet_symbol;
		}
		int main_branch_idx = -1;
		for (int i = 0; i < root->children.size(); i++){
			if (root->children[i]->main_branch){
				main_branch_idx = i;
				continue;
			}
			str += Nary_write_rules(root->children[i], false).successor;
		}
		if (main_branch_idx != -1){
			str += Nary_write_rules(root->children[main_branch_idx], false).successor;
		}
		if (!new_tree){
			str += "]";
		}
		//std::cout << "[-F]";
	}
	rule.successor = str;
	return rule;
}

bool GlViewer::Nary_update_cluster_infomation(Nary_TreeNode *node, int cluster_id, string symb){
	if (node == NULL){
		return true;
	}
	bool update = true;
	node->alphabet_symbol = symb;
	node->cluster_id = cluster_id;
	node->cluster_level += 1;

	int main_branch_idx = -1;
	for (int i = 0; i < node->children.size(); i++){
		if (node->children[i]->main_branch){
			main_branch_idx = i;
			continue;
		}
		bool updateChild = Nary_update_cluster_infomation(node->children[i], cluster_id, symb);
	}
	if (main_branch_idx != -1){
		bool updateChild = Nary_update_cluster_infomation(node->children[main_branch_idx], cluster_id, symb);
	}

	return update;
}

void GlViewer::Nary_perform_clustring(Nary_TreeNode *node, bool find){
	bool collaps_happen = true, move_happen = true;
	int max_iter = 10;
	for (int i = 0; i < max_iter; i++){
		collaps_happen = Nary_perform_collaps_leaf(node, find);
		//print_tree(node, 0);
		move_happen = Nary_perform_move_leaf(node, find);
		if (!collaps_happen && !move_happen) break;
		//if (!collaps_happen) break;
	}
}

bool GlViewer::Nary_perform_collaps_leaf(Nary_TreeNode *node, bool find){
	bool collaps_happen = false, collaps3 = false;
	if (node == NULL)
		return false;

	int main_branch_idx = -1;
	for (int i = 0; i < node->children.size(); i++){
		if (node->children[i]->main_branch){
			main_branch_idx = i;
			continue;
		}
		bool collaps_1 = Nary_perform_collaps_leaf(node->children[i], find);
		collaps_happen = (collaps_happen || collaps_1);
	}
	if (main_branch_idx != -1){
		bool collaps_2 = Nary_perform_collaps_leaf(node->children[main_branch_idx], find);
		collaps_happen = (collaps_happen || collaps_2);
	}
	if (node->children.size() == 0){
		//if (node->cluster_level == 1 && node->type_id == node->parent->type_id && node->cluster_id == node->parent->cluster_id){
		if (!find && node->alphabet_symbol == node->parent->alphabet_symbol && node->cluster_id == node->parent->cluster_id){
			node->parent->children.erase(std::find(node->parent->children.begin(), node->parent->children.end(), node));
			DestroyTree(node);
			collaps3 = true;
		}
		if (find && node->alphabet_symbol == node->parent->alphabet_symbol && node->cluster_level == node->parent->cluster_level){
			node->parent->children.erase(std::find(node->parent->children.begin(), node->parent->children.end(), node));
			DestroyTree(node);
			collaps3 = true;
		}
	}
	
	return (collaps_happen || collaps3);
}

bool GlViewer::Nary_perform_move_leaf(Nary_TreeNode *node, bool find){
	bool move_happen = false, move3 = false;
	if (node == NULL)
		return false;

	int main_branch_idx = -1;
	for (int i = 0; i < node->children.size(); i++){
		if (node->children[i]->main_branch){
			main_branch_idx = i;
			continue;
		}
		bool move_1 = Nary_perform_move_leaf(node->children[i], find);
		move_happen = (move_happen || move_1);
	}
	if (main_branch_idx != -1){
		bool move_2 = Nary_perform_move_leaf(node->children[main_branch_idx], find);
		move_happen = (move_happen || move_2);
	}

	if (node->parent != NULL && node->cluster_level >= 1){
		TreeNode *left = NULL, *right = NULL;

		std::vector<Nary_TreeNode*> children_tmp;
		int main_branch_idx = -1;
		for (int i = 0; i < node->children.size(); i++){
			if (node->children[i]->cluster_level >= 1){
				if (find && node->cluster_level != node->children[i]->cluster_level){
					children_tmp.push_back(node->children[i]);
				}
			}
		}
		if (children_tmp.size() == 0) return false;
		if (node->parent != NULL && node->cluster_level == node->parent->cluster_level){
			for (int j = 0; j < children_tmp.size(); j++){
				node->parent->children.push_back(children_tmp[j]);
				children_tmp[j]->parent = node->parent;
			}
			node->parent->children.erase(std::find(node->parent->children.begin(), node->parent->children.end(), node));
			//DestroyTree(node);
			move3 = true;
		}
	}

	return (move_happen || move3);
}

void GlViewer::Nary_compute_strahler_number(Nary_TreeNode *root) {
	std::vector<std::vector<Nary_TreeNode*> > matrix;
	if (root == NULL)
	{
		//return matrix;
		return;
	}

	stack<vector<Nary_TreeNode*> > sv;
	vector<Nary_TreeNode*> temp;
	temp.push_back(root);
	sv.push(temp);

	vector<Nary_TreeNode *> path;
	path.push_back(root);

	int count = 1;
	while (!path.empty())
	{
		for (int i = 0; i < path[0]->children.size(); i++) {
			if (path[0]->children[i] != NULL ) {
				path.push_back(path[0]->children[i]);
			}
		}
		path.erase(path.begin());
		count--;
		if (count == 0)
		{
			/* vector<TreeNode*> tmp;
			vector<TreeNode *>::iterator it = path.begin();
			for(; it != path.end(); ++it)
			{
			tmp.push_back((*it));
			}*/
			sv.push(path);
			count = path.size();
		}
	}
	
	while (!sv.empty())
	{
		if (sv.top().size() > 0)
		{
			matrix.push_back(sv.top());
		}
		sv.pop();
	}
	
	for (int i = 0; i < matrix.size(); i++) {
		for (int j = 0; j < matrix[i].size(); j++) {
			Nary_TreeNode *cur_node = matrix[i][j];
			if (cur_node->children.size() == 0) {
				cur_node->strahler_number = 1;
				continue;
			}
			int max_strahler_number = cur_node->children[0]->strahler_number;
			bool same_order = true;
			for (int k = 1; k < cur_node->children.size(); k++) {
				if (cur_node->children[k]->strahler_number != max_strahler_number) {
					max_strahler_number = std::max(cur_node->children[k]->strahler_number, max_strahler_number);
					same_order = false;
				}
			}
			if ((same_order && cur_node->children.size() > 1) || cur_node->children.size() > 2) {
				cur_node->strahler_number = max_strahler_number + 1;
			}
			else {
				cur_node->strahler_number = max_strahler_number;
			}
		}
	}
}

struct pair_rule{
	int rule1_id;
	int rule2_id;
	double merge_loss;
	pair_rule() :rule1_id(-1), rule2_id(-1), merge_loss(0){
	}
};

void GlViewer::grammar_induction(){
	// collect information about the grammar 
	std::vector<ruleProductions> productions;
	newAssociativeArray::const_iterator iter;
	std::unordered_map<string, int> symbols_infor;
	for (iter = m_rules.begin(); iter != m_rules.end(); ++iter)
	{
		string left = iter->first;
		symbols_infor[left] = 0;
		vector<ruleSuccessor> sucs = iter->second;
		for (int i = 0; i < sucs.size(); i++){
			string right = sucs[i].successor;
			//bool has_non_terminals = rule_has_non_terminals(right);
			/*bool has_non_terminals = false;
			for (int i = 0; i < right.length(); i++) {
			if (!(right.at(i) == '(' || right.at(i) == ')' || right.at(i) == '+' || right.at(i) == '-' || right.at(i) == '[' || right.at(i) == ']' || right.at(i) == 'F')) {
			has_non_terminals = true;
			break;
			}
			}*/
			//if(has_non_terminals) {
			ruleProductions rule(left, right);
			productions.push_back(rule);
			symbols_infor[left] += 1;
			//}
		}
	}

	// we merge similar rules in a greedy way
	int max_iter = 10;
	double edit_dis_threshold = 6;
	for (int i = 0; i < max_iter; i++){
		bool merged = false;
		//compute current grammar length
		int grammar_length = compute_grammar_length(productions, symbols_infor);

		//find and merge two rules that can be merged
		//std::vector<pair_rule> candidate_pairs;
		//std::vector<int> min_loss_ids;
		double min_loss = 100000;
		std::vector<ruleProductions> productions_new;
		std::vector<ruleProductions> productions_final;
		//bool update = false;
		for (int j = 0; j < productions.size(); j++)
		{
			//cout << iter->second[0].successor << endl;
			string left1 = productions[j].precessor;
			string right1 = productions[j].successor;
			//if (left1 == "S") {
			for (int k = 0; k < productions.size(); k++) {
				bool update = false;
				if (j == k) continue;
				//cout << iter2->second[0].successor << endl;
				string left2 = productions[k].precessor;
				string right2 = productions[k].successor;

				if (left1 == left2) continue;

				double min_edit_distance;
				if (left2 == "S") {
					min_edit_distance = edit_distance_DP(right1, right2);
				}
				else{
					min_edit_distance = edit_distance_DP(right2, right1);
				}

				if (productions.size() == 2) {
					min_edit_distance = 0;
				}
				if (min_edit_distance > edit_dis_threshold) {
					continue;
				}
				productions_new.clear();
				productions_new.assign(productions.begin(), productions.end()); // copy all rules

				if (left2 == "S") {
					merged = merge_two_rules(productions_new, k, j, symbols_infor);
				}
				else {
					merged = merge_two_rules(productions_new, j, k, symbols_infor);
				}
				int grammar_length_new = compute_grammar_length(productions_new, symbols_infor);
				double loss = (grammar_length_new - grammar_length) + min_edit_distance;
				if (loss < min_loss) {
					min_loss = loss;
					productions_final.clear();
					productions_final.assign(productions_new.begin(), productions_new.end());
					update = true;
				}
				//swap
				if (update) {
					productions.assign(productions_final.begin(), productions_final.end());
					productions_final.clear();
				}
			}
			//}
		}

		//cout << "Merge iteration: " << i << endl;
		if (merged == false) break;
	}

	//output 
	cout << "After grammar generalization..." << endl;
	m_final_grammar_length_ = 0;
	for (int k = 0; k < productions.size(); k++){

		string right = productions[k].successor;
		bool has_non_terminals = false;
		for (int i = 0; i < right.length(); i++) {
			if (!(right.at(i) == '(' || right.at(i) == ')' || right.at(i) == '+' || right.at(i) == '-' || right.at(i) == '[' || right.at(i) == ']' || right.at(i) == 'F')) {
				has_non_terminals = true;
				break;
			}
		}
		if (!has_non_terminals) continue; // if the right side only contains terminal 'F', we discard this rule

		if (symbols_infor[productions[k].precessor] > 1) {
			string::size_type idx = productions[k].successor.find(productions[k].precessor);
			if (idx != string::npos) {
				cout << productions[k].precessor << "->" << productions[k].successor << endl;
				m_final_grammar_length_ += productions[k].precessor.length();
				m_final_grammar_length_ += productions[k].successor.length();
			}
			continue;
		}

		cout << productions[k].precessor << "->" << productions[k].successor << endl;
		m_final_grammar_length_ += productions[k].precessor.length();
		m_final_grammar_length_ += productions[k].successor.length();
	}
}

bool GlViewer::merge_two_rules(std::vector<ruleProductions> &productions, int ri, int rj, std::unordered_map<string, int> &symbols_info){
	bool merged = false;
	string left1 = productions[ri].precessor;
	string left2 = productions[rj].precessor;
	for (int k = 0; k < productions.size(); k++){
		//if (k == ri) continue;
		if (productions[k].precessor == left2){
			productions[k].precessor = left1;
			symbols_info[left2] -= 1;
			symbols_info[left1] += 1;
			merged = true;
		}
		string rhs = productions[k].successor;
		//replace left2 with left1 in the RHS string
		//rhs = regex_replace(rhs, regex(left2), left1);
		size_t pos = 0;
		while ((pos = rhs.find(left2, pos)) != std::string::npos) {
			rhs.replace(pos, left2.length(), left1);
			pos += left1.length();
		}
		productions[k].successor = rhs;
	}

	// we handle the case that the RHS of two rules are identical
	std::vector<ruleProductions> productions_copy;
	std::set<int> deleted_ids;
	for (int k = 0; k < productions.size(); k++){
		for (int m = k + 1; m < productions.size(); m++){
			if (k == m) continue;
			string left1 = productions[k].precessor;
			string right1 = productions[k].successor;

			string left2 = productions[m].precessor;
			string right2 = productions[m].successor;
			if (right1 == right2){
				bool findLeft1 = (right1.find(left1) == std::string::npos) ? false : true;
				bool findLeft2 = (right2.find(left2) == std::string::npos) ? false : true;
				if (findLeft1){
					deleted_ids.insert(m);
				}
				else if (findLeft2){
					deleted_ids.insert(k);
				}
				else{
					deleted_ids.insert(m);
				}
			}
		}
	}
	for (int k = 0; k < productions.size(); k++){
		std::set<int>::iterator iter = deleted_ids.find(k);
		if (iter == deleted_ids.end()){
			productions_copy.push_back(productions[k]);
		}
	}
	productions.assign(productions_copy.begin(), productions_copy.end());

	return merged;
}

int GlViewer::compute_grammar_length(std::vector<ruleProductions> &productions, std::unordered_map<string, int> &symbols_info){
	//compute current grammar length
	//std::set<string> used_symbols;
	int symbols_length = 0;
	int grammar_length = 0;
	for (int j = 0; j < productions.size(); j++){
		string left = productions[j].precessor;
		std::unordered_map<string, int>::iterator it = symbols_info.find(left);
		if (it->second > 0){
			symbols_length += left.length();
		}

		string right = productions[j].successor;
		grammar_length += right.length();
	}
	grammar_length += symbols_length;

	return grammar_length;
}

// A Dynamic Programming program to find minimum number operations to convert str1 to str2
double GlViewer::edit_distance_DP(string str1, string str2)
{
	// Create a table to store results of subproblems 
	int m = str1.length(), n = str2.length();
	//int dp[m + 1][n + 1];
	//int[][] dp = new int[m + 1][n + 1];
	double **dp;
	dp = new double *[m + 1];
	for (int i = 0; i <= m; i++)
	{
		dp[i] = new double[n + 1];
	}

	// Fill d[][] in bottom up manner 
	for (int i = 0; i <= m; i++)
	{
		if (i == 9) {
			int dddd = 0;
		}
		for (int j = 0; j <= n; j++)
		{
			// If first string is empty, only option is to isnert all characters of second string 
			if (i == 0){
				dp[i][j] = j;  // Min. operations = j 
			}
			// If second string is empty, only option is to remove all characters of second string 
			else if (j == 0){
				dp[i][j] = i; // Min. operations = i 
			}
			// If last characters are same, ignore last char and recur for remaining string 
			else if (str1[i - 1] == str2[j - 1]){
				dp[i][j] = dp[i - 1][j - 1];
			}
			// If the last character is different, consider all possibilities and find the minimum 
			else{
				double insert_op = dp[i][j - 1]; // Insert
				double remove_op = dp[i - 1][j];
				double replace_op = dp[i - 1][j - 1];// Replace
				dp[i][j] = 1 + std::min(insert_op, std::min(remove_op, replace_op));
			}
		}
	}

	double distance = dp[m][n];
	//release
	for (int i = 0; i<m + 1; i++)
	{
		delete[] dp[i];
	}
	delete dp;

	return distance;
}

/*
Simulated Annealing
*/

struct CmpByStringLength {
	bool operator()(const string& k1, const string& k2) {
		return k1.length() < k2.length();
	}
};

inline bool subString_is_valid(string str){
	if (str.find("F") == str.npos){
		return false;
	}
	if (str == "F"){
		return false;
	}
	std::stack<char> s;
	for (int i = 0; i < str.length(); i++){
		if (str.at(i) == '['){
			s.push('[');
		}
		else if (str.at(i) == ']'){
			if (s.empty()){
				return false;
			}
			s.pop();
		}
	}
	if (!s.empty()){
		return false;
	}
	return true;
}

int GlViewer::Nary_generate_compact_grammar_simulatedAnnealing(string left_NonT, string str){

	if (str.length() < 6){
		return 0;
	}
	std::unordered_map<string, std::vector<int>> map_repeated_subStrings = find_all_repeating_substring(str);
	if (map_repeated_subStrings.size() == 0){
		return 0;
	}

	std::unordered_map<string, std::vector<int>>::iterator iter;

	std::vector<Repeated_substring> list_repeated_substrings;// (map_repeated_subStrings.size());
	for (iter = map_repeated_subStrings.begin(); iter != map_repeated_subStrings.end(); iter++){
		if (subString_is_valid(iter->first)){
			list_repeated_substrings.push_back(Repeated_substring(iter->first, iter->second));
		}
		//list_repeated_substrings[count].index = iter->second;
	}

	if (list_repeated_substrings.size() == 0){
		return 0;
	}

	std::sort(list_repeated_substrings.begin(), list_repeated_substrings.end());
	int list_size = list_repeated_substrings.size();

	// run 
	double weight_len = 0.5;
	int selected_str_idx = 0;
	double current_energy = weight_len *(1 + str.length()) + (1.0 - weight_len) * str.length(); // 

	std::srand(unsigned(std::time(0)));

	//Monte_Carlo to get a good intial solution

	newAssociativeArray infer_rules;
	for (int t = 0; t<20; t++){
		newAssociativeArray cur_rules;
		int new_subStr = rand() % list_size;
		//cout << list_repeated_substrings[new_subStr].value << endl;
		generate_new_rule_SA(left_NonT, str, list_repeated_substrings[new_subStr], cur_rules);
		double new_energy = compute_compact_energy(cur_rules, weight_len);
		//cout << "E: " << new_energy  << endl;
		//cout << endl;
		if (new_energy < current_energy){
			selected_str_idx = new_subStr;
			current_energy = new_energy;
			infer_rules = cur_rules;
		}
	}
	//cout << "Best: " << list_repeated_substrings[selected_str_idx].value << endl;
	//cout << endl;

	if (infer_rules.size() == 0){
		return 0;
	}
	//cout << "list size: " << list_size << endl;

	double e = 1e-16, at = 0.99999999, T = 1.0;
	int L_max = 3000;
	int L = L_max; //max iteration number
	while (L--){ 
		if (list_size == 1){
			break;
		}
		int window_size = 5;
		if (list_size < 5){
			window_size = 2;
		}
		int c1 = (rand() % (window_size * 2)) - window_size;
		if (c1 == 0 || (selected_str_idx + c1)<0 || (selected_str_idx + c1) >= list_size){
			L++; 
			continue;
		}
		newAssociativeArray cur_rules;
		generate_new_rule_SA(left_NonT, str, list_repeated_substrings[selected_str_idx + c1], cur_rules);
		double new_energy = compute_compact_energy(cur_rules, weight_len);
		//cout << "E: " << new_energy << endl;
		//cout << endl;
		double dE = new_energy - current_energy;  
		double sj = rand() % 10000;     
		sj /= 10000;
		if (dE < 0){  
			selected_str_idx = selected_str_idx + c1;
			current_energy = new_energy;
			infer_rules = cur_rules;
		}
		else if (exp(-dE / T)>sj){  
			selected_str_idx = selected_str_idx + c1;
			current_energy = new_energy;
			infer_rules = cur_rules;
		}
		//cout << list_repeated_substrings[selected_str_idx].value << endl;
		T *= at;  
		if (T<e) break;  	
	}

	//cout << "Final Best: " << list_repeated_substrings[selected_str_idx].value << endl;
	//cout << endl;

	cout << "SA Iterations: " << L_max - L << endl;
	cout << endl;

	print_rules_SA(infer_rules);

	m_alphabet_pointer++;
	newAssociativeArray::const_iterator iter2;
	for (iter2 = infer_rules.begin(); iter2 != infer_rules.end(); ++iter2)
	{
		string left = iter2->first;
		vector<ruleSuccessor> sucs = iter2->second;
		for (int i = 0; i < sucs.size(); i++){
			string right = sucs[i].successor;
			int derived = Nary_generate_compact_grammar_simulatedAnnealing(left, right);
			if (derived == 0){
				m_rules[left].push_back(right);
			}
		}
	}

	return 1;
}

void GlViewer::print_rules_SA(newAssociativeArray& rules){
	newAssociativeArray::const_iterator iter;
	for (iter = rules.begin(); iter != rules.end(); ++iter)
	{
		string left = iter->first;
		vector<ruleSuccessor> sucs = iter->second;
		for (int i = 0; i < sucs.size(); i++){
			string right = sucs[i].successor;
			cout << left << " -> " << right << endl;
		}
	}
}

void GlViewer::generate_new_rule_SA(string left_NonT, string str, Repeated_substring select_substring, newAssociativeArray& rules) {

	int count = 0;
	for (int i = 0; i < select_substring.index.size(); i++){
		int idx = select_substring.index[i] - (select_substring.value.length() - 1)*count;
		if (str.substr(idx, select_substring.value.length()) == select_substring.value){
			str = str.replace(str.begin() + idx, str.begin() + idx + select_substring.value.length(), alphabet[m_alphabet_pointer+1]);
			count++;
		}
	}

	ruleSuccessor rule(str);
	rules[left_NonT].clear();
	rules[left_NonT].push_back(rule);
	//cout << alphabet[m_alphabet_pointer] << " -> " << rule.successor << endl;

	ruleSuccessor rule2(select_substring.value);
	rules[alphabet[m_alphabet_pointer + 1]].push_back(rule2);
	//cout << alphabet[m_alphabet_pointer + 1] << " -> " << rule2.successor << endl;

}

struct LHS_infor_SA
{
	int frequency;
	int length;
	LHS_infor_SA(int f, int l) : frequency(f), length(l){
	}
	LHS_infor_SA() : frequency(0), length(0){
	}
};

double GlViewer::compute_compact_energy(newAssociativeArray rules, double weight){
	std::unordered_map<string, LHS_infor_SA> LHS_map;

	newAssociativeArray::const_iterator iter;
	for (iter = rules.begin(); iter != rules.end(); ++iter)
	{
		string left = iter->first;
		vector<ruleSuccessor> sucs = iter->second;
		int cur_rule_len = 0;
		for (int i = 0; i < sucs.size(); i++){
			string right = sucs[i].successor;
			for (int j = 0; j < right.length(); j++){
				if (right.at(j) != 'F' && right.at(j) != '+' && right.at(j) != '-'){
					LHS_map[right.substr(j,1)].frequency += 1;
				}
			}
			cur_rule_len += left.length();
			cur_rule_len += right.length();
		}
		LHS_map[left].length += cur_rule_len;
	}

	std::unordered_map<string, LHS_infor_SA>::const_iterator iter2;
	double E=0.0;
	for (iter2 = LHS_map.begin(); iter2 != LHS_map.end(); ++iter2)
	{
		E += weight *iter2->second.length + (1.0 - weight) * std::max(iter2->second.frequency,1);
	}

	return E;
}

std::unordered_map<string, std::vector<int>> GlViewer::find_all_repeating_substring(string str){

	int MINLEN=1, MINCNT=2;
	int str_len = str.length();
	std::unordered_map<string, std::vector<int>> repeated_patterns;
	
	SuffixTree tree(str);
	for (int sublen = MINLEN; sublen <= int(str_len / MINCNT); sublen++){
		for (int i = 0; i <= str_len - sublen; i++){
			string sub_str = str.substr(i, sublen);
			std::vector<int> search_result = tree.getIndexes(sub_str);
			if (search_result.size() >= MINCNT && repeated_patterns.find(sub_str) == repeated_patterns.end()){
				repeated_patterns[sub_str] = search_result;
			}
		}
	}

	return repeated_patterns;
}

/*
Genetic Algorithm
*/
#define CROSSOVER_RATE            0.7
#define MUTATION_RATE             0.05 //0.001
#define POP_SIZE                  10           //must be an even number
#define MAX_ALLOWABLE_GENERATIONS   2000
#define RANDOM_NUM      ((float)rand()/(RAND_MAX+1)) //returns a float between 0 & 1

int GlViewer::Nary_generate_compact_grammar_geneticAlgorithms(string left_NonT, string str){

	if (str.length() < 6){
		return 0;
	}
	// find all of the repeated syrstrings from current string
	std::unordered_map<string, std::vector<int>> map_repeated_subStrings = find_all_repeating_substring(str);
	if (map_repeated_subStrings.size() == 0){
		return 0;
	}
	std::unordered_map<string, std::vector<int>>::iterator iter;
	std::vector<Repeated_substring> list_repeated_substrings;// (map_repeated_subStrings.size());
	for (iter = map_repeated_subStrings.begin(); iter != map_repeated_subStrings.end(); iter++){
		if (subString_is_valid(iter->first)){
			list_repeated_substrings.push_back(Repeated_substring(iter->first, iter->second));
		}
		//list_repeated_substrings[count].index = iter->second;
	}
	if (list_repeated_substrings.size() == 0){
		return 0;
	}
	std::sort(list_repeated_substrings.begin(), list_repeated_substrings.end()); // sort the repeated substring according to their length
	int list_size = list_repeated_substrings.size();

	/*for (int i = 0; i < list_repeated_substrings.size(); i++){
		std::cout << list_repeated_substrings[i].value << " ";
		if (list_repeated_substrings[i].value == "FA" || list_repeated_substrings[i].value == "FB" || list_repeated_substrings[i].value == "FC"){
			int ddd = 0;
		}
		for (int j = 0; j<list_repeated_substrings[i].index.size(); ++j)
			std::cout << list_repeated_substrings[i].index[j] << " ";
		std::cout << std::endl;
	}*/

	// run 
	double weight_len = 1.0;
	int best_str_idx = 0;
	double inital_energy = weight_len *(1 + str.length()) + (1.0 - weight_len) * str.length()*0.5; // 
	newAssociativeArray best_infer_rules;

	double best_energy = inital_energy;

	std::srand(unsigned(std::time(0)));

	int CHROMO_LENGTH = list_size-1;
	int real_iters = 0;
	for (int it = 0; it < MAX_ALLOWABLE_GENERATIONS;  it++) //enter the main GA loop
	{
		real_iters++;
		//storage for our population of chromosomes.
		chromo_typ Population[POP_SIZE];

		//first create a random population, all with zero fitness.
		for (int i = 0; i<POP_SIZE; i++)
		{
			Population[i].bits = GetRandomBits(CHROMO_LENGTH);
			Population[i].fitness = 0.0f;
		}
		//this is used during roulette wheel sampling
		float TotalFitness = 0.0f;

		// test and update the fitness of every chromosome in the 
		// population
		//double best_energy = inital_energy;
		for (int i = 0; i<POP_SIZE; i++)
		{
			newAssociativeArray cur_rules;
			int cur_subStr = BinToDec(Population[i].bits); //  rand() % list_size;
			//cout << list_repeated_substrings[new_subStr].value << endl;
			generate_new_rule_SA(left_NonT, str, list_repeated_substrings[cur_subStr], cur_rules);
			Population[i].fitness = compute_compact_energy(cur_rules, weight_len);
			TotalFitness += Population[i].fitness;

			if (Population[i].fitness < best_energy){
				best_str_idx = cur_subStr;
				best_energy = Population[i].fitness;
				best_infer_rules = cur_rules;
			}
		}

		//std::cout << "Best: " << list_repeated_substrings[best_str_idx].value << endl;

		// create a new population by selecting two parents at a time and creating offspring
		// by applying crossover and mutation. Do this until the desired number of offspring
		// have been created. 

		//define some temporary storage for the new population we are about to create
		chromo_typ temp[POP_SIZE];

		int cPop = 0;

		//loop until we have created POP_SIZE new chromosomes
		while (cPop < POP_SIZE)
		{
			// we are going to create the new population by grabbing members of the old population
			// two at a time via roulette wheel selection.
			string offspring1 = Roulette(TotalFitness, Population);
			string offspring2 = Roulette(TotalFitness, Population);

			//add crossover dependent on the crossover rate
			Crossover(offspring1, offspring2, CHROMO_LENGTH);

			//now mutate dependent on the mutation rate
			Mutate(offspring1);
			Mutate(offspring2);

			//add these offspring to the new population. (assigning zero as their
			//fitness scores)
			temp[cPop++] = chromo_typ(offspring1, 0.0f);
			temp[cPop++] = chromo_typ(offspring2, 0.0f);

		}//end loop

		//copy temp population into main population array
		for (int i = 0; i<POP_SIZE; i++)
		{
			Population[i] = temp[i];
		}
	}//end for

	if (best_infer_rules.size() == 0){
		return 0;
	}

	cout << "Final Best: " << list_repeated_substrings[best_str_idx].value << endl;
	cout << endl;

	cout << "GA Iterations: " << real_iters << endl;
	cout << endl;

	print_rules_SA(best_infer_rules);

	m_alphabet_pointer++;
	newAssociativeArray::const_iterator iter2;
	for (iter2 = best_infer_rules.begin(); iter2 != best_infer_rules.end(); ++iter2)
	{
		string left = iter2->first;
		vector<ruleSuccessor> sucs = iter2->second;
		for (int i = 0; i < sucs.size(); i++){
			string right = sucs[i].successor;
			int derived = Nary_generate_compact_grammar_geneticAlgorithms(left, right);
			if (derived == 0){
				m_rules[left].push_back(right);
			}
		}
	}

	return 1;
}

string  GlViewer::GetRandomBits(int length) //  This function returns a string of random 1s and 0s of the desired length.
{
	string bits;

	for (int i = 0; i<length; i++)
	{
		if (RANDOM_NUM > 0.5f)

			bits += "1";

		else

			bits += "0";
	}

	return bits;
}

//---------------------------------BinToDec-----------------------------------------
//
//  converts a binary string into a decimal integer
//
//-----------------------------------------------------------------------------------
int GlViewer::BinToDec(string bits)
{
	int val = 0;
	//int value_to_add = 1;
	for (int i = bits.length(); i > 0; i--)
	{
		if (bits.at(i - 1) == '1'){
			val += 1;
		}
		//value_to_add *= 2;
	}//next bit

	return val;
}

//------------------------------------Mutate---------------------------------------
//
//  Mutates a chromosome's bits dependent on the MUTATION_RATE
//-------------------------------------------------------------------------------------
void GlViewer::Mutate(string &bits)
{
	for (int i = 0; i<bits.length(); i++)
	{
		if (RANDOM_NUM < MUTATION_RATE)
		{
			if (bits.at(i) == '1')

				bits.at(i) = '0';

			else

				bits.at(i) = '1';
		}
	}

	return;
}

//---------------------------------- Crossover ---------------------------------------
//
//  Dependent on the CROSSOVER_RATE this function selects a random point along the 
//  lenghth of the chromosomes and swaps all the  bits after that point.
//------------------------------------------------------------------------------------
void GlViewer::Crossover(string &offspring1, string &offspring2, int chromo_length)
{
	//dependent on the crossover rate
	if (RANDOM_NUM < CROSSOVER_RATE)
	{
		//create a random crossover point
		int crossover = (int)(RANDOM_NUM * chromo_length);

		string t1 = offspring1.substr(0, crossover) + offspring2.substr(crossover, chromo_length);
		string t2 = offspring2.substr(0, crossover) + offspring1.substr(crossover, chromo_length);

		offspring1 = t1; offspring2 = t2;
	}
}

//--------------------------------Roulette-------------------------------------------
//
//  selects a chromosome from the population via roulette wheel selection
//------------------------------------------------------------------------------------
string GlViewer::Roulette(int total_fitness, chromo_typ* Population)
{
	//generate a random number between 0 & total fitness count
	float Slice = (float)(RANDOM_NUM * total_fitness);

	//go through the chromosones adding up the fitness so far
	float FitnessSoFar = 0.0f;

	for (int i = 0; i<POP_SIZE; i++)
	{
		FitnessSoFar += Population[i].fitness;

		//if the fitness so far > random number return the chromo at this point
		if (FitnessSoFar >= Slice)

			return Population[i].bits;
	}

	return "";
}

void GlViewer::wheelEvent(QWheelEvent *event) 
{
    if (!m_scene) return;
	//m_scale = 0.2;
    m_scale += 0.08 * (event->delta() / 120);
    if (m_scale <= 0.0) m_scale = 0.0;

	//update_resample();
    update();
	repaint();
}

void GlViewer::mousePressEvent(QMouseEvent *event) 
{
    if (!m_scene) return;
    m_mouse_click = event->pos();
    
    if (event->button() == Qt::RightButton)
    {
        setCursor(QCursor(Qt::ClosedHandCursor));
    }
	if (event->button() == Qt::MiddleButton)
    {
        setCursor(QCursor(Qt::ClosedHandCursor));
    }
}

void GlViewer::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_scene) return;
    m_mouse_move = event->pos();
    
    if (event->buttons() == Qt::RightButton)
    {
		m_scale += 1.1 * double(m_mouse_move.y() - m_mouse_click.y()) / double(height());;
        if (m_scale <= 0.0) m_scale = 0.0;
		//update_resample();
    }
	if(event->buttons() == Qt::MiddleButton){
		 move_camera(m_mouse_click, m_mouse_move);
	}
    
    m_mouse_click = m_mouse_move;
    //update();
	repaint();
}

void GlViewer::mouseReleaseEvent(QMouseEvent *event) 
{
    if (!m_scene) return;
    m_mouse_move = event->pos();
    
    if (event->button() == Qt::RightButton)
    {
        move_camera(m_mouse_click, m_mouse_move);
    }
    
    m_mouse_click = m_mouse_move;
    setCursor(QCursor(Qt::ArrowCursor));
    //updateGL();
	//update();
	repaint();
}

void GlViewer::move_camera(const QPoint& p0, const QPoint& p1)
{
    m_center_x -= double(p1.x() - p0.x()) / double(width());
    m_center_y += double(p1.y() - p0.y()) / double(height());

    //update();
	repaint();
}
