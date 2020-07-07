// STL
#include <fstream>
#include <sstream>
#include <cassert>
// Qt
#include <QtGui>
#include <QDialog>
#include <QActionGroup>
#include <QFileDialog>
#include <QInputDialog>
#include <QClipboard>
// local
#include "window.h"

MainWindow::MainWindow() :
QMainWindow(), Ui_MainWindow()
{
    setupUi(this);
    viewer->set_scene(&m_scene);

	init_slots();
}

MainWindow::~MainWindow()
{
}

MainWindow* MainWindow::getInstance()
{
	static MainWindow fancyWindow;
	return &fancyWindow;
}

void MainWindow::init_slots(){

	// parameters
	connect(spinBox_Iter, SIGNAL(valueChanged(int)), this, SLOT(updateIteration_triggered(int)));
	connect(doubleSpinBox_branchAngle, SIGNAL(valueChanged(double)), this, SLOT(updateBranchAngle_triggered(double)));
	connect(doubleSpinBox_anglePerturbation, SIGNAL(valueChanged(double)), this, SLOT(updateAnglePerturbation_triggered(double)));

	//operations
	connect(pushButton_loadLSystem, SIGNAL(pressed()), this, SLOT(loadLSystem_triggered()));
	connect(pushButton_render, SIGNAL(pressed()), this, SLOT(actionRedner_triggered()));
	connect(pushButton_drawLsystem, SIGNAL(pressed()), this, SLOT(drawLSystem_triggered()));
	connect(pushButton_loadParsing, SIGNAL(pressed()), this, SLOT(loadParsingImg_triggered()));
	connect(pushButton_parseGrammar, SIGNAL(pressed()), this, SLOT(parseGrammarImg_triggered()));
	connect(pushButton_buildGraph, SIGNAL(pressed()), this, SLOT(buildGraph_triggered()));
	connect(pushButton_extractSpanningTree, SIGNAL(pressed()), this, SLOT(extractSpanningTree_triggered()));

	// view
	connect(actionOptionDialog, SIGNAL(triggered()), this, SLOT(showDialog_triggered()));
	connect(doubleSpinBox_LineWidth, SIGNAL(valueChanged(double)), this, SLOT(updateLineWidth_triggered(double)));
	connect(spinBox_bbxID, SIGNAL(valueChanged(int)), this, SLOT(updatebbxID_triggered(int)));
	connect(checkBox_showTreeRender, SIGNAL(stateChanged(int)), this, SLOT(showTreeRender_triggered()));
	connect(checkBox_showTree, SIGNAL(stateChanged(int)), this, SLOT(showTree_triggered()));
	connect(checkBox_showImage, SIGNAL(stateChanged(int)), this, SLOT(showImage_triggered()));
	connect(checkBox_randomRule, SIGNAL(stateChanged(int)), this, SLOT(useRandomRules_triggered()));
	connect(checkBox_useSetBranchAngle, SIGNAL(stateChanged(int)), this, SLOT(useSetBranchAngle_triggered())); 
	connect(checkBox_showSampledPoints, SIGNAL(stateChanged(int)), this, SLOT(showSampledPoints_triggered()));

	connect(doubleSpinBox_weightThrs, SIGNAL(valueChanged(double)), this, SLOT(updateWeightThrs_triggered(double)));
	connect(doubleSpinBox_centerDisThrs, SIGNAL(valueChanged(double)), this, SLOT(updateCenterDisThrs_triggered(double)));
	connect(doubleSpinBox_cornerDisThrs, SIGNAL(valueChanged(double)), this, SLOT(updatecornerDisThrs_triggered(double)));
	connect(doubleSpinBox_mianBranchAngle, SIGNAL(valueChanged(double)), this, SLOT(updatemianBranchAngleThrs_triggered(double)));
	//connect(comboBox_methods, SIGNAL(&QComboBox::currentIndexChanged), this, SLOT(updatecomboBox_OptAlgorithms_triggered(QString)));
	connect(comboBox_methods, SIGNAL(currentIndexChanged(QString)), this, SLOT(updatecomboBox_OptAlgorithms_triggered(const QString &)));
	//connect(comboBox_methods, SIGNAL(&QComboBox::currentIndexChanged), [=](const QString &text){ /* ... */ });
}

void MainWindow::update()
{
    viewer->repaint();
	//viewer->update();
}

void MainWindow::loadLSystem_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open L-system"), ".l");
    if (fileName.isEmpty()) return;

	std::cerr << "Load L-system rules..." << std::endl;
	//QApplication::setOverrideCursor(Qt::WaitCursor);
	m_scene.load_rules(fileName);
    //QApplication::restoreOverrideCursor();
    update();
    std::cerr << "done." << std::endl;
}

void MainWindow::drawLSystem_triggered(){
	std::cerr << "Draw L-system rule...";
	QApplication::setOverrideCursor(Qt::WaitCursor);
	//m_scene.draw_rules();
	m_scene.draw_rules_random();
	QApplication::restoreOverrideCursor();
	update();
	std::cerr << "done." << std::endl;
}

void MainWindow::actionRedner_triggered(){
	if (checkBox_randomRule->isChecked()){
		viewer->render_images_all_FRCNN_random();
		//viewer->render_images_all_FRCNN_random_test();
		//viewer->render_current_image();
	}
	else{
		//viewer->render_images_all_FRCNN();
		//viewer->render_images_all_FRCNN_test();
		viewer->render_current_image();
	}
}

void MainWindow::loadParsingImg_triggered(){
	std::cerr << "Load parsing images...";
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Images"), ".jpg");
	if (fileName.isEmpty()) return;
	viewer->parsing_image(fileName);
	std::cerr << "done." << std::endl;
}

void MainWindow::parseGrammarImg_triggered(){
	viewer->build_NaryTree();
}

void MainWindow::buildGraph_triggered(){
	viewer->build_graph();
}

void MainWindow::extractSpanningTree_triggered(){
	viewer->remove_cycles();
	viewer->extract_minimal_spanning_tree();
}

//////////
// VIEW //
//////////
void MainWindow::showDialog_triggered()
{
	if(dockWidget_optionsDialog->isVisible()){
	  dockWidget_optionsDialog->setVisible(false);
	}
	else{
		dockWidget_optionsDialog->setVisible(true);
	}
}

void MainWindow::updateLineWidth_triggered(double s){
	viewer->line_thickness() = s;
	update();
}

void MainWindow::updatebbxID_triggered(int id){
	viewer->set_bbxID() = id;
	update();
}

void MainWindow::showTreeRender_triggered(){
	if (checkBox_showTreeRender->isChecked()){
		viewer->set_show_tree_render(true);
	}
	else{
		viewer->set_show_tree_render(false);
	}
	update();
}

void MainWindow::showTree_triggered(){
	if(checkBox_showTree->isChecked()){
		viewer->set_show_tree(true);
	}
	else{
		viewer->set_show_tree(false);
	}
	update();
}

void MainWindow::showImage_triggered(){
	if (checkBox_showImage->isChecked()){
		viewer->set_show_image(true);
	}
	else{
		viewer->set_show_image(false);
	}
	update();
}

void MainWindow::useRandomRules_triggered(){
	if (checkBox_randomRule->isChecked()){
		m_scene.set_random_rules(true);
	}
	else{
		m_scene.set_random_rules(false);
	}
	update();
}

void MainWindow::useSetBranchAngle_triggered(){
	if (checkBox_useSetBranchAngle->isChecked()){
		m_scene.lSystem()->use_set_branchAngle(true);
	}
	else{
		m_scene.lSystem()->use_set_branchAngle(false);
	}
	update();
}

void MainWindow::showSampledPoints_triggered(){
	if (checkBox_showSampledPoints->isChecked()){
		viewer->set_show_sampledPoints(true);
	}
	else{
		viewer->set_show_sampledPoints(false);
	}
	update();
}

//////////
// Dialog //
//////////
void MainWindow::updateIteration_triggered(int iter){
	m_scene.lSystem()->set_iterations(iter);
}

void MainWindow::updateBranchAngle_triggered(double angle){
	m_scene.lSystem()->set_branchingAngle(angle) ;
}

void MainWindow::updateAnglePerturbation_triggered(double angle){
	m_scene.lSystem()->set_angle_perturbation(angle);
}

void MainWindow::updateWeightThrs_triggered(double thrs){
	viewer->set_weight_thrs() = thrs;
	viewer->build_graph();
	update();
}

void MainWindow::updateCenterDisThrs_triggered(double thrs){
	viewer->set_centerDis_thrs() = thrs;
	viewer->build_graph();
	update();
}

void MainWindow::updatecornerDisThrs_triggered(double thrs){
	viewer->set_cornerDis_thrs() = thrs;
	viewer->build_graph();
	update();
}

void MainWindow::updatemianBranchAngleThrs_triggered(double thrs){
	viewer->set_mianBranchAngle_thrs() = thrs;
}

void MainWindow::updatecomboBox_OptAlgorithms_triggered(const QString &text){
	int aa = 0;
	viewer->set_optAlgorithm() = text.toStdString();
}