#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <string>
#include <iostream>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QKeyEvent>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QtConcurrent>
#include "core.h"
#include "utility.h"
#include "previewwidget.h"
#include "eigenutility.h"
#include "imagewidget.h"

using namespace std;

namespace
{
    Core& core = Core::getInstance();

    // For sliders
    const int maxValue = 1000;
    const int minValue = 1;
    inline double intToDouble(int i) { return static_cast<double>(i - minValue) / static_cast<double>(maxValue - minValue); }
    inline int doubleToInt(double d) { return static_cast<int>(d * (maxValue - minValue) + minValue); }

    // For labels
    inline void setText(shared_ptr<QLineEdit> lineEdit, const double value)
    {
        QString buffer;
        buffer.sprintf("%.2f", value);
        lineEdit->setText(buffer);
    }

    // For window control
    const string windowName("SelPh");
}

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    core.mainWindow = this;
    setWindowTitle(windowName.c_str());

    // set preview widget
    ui->previewwidget->setMinimumWidth(420);
    core.previewWidget = ui->previewwidget;

    // set user interface size
    changeUiSize(core.uiSize);

#if 0
    // Disable the reference widget
    ui->scrollArea_reference->setVisible(false);
#endif

    // others
    ui->checkBox_vis->setChecked(core.useVisualization);
    ui->checkBox_opt->setChecked(core.useOptimization);
    updateConfidenceValueInUI(0.0);
    ui->scrollAreaWidgetContents_reference->setAutoFillBackground(true);
    ui->scrollAreaWidgetContents_reference->setBackgroundRole(QPalette::ColorRole::Dark);

    // initialize sliders
    initializeSliders();

    // set the directory path
    QString dirPath;
    dirPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), QString((Utility::getResourceDirectory() + "/data/").c_str()), QFileDialog::ShowDirsOnly);
    dirPath += "/";

    // initialize images
    QProgressDialog dialog(QString("Loading image files..."), QString(), 0, 0, this);
    QFutureWatcher<void> watcher;
    QObject::connect(&watcher, SIGNAL(finished()), &dialog, SLOT(reset()));
    watcher.setFuture(QtConcurrent::run([dirPath] ()
                                        {
                                            core.initialize(dirPath.toStdString());
                                        }));
    dialog.exec();
    watcher.waitForFinished();

    // status bar
    ui->statusBar->showMessage(QString::number(core.currentIndex + 1) + QString(" / ") + QString::number(core.images.size()));

    // finalize
    this->adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) // change the baseline size of GUI widgets (temporarily disable the window updates)
    {
        window()->setUpdatesEnabled(false);
        changeUiSize((core.uiSize + 1) % 4);
        QKeyEvent* updateEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::ShiftModifier);
        QCoreApplication::postEvent(this, updateEvent);
    }
    else if (event->modifiers() == Qt::ShiftModifier && event->key() == Qt::Key_Escape) // repaint the window (enable the window updates)
    {
        window()->setUpdatesEnabled(true);
        repaint();
    }
}

void MainWindow::changeUiSize(int size)
{
    core.uiSize = size;
    QApplication* a = qApp;
    QFont font("Helvetica Neue", core.getSizeOfFont(), QFont::Normal, false);
    a->setFont(font);
    ui->scrollArea->setMinimumWidth(core.getSizeOfScrollAreaWidth());
    for (shared_ptr<QLineEdit> edit : edits)
    {
        edit->setMinimumHeight(core.getSizeOfTextBoxHeight());
        edit->setMaximumHeight(core.getSizeOfTextBoxHeight());
        edit->setMinimumWidth(core.getSizeOfTextBoxWidth());
        edit->setMaximumWidth(core.getSizeOfTextBoxWidth());
    }
    for (shared_ptr<VisualizationWidget> vWidget : visualizationWidgets)
    {
        vWidget->setMaximumHeight(core.getSizeOfVisualizationHeight());
        vWidget->setMinimumHeight(core.getSizeOfVisualizationHeight());
    }
    ui->nextButton->setFixedHeight(core.getSizeOfButtonHeight());
    ui->pushButton_auto->setFixedHeight(core.getSizeOfButtonHeight());
    repaint();
}

void MainWindow::updateUIFromParameters()
{
    for (unsigned i = 0; i < sliders.size(); ++ i)
    {
        shared_ptr<QSlider>   s = sliders[i];
        shared_ptr<QLineEdit> l = edits[i];
        s->setValue(doubleToInt(core.parameters[i]));
        setText(l, core.parameters[i]);
    }
}

void MainWindow::updateConfidenceValueInUI(double confidence)
{
    if (core.currentIndex > 1)
    {
        ui->label_conf->setText(QString("Confidence: ") + QString::number(confidence * 100.0, 'f', 1) + QString("%"));
    }
    else
    {
        ui->label_conf->setText(QString("Confidence: <font color=gray>(N/A)</font>%"));
    }
}

void MainWindow::updateParametersBySlider()
{
    // detect focused slider
    int focused = -1;
    for (int i = 0; i < core.parameterDim; ++ i)
    {
        if (sliders[i]->isSliderDown()) focused = i;
    }

    // user study
    const double before = core.parameters[focused];
    const double after  = intToDouble(sliders[focused]->value());
    core.getCurrentStudyData().sliderMovementSum += fabs(after - before);

    // update core parameters from sliders
    const shared_ptr<QSlider> s = sliders[focused];
    const double              v = intToDouble(s->value());
    core.parameters[focused] = v;

    // optimization
    int n = core.useOptimization ? core.nIterations : 0;

    for (int i = 0; i < n; ++ i)
    {
        core.optimizeParameters(focused);
    }

    // update all sliders
    for (int i = 0; i < core.parameterDim; ++ i) {
        shared_ptr<QSlider>   s = sliders[i];
        shared_ptr<QLineEdit> l = edits[i];

        const double value = core.parameters[i];
        setText(l, value);
        if (i != focused) s->setValue(doubleToInt(value));
    }

    // user study
    core.getCurrentStudyData().parameterSequence.push_back(EigenUtility::std2eigen(core.parameters));

    // refresh
    for (shared_ptr<VisualizationWidget> vw : visualizationWidgets)
    {
        vw->repaint();
    }
    core.previewWidget->repaint();
}

void MainWindow::updateParametersByText() {
    cerr << "Updating-parameters-by-text is not supported." << endl;
    return;

    for (int i = 0; i < core.parameterDim; ++ i) {
        shared_ptr<QSlider>   s = sliders[i];
        shared_ptr<QLineEdit> l = edits[i];

        double value = l->text().toDouble();

        s->setValue(doubleToInt(value));
        core.parameters[i] = value;
    }

    // user study
    core.getCurrentStudyData().parameterSequence.push_back(EigenUtility::std2eigen(core.parameters));

    // refresh
    for (shared_ptr<VisualizationWidget> vw : visualizationWidgets)
    {
        vw->repaint();
    }
    core.previewWidget->repaint();
}

#define COLORBALANCE

void MainWindow::initializeSliders() {
#ifdef COLORBALANCE
    core.parameterDim = 6;
    core.parameters.resize(core.parameterDim, 0.5);

    const string names[] =
    {
        "Brightness",
        "Contrast",
        "Saturation",
        "Balance (R)",
        "Balance (G)",
        "Balance (B)"
    };
#else
    core.dimension = 3;
    core.parameters.resize(core.dimension, 0.5);

    const string names[] =
    {
        "Brightness",
        "Contrast",
        "Saturation",
    };
#endif

    for (int i = 0; i < core.parameterDim; ++ i)
    {
        generateSliderComponent(names[i], i, i + 1 == core.parameterDim);
    }
    ui->sliderGridLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), core.parameterDim * 2, 0);
}

/////////////////////////////////////////////////////////
// Reference photos
/////////////////////////////////////////////////////////

void MainWindow::setReferencePhotos(const std::vector<std::shared_ptr<QImage>>& images)
{
    referenceImages = images;
}

void MainWindow::clearReferenceLayout()
{
    QLayoutItem *child;
    while ((child = ui->scrollAreaWidgetContents_reference->layout()->takeAt(0)) != nullptr)
    {
        delete child;
    }
}

void MainWindow::generateReferenceLayout()
{
    const int height = ui->scrollAreaWidgetContents_reference->height();
    for (const shared_ptr<QImage> q : referenceImages)
    {
        ui->scrollAreaWidgetContents_reference->layout()->addWidget(new ImageWidget(q, height));
    }
    ui->scrollAreaWidgetContents_reference->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
}

/////////////////////////////////////////////////////////
// miscs
/////////////////////////////////////////////////////////

void MainWindow::generateSliderComponent(const string& name, const int index, const bool last)
{
    // generate a slider
    shared_ptr<QSlider> s = make_shared<QSlider>(Qt::Horizontal, this);
    s->setMinimum(minValue);
    s->setMaximum(maxValue);
    s->setValue(doubleToInt(core.parameters[index]));
    s->setMinimumWidth(200);
    QObject::connect(s.get(), SIGNAL(sliderMoved(int)), this, SLOT(updateParametersBySlider()));
    QObject::connect(s.get(), SIGNAL(sliderPressed()), this, SLOT(updateParametersBySlider()));

    // generate a textedit
    shared_ptr<QLineEdit> l = make_shared<QLineEdit>(this);
    setText(l, core.parameters[index]);
    l->setMinimumWidth(core.getSizeOfTextBoxWidth());
    l->setMaximumWidth(core.getSizeOfTextBoxWidth());
    l->setMinimumHeight(core.getSizeOfTextBoxHeight());
    l->setMaximumHeight(core.getSizeOfTextBoxHeight());
    QObject::connect(l.get(), SIGNAL(editingFinished()), this, SLOT(updateParametersByText()));

    // generate a visualization widget
    shared_ptr<VisualizationWidget> w = make_shared<VisualizationWidget>(this, index);

    // register them
    sliders.push_back(s);
    edits.push_back(l);
    visualizationWidgets.push_back(w);

    // add them to user interface
    ui->sliderGridLayout->addWidget(s.get(), index * 5, 1);
    ui->sliderGridLayout->addWidget(l.get(), index * 5, 2);
    ui->sliderGridLayout->addWidget(w.get(), index * 5 + 1, 1);

    // add a label
    ui->sliderGridLayout->addWidget(new QLabel(name.c_str(), this), index * 5, 0);

    if (last) return;

    // add a line to the interface
    ui->sliderGridLayout->addItem(new QSpacerItem(0, 3, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding), index * 5 + 2, 0, 1, 3);
    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    ui->sliderGridLayout->addWidget(line, index * 5 + 3, 0, 1, 3);
    ui->sliderGridLayout->addItem(new QSpacerItem(0, 3, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding), index * 5 + 4, 0, 1, 3);
}

void MainWindow::on_checkBox_vis_clicked()
{
    core.useVisualization = ui->checkBox_vis->isChecked();
    for (shared_ptr<VisualizationWidget> vw : visualizationWidgets)
    {
        vw->repaint();
    }
}

void MainWindow::on_checkBox_opt_clicked()
{
    core.useOptimization = ui->checkBox_opt->isChecked();
    for (shared_ptr<VisualizationWidget> vw : visualizationWidgets)
    {
        vw->repaint();
    }
}

void MainWindow::on_nextButton_clicked()
{
    shared_ptr<QProgressDialog> dialog = make_shared<QProgressDialog>(QString("Please wait..."), QString(), 0, 0, this);
    QFutureWatcher<void> watcher;
    QObject::connect(&watcher, SIGNAL(finished()), dialog.get(), SLOT(reset()));

    // Prevent widgets from updating
    window()->setUpdatesEnabled(false);

    // run the candidate generation in background
    bool result;
    watcher.setFuture(QtConcurrent::run([&result] ()
                                        {
                                            result = core.goNext();
                                        }));

    // show a progress dialog in foreground
    dialog->exec();

    watcher.waitForFinished();

    // finish
    if (!result) exit(0);

    // Reference photos
    clearReferenceLayout();
    generateReferenceLayout();
    ui->scrollArea_reference->ensureVisible(0, 0);

    // status bar
    ui->statusBar->showMessage(QString::number(core.currentIndex + 1) + QString(" / ") + QString::number(core.images.size()));

    // Re-enable updating widgets
    window()->setUpdatesEnabled(true);

    updateUIFromParameters();
    core.previewWidget->repaint();
    for (shared_ptr<VisualizationWidget> vw : visualizationWidgets)
    {
        vw->repaint();
    }
}

void MainWindow::on_pushButton_auto_clicked()
{
    // Exception
    if (core.goodnessFunction.getParameterList().empty()) return;

    // Set the optimal parameter set
    Eigen::VectorXd x = core.isBaselineMode ? core.goodnessFunction.getAverageParameterSet() : core.goodnessFunction.getBestParameterSet(core.getCurrentFeatureVector());
    core.parameters.clear();
    for (unsigned i = 0; i < x.rows(); ++ i) core.parameters.push_back(x(i));
    updateUIFromParameters();

    // User study
    core.getCurrentStudyData().autoEnhanced = true;
    core.getCurrentStudyData().parameterSequence.push_back(EigenUtility::std2eigen(core.parameters));

    // Repaint
    core.previewWidget->repaint();
    for (shared_ptr<VisualizationWidget> vw : visualizationWidgets) {
        vw->repaint();
    }
}

void MainWindow::on_actionExport_transformed_feature_coordinates_triggered()
{
    core.printFeatureCoordinates();
}

void MainWindow::on_actionRestart_timer_triggered()
{
    core.time_point = std::chrono::system_clock::now();
}

void MainWindow::on_actionDisable_functions_triggered()
{
    ui->checkBox_vis->setCheckable(false);
    ui->checkBox_opt->setChecked(false);
    ui->checkBox_vis->setDisabled(true);
    ui->checkBox_opt->setDisabled(true);
    ui->label_conf->setVisible(false);

    core.useVisualization = false;
    core.useOptimization  = false;
    core.useSortingPhotos = false;

    core.isBaselineMode = true;
}
