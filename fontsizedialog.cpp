// FontSizeDialog.cpp
#include "FontSizeDialog.h"


FontSizeDialog::FontSizeDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("设置字体大小"));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // // 字体选择
    // fontComboBox = new QFontComboBox(this);
    // mainLayout->addWidget(fontComboBox);

    // 大小选择
    spinBox = new QSpinBox(this);
    spinBox->setMinimum(8);
    spinBox->setMaximum(72);
    mainLayout->addWidget(spinBox);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton(tr("确定"), this);
    QPushButton *cancelButton = new QPushButton(tr("取消"), this);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(okButton, &QPushButton::clicked, this, &FontSizeDialog::onOkClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

int FontSizeDialog::fontSize() const
{
    return spinBox->value();
}

void FontSizeDialog::onOkClicked()
{
    accept();
}

void FontSizeDialog::setDefaultFontSize(int size)
{
    spinBox->setValue(size); // 使用setValue方法设置QSpinBox的默认值
}
