#ifndef _WINDOW_
#define _WINDOW_

// Qt
#include <QWidget>
#include <QString>
#include <QMainWindow>
#include <QComboBox>
// local
#include "scene.h"
#include "ui_mainwindow.h"

class MainWindow : public QMainWindow, public Ui_MainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow();
	static MainWindow* getInstance();
    void init_slots();

protected slots:
    // io
    void update();

	void loadLSystem_triggered();
	void drawLSystem_triggered();
	void actionRedner_triggered();
	void loadParsingImg_triggered();
	void parseGrammarImg_triggered();
	void buildGraph_triggered();
	void extractSpanningTree_triggered();

    // view
    void showDialog_triggered();  
	void updateLineWidth_triggered(double s);
	void showTreeRender_triggered();
	void showTree_triggered();
	void showImage_triggered();
	void useRandomRules_triggered();
	void useSetBranchAngle_triggered();
	void showSampledPoints_triggered();

	// dialog
	void updateIteration_triggered(int iter);
	void updateBranchAngle_triggered(double angle);
	void updatebbxID_triggered(int id);
	void updateAnglePerturbation_triggered(double angle);

	void updateWeightThrs_triggered(double thrs);
	void updateCenterDisThrs_triggered(double thrs); 
	void updatecornerDisThrs_triggered(double thrs);
	void updatemianBranchAngleThrs_triggered(double thrs);
	void updatecomboBox_OptAlgorithms_triggered(const QString &text);
signals:
    //void openRecentFile(QString filename);

private:
	//Ui_MainWindow m_ui;
    Scene m_scene;
};

#endif // _WINDOW_
