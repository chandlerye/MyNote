// FontSizeDialog.h
#ifndef FONTSIZE_DIALOG_H
#define FONTSIZE_DIALOG_H

#include <QDialog>
#include<QFontComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include<QSpinBox>

class QFontComboBox;
class QLabel;
class QPushButton;
class QSpinBox;

class FontSizeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FontSizeDialog(QWidget *parent = nullptr);
    int fontSize() const;
    void setDefaultFontSize(int size);

private slots:
    void onOkClicked();

private:
    QFontComboBox *fontComboBox;
    QSpinBox *spinBox;
};

#endif // FONTSIZE_DIALOG_H
