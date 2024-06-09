#include "form_mode_change.h"
#include "ui_form_mode_change.h"

Form_mode_change::Form_mode_change(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Form_mode_change)
{
    ui->setupUi(this);



    this->setFixedSize(350, 100);
    // 创建ButtonGroup并添加RadioButton
    buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->local_radioButton);
    buttonGroup->addButton(ui->clould_radioButton);
    mode = new QString;

    QSettings settings("MyNote","setting");

    QString value = settings.value("mode").toString();

    if (!value.isEmpty()) {
        *mode = value; // 确保mode指向的QString对象被赋予settings中的"value"值，且非空

        if (*mode == "QSQLITE") {
            ui->local_radioButton->setChecked(true);
        } else if (*mode == "QMYSQL") {
            ui->clould_radioButton->setChecked(true);

        }
    } else {
        *mode = "QSQLITE"; // 如果value为空，则默认设置mode为"QSQLITE"

        ui->local_radioButton->setChecked(true); // 并且默认选中本地数据库RadioButton
    }


    // 连接信号和槽
    connect(ui->local_radioButton, &QRadioButton::clicked, this, &Form_mode_change::onLocalRadioButtonClicked);
    connect(ui->clould_radioButton, &QRadioButton::clicked, this, &Form_mode_change::onClouldRadioButtonClicked);

}

Form_mode_change::~Form_mode_change()
{
    delete ui;
    delete mode;
}

void Form_mode_change::onLocalRadioButtonClicked(bool checked)
{
    if (checked) {
        *mode = "QSQLITE";

        emit modeChanged(*mode); // 发射模式改变信号
        qDebug()<<*mode;
        // 保存到QSettings
        QSettings settings("MyNote","setting");

        settings.setValue(QString("mode"),*mode);

        settings.sync();
    }
}

void Form_mode_change::onClouldRadioButtonClicked(bool checked)
{
    if (checked) {
        *mode = "QMYSQL";

        emit modeChanged(*mode); // 发射模式改变信号
        qDebug()<<*mode;

        // 保存到QSettings
        QSettings settings("MyNote","setting");

        settings.setValue(QString("mode"),*mode);

        settings.sync();
    }
}

void Form_mode_change::on_pushButton_clicked()
{
    emit push_clicked();
}

