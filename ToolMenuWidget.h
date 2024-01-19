#ifndef TOOLMENUWIDGET_H
#define TOOLMENUWIDGET_H

#include <QWidget>

#include <QSettings>

namespace Ui {
class ToolMenuWidget;
}

class QComboBox;
class QAbstractButton;
class QButtonGroup;

class ToolMenuWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ToolMenuWidget(QWidget *parent = nullptr);
    ~ToolMenuWidget();

    void setSegmentEnable(bool enable);
    void setTeethAlignEnable(bool enable);
    void setAdjustEnable(bool enable);
signals:
    void signalLoadOnnxModel(QString path);
    void signalLoadDentalModel(QString path);
    void signalStartSegment();
    void signalEnableBrush(bool);
    void signalUndoBrush();
    void signalRedoBrush();
    void signalSetBrushColor(int);
    void signalDeleteBrushedCells();
    void signalExport();
    void signalToTeethAlignStep();

    void signallambdaChanged(int newLambda);
    void signalRoundChanged(int newRound);
private slots:
    void slotchooseOnnxBtn();
    void slotChooseDentalBtn();
    void slotRadioBtn(QAbstractButton*);

    void lambdaPressed();
    void roundpressed();

private:
    void initWidget();

    void updateComboBox(QComboBox* comboBox, const QString& fileName);

    Ui::ToolMenuWidget *ui;

    QStringList mOnnxPathList;
    QStringList mDentalPathList;

    QSettings mSettings;
};

#endif // TOOLMENUWIDGET_H
