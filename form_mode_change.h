#ifndef FORM_MODE_CHANGE_H
#define FORM_MODE_CHANGE_H

#include <QWidget>
#include<QButtonGroup>
#include<QSettings>
namespace Ui {
class Form_mode_change;
}

class Form_mode_change : public QWidget
{
    Q_OBJECT

public:
    explicit Form_mode_change(QWidget *parent = nullptr);
    ~Form_mode_change();
    QButtonGroup *buttonGroup;
    QString *mode;



private:
    Ui::Form_mode_change *ui;

private slots:
    void onLocalRadioButtonClicked(bool checked);
    void onClouldRadioButtonClicked(bool checked);

    void on_pushButton_clicked();

signals:
    // 自定义信号，用于通知模式改变
    void modeChanged(const QString &newMode);
    void push_clicked();
};

#endif // FORM_MODE_CHANGE_H
