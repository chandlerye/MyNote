#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSql>
#include <QSqlDatabase>
#include<QSqlQuery>
#include<QSqlTableModel>
#include<QMessageBox>
#include<QListWidgetItem>
#include <QSqlError>
#include<QContextMenuEvent>
#include<QTimer>
#include<QFileDialog>
#include<QSqlRecord>
#include<QTextCodec>
#include<QDesktopServices>
#include<QSplitter>

#include<form_sql_config.h>
#include<form_mode_change.h>
#include<fontsizedialog.h>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 数据库相关操作
    QSqlDatabase db; // 数据库连接实例
    QSqlTableModel *model; // 数据模型，用于显示数据
    void loadTitlesFromDatabase(); // 从数据库加载笔记标题
    void updateNoteContent(); // 更新当前显示的笔记内容

    // 列表项管理
    void deleteSelectedItem(); // 删除选中的笔记项
    QMenu *rightClickMenu; // 右键菜单实例
    bool eventFilter(QObject *watched, QEvent *event) override; // 事件过滤器，用于处理特定事件

    // 文本编辑相关
    QString m_previousTitle; // 记录上一个笔记标题
    void onTextEditContentChanged(); // 文本编辑内容变化时的处理
    bool first_push = true; // 标记是否首次操作
    bool is_newAdd = false; // 标记是否为新增笔记
    void onItemTextChanged_doublecliked(QListWidgetItem *item); // 双击列表项时的处理
    bool is_lastDelete = false; // 记录最近是否有删除操作
    void onActionTriggered(); // 数据库配置窗口的确定修改操作

    // 数据库心跳检测与连接管理
    void ask_check(); // 心跳检测，保持连接活跃
    void connect_manage(); // 数据库连接管理

    // 数据库初始化
    bool setupDatabase(QString *); // 根据参数配置数据库连接

    // 导出功能
    void exportToCSV(); // 导出笔记到CSV文件

    // 数据库配置窗口指针
    Form_sql_config *w_sql; // 数据库配置窗口对象

    // 文本编辑状态标记
    bool isTextEditFocused = false; // 文本编辑框是否获得焦点
    bool isEditingDueToDoubleClick = false; // 是否因双击进入编辑
    bool editingFinished = true; // 编辑状态是否结束

    // 列表项置顶相关操作
    QVector<int> fetchAllIds(); // 获取所有ID
    void onTopItemAt(QListWidgetItem *item); // 置顶操作
    void onCancelTopItemAt(QListWidgetItem *item); // 取消置顶操作
    void toggleTopItemAt(QListWidgetItem *item); // 切换置顶状态
    void showHelp(); // 显示帮助信息

    // 数据库模式切换相关
    void onModelChangeTriggered(); // 触发数据库模式切换
    Form_mode_change *w_mode_c; // 数据库模式切换窗口对象
    bool falg_set_successful; // 数据库模式设置成功标志

    // 右键菜单动作
    QAction *topAction; // 置顶菜单动作
    QAction *deleteAction; // 删除菜单动作

    // 信号槽与设置
    void setupConnections(); // 初始化信号槽连接
    void notefontset(); // 设置字体大小
    void readSettings(); // 读取设置
    void showNoteForItem(QListWidgetItem *item); // 根据列表项显示笔记内容

    // 界面组件初始化
    void initializeUIComponents(); // 初始化界面元素

    // 数据库模式初始化
    bool initializeDatabaseMode(); // 根据配置初始化数据库模式

    QListWidgetItem* addItemToWidget(const QString &title, int sortOrder, int idOrder, QFont item_font, QListWidget *listWidget);


    // item的字体大小
    QFont item_font;
    // 水平调整
    // QSplitter *splitter;


private slots:

    void on_inset_note_clicked(); // 插入新笔记按钮点击处理
    void handleDataFromFormsql(); // 处理数据库配置窗体返回的数据
    void showInfoMessage(); // 显示信息提示框
    void handleModeChange(const QString &newMode); // 处理数据库模式变更的信号

private:
    Ui::MainWindow *ui;
    QColor highlightedBackgroundColor; // 高亮背景颜色
    QColor highlightedBackgroundColor2; // 另一个高亮背景颜色

};
#endif // MAINWINDOW_H
