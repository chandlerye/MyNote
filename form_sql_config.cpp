#include "form_sql_config.h"
#include "ui_form_sql_config.h"

Form_sql_config::Form_sql_config(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Form_sql_config)
{
    ui->setupUi(this);

    // 从QSettings加载数据到每个lineEdit
    QSettings settings("MyNote","setting");
    for (int i = 0; i < 6; ++i) {
        QString settingKey = QString("lineEdit_%1").arg(i + 1); // 匹配lineEdit_1至lineEdit_6
        QString savedText = settings.value(settingKey).toString();

        // 如果设置中有值，则从QSettings加载；否则直接使用lineEdit的初始值（可能是空）
        QLineEdit* lineEdit = findChild<QLineEdit*>(QString("lineEdit_%1").arg(i + 1));
        if (lineEdit) {

            // qDebug()<<lineEdit->text()<<savedText;
            lineEdit->setText(savedText.isEmpty() ? lineEdit->text() : savedText);
            // 初始化config数组
            config[i] = lineEdit->text();
        }
    }

    connect(ui->pushButton,&QPushButton::clicked,this, &Form_sql_config::sendDataToParent);

    connect(ui->lineEdit_1, &QLineEdit::textChanged, this, &Form_sql_config::onTextChanged);
    connect(ui->lineEdit_2, &QLineEdit::textChanged, this, &Form_sql_config::onTextChanged);
    connect(ui->lineEdit_3, &QLineEdit::textChanged, this, &Form_sql_config::onTextChanged);
    connect(ui->lineEdit_4, &QLineEdit::textChanged, this, &Form_sql_config::onTextChanged);
    connect(ui->lineEdit_5, &QLineEdit::textChanged, this, &Form_sql_config::onTextChanged);
    connect(ui->lineEdit_6, &QLineEdit::textChanged, this, &Form_sql_config::onTextChanged);

}

Form_sql_config::~Form_sql_config()
{
    delete ui;
}



void Form_sql_config::onTextChanged(const QString &text)
{

 // 确定哪个lineEdit发送了信号并更新相应的config元素
 QLineEdit *senderLineEdit = qobject_cast<QLineEdit*>(sender());
 if (senderLineEdit) {
     int index = senderLineEdit == ui->lineEdit_1 ? 0 :
                     senderLineEdit == ui->lineEdit_2 ? 1 :
                     senderLineEdit == ui->lineEdit_3 ? 2 :
                     senderLineEdit == ui->lineEdit_4 ? 3 :
                     senderLineEdit == ui->lineEdit_5 ? 4 :
                     senderLineEdit == ui->lineEdit_6 ? 5 : -1;

     if (index >= 0) {
         config[index] = text;
         qDebug()<<text;
         // 保存到QSettings
         QSettings settings("MyNote","setting");

         settings.setValue(QString("lineEdit_%1").arg(index + 1), text);

         settings.sync();
     }
 }

}
