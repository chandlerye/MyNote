#ifndef FORM_SQL_CONFIG_H
#define FORM_SQL_CONFIG_H

#include <QWidget>
#include<QSettings>
namespace Ui {
class Form_sql_config;
}

class Form_sql_config : public QWidget
{
    Q_OBJECT

public:

    explicit Form_sql_config(QWidget *parent = nullptr);
    ~Form_sql_config();
    QString config[6];

signals:
    void sendDataToParent();
private slots:
    void onTextChanged(const QString &text);


private:
   Ui::Form_sql_config *ui;
};

#endif // FORM_SQL_CONFIG_H
