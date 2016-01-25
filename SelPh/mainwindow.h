#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <QMainWindow>
#include <QSlider>
#include <QLineEdit>
#include <QTimer>
#include "visualizationwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void changeUiSize(int size);
    void updateUIFromParameters();
    void updateConfidenceValueInUI(double confidence);

    void setReferencePhotos(const std::vector<std::shared_ptr<QImage>>& images);

public slots:
    void updateParametersBySlider();
    void updateParametersByText();
    
private slots:
    void on_checkBox_vis_clicked();

    void on_checkBox_opt_clicked();

    void on_nextButton_clicked();

    void on_pushButton_auto_clicked();

    void on_actionExport_transformed_feature_coordinates_triggered();

    void on_actionRestart_timer_triggered();

    void on_actionDisable_functions_triggered();

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    Ui::MainWindow *ui;
    void initializeSliders();

    std::vector<std::shared_ptr<QImage>> referenceImages;
    void clearReferenceLayout();
    void generateReferenceLayout();

    void generateSliderComponent(const std::string &name, const int index, const bool last = false);

    std::vector<std::shared_ptr<QSlider>>             sliders;
    std::vector<std::shared_ptr<QLineEdit>>           edits;
    std::vector<std::shared_ptr<VisualizationWidget>> visualizationWidgets;
};

#endif // MAINWINDOW_H
