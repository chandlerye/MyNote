#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initializeUIComponents();//组件初始化

    falg_set_successful = initializeDatabaseMode();//数据库初始化，显示初始化

    showNoteForItem(ui->listWidget->item(0));  // 首项显示

    // 消息栏显示
    if(falg_set_successful){ui->statusbar->showMessage("欢迎使用MyNote");}
    else{ui->statusbar->showMessage("初始化失败");}

    ask_check();     // 心跳连接

    setupConnections();  //信号连接

    readSettings();  //读取字体设置
}

MainWindow::~MainWindow()
{
    delete ui;
    // 关闭数据库连接，释放资源
    db.close();
}
// ==================================================================================
// initializeDatabaseMode()函数：用于初始化主窗口的UI组件，包括设置背景色、创建数据库配置和模式
// 配置的表单实例、设置右键菜单选项、安装事件过滤器、以及配置文本编辑器的字体样式。
// ==================================================================================
void MainWindow::initializeUIComponents()
{
    highlightedBackgroundColor = QColor(200, 255, 200); // 示例颜色，绿色背景
    highlightedBackgroundColor2 = QColor(255, 255, 255); // 示例颜色，绿色背景
    // 创建数据库配置表单实例
    w_sql = new Form_sql_config;
    //模式配置实例
    w_mode_c = new Form_mode_change;

    // 右键菜单添加置顶/取消置顶选项
    rightClickMenu = new QMenu(this);
    topAction = new QAction(tr("置顶/取消置顶"), this);
    topAction->setIcon(QIcon(":/ico/top.ico"));
    rightClickMenu->addAction(topAction);


    //右键删除菜单
    deleteAction = new QAction(tr("删除"), this);
    deleteAction->setIcon(QIcon(":/ico/delete.ico"));
    rightClickMenu->addAction(deleteAction);

    // 为listWidget安装事件过滤器
    ui->listWidget->installEventFilter(this);
    // 为textEdit安装事件过滤器
    ui->textEdit->installEventFilter(this);

    //为笔记输入框设置字体
    QFont font = ui->textEdit->font(); // 获取当前字体
    font.setPointSize(14); // 设置字体大小为14号
    ui->textEdit->setFont(font); // 应用新字体到QTextEdit
}

// ==================================================================================
// initializeDatabaseMode()函数：数据库模式初始化
// ==================================================================================
bool MainWindow::initializeDatabaseMode()
{
    if(w_mode_c->mode[0] == "QSQLITE") {

        return setupDatabase(w_mode_c->mode);
        // 首项显示

    }
    else if(w_sql->config[0] == "QMYSQL") {
        // 首项显示

        return setupDatabase(w_sql->config);
    }

    return false;
}

// ==================================================================================
// showNoteForItem函数：根据提供的QListWidgetItem（列表项）显示对应的笔记内容。
// 它从列表项的用户角色中获取笔记ID，查询数据库以获取该ID对应的笔记内容，然后在textEdit中显示该内容。
// ==================================================================================
void MainWindow::showNoteForItem(QListWidgetItem *item)
{
    if (!item) {
        return;
    }
    int noteId = item->data(Qt::UserRole+1).toInt(); // 假设列表项的文本是笔记的标题
    qDebug()<<"noteid"<<noteId;
    // 查询数据库以获取与标题对应的笔记内容
    QSqlQuery query;
    query.prepare("SELECT note FROM notes WHERE id = :id");
    query.bindValue(":id",noteId);

    if (query.exec() && query.next()) {
        QString noteContent = query.value(0).toString(); // 获取第一条查询结果中的笔记内容
        ui->listWidget->setCurrentItem(item);
        ui->textEdit->setText(noteContent);
    } else {
        ui->textEdit->clear(); // 如果没有找到对应内容，清空textEdit
    }
}

// ==================================================================================
// setupConnections函数：信号连接
// ==================================================================================
void MainWindow::setupConnections()
{
    // 模式设置
    connect(w_mode_c, &Form_mode_change::modeChanged, this, &MainWindow::handleModeChange);

    // 双击修改标题的连接逻辑
    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item){
        if(item){
            isEditingDueToDoubleClick = true;
            connect(ui->listWidget, &QListWidget::itemChanged, this, &MainWindow::onItemTextChanged_doublecliked);
        }
    });

    // 切换显示
    connect(ui->listWidget, &QListWidget::itemClicked, this, &MainWindow::updateNoteContent);
    // 导出文件
    connect(ui->exportToCSV, &QAction::triggered, this, &MainWindow::exportToCSV);

    // 显示模式转换界面
    connect(ui->mode_change, &QAction::triggered, this, &MainWindow::onModelChangeTriggered);

    // 接受从数据库配置界面点击“确定修改”按钮传过来的信号
    connect(w_mode_c, &Form_mode_change::push_clicked, this, &MainWindow::onActionTriggered);
    connect(w_sql, &Form_sql_config::sendDataToParent, this, &MainWindow::handleDataFromFormsql);

    // 显示帮助
    connect(ui->AboutMyNote, &QAction::triggered, this, &MainWindow::showInfoMessage);
    // 使用说明
    connect(ui->use_help, &QAction::triggered, this, &MainWindow::showHelp);

    // 字体设置
    connect(ui->notefont, &QAction::triggered, this, &MainWindow::notefontset);

    // 右键菜单操作
    connect(topAction, &QAction::triggered, this, [this]() {
        QListWidgetItem *currentItem = ui->listWidget->currentItem();
        if (currentItem) {
            toggleTopItemAt(currentItem);
        }
    });
    // 右键删除
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteSelectedItem);
}

// ==================================================================================
// 字体相关功能实现
// ==================================================================================
/*
 * @brief MainWindow::notefontset
 * 允许用户自定义文本编辑器的字体大小。
 * 此功能通过弹出一个`FontSizeDialog`对话框，让用户选择或输入期望的字体大小，
 * 并在用户确认选择后，保存所选字体大小至应用程序设置，并即时应用到文本编辑器上。
 */
void MainWindow::notefontset()
{
    // 从QSettings读取之前的字体大小设置
    QSettings settings("MyNote", "setting");
    int savedFontSize = settings.value("FontSize", 12).toInt(); // 默认字体大小设为12，如果没有设置则使用默认值

    // 在显示对话框之前，先设置上次保存的字体大小（如果有的话）
    FontSizeDialog dialog(this);
    dialog.setDefaultFontSize(savedFontSize); // 假设FontSizeDialog有一个方法来设置默认字体大小

    if (dialog.exec() == QDialog::Accepted) {
        int fontSize = dialog.fontSize();

        // 保存到设置
        QSettings settings("MyNote","setting");

        settings.setValue(QString("FontSize"), fontSize);

        settings.sync();
        // 应用新的字体大小到你的文本编辑器部件（这里假设存在一个名为textEdit的QTextEdit部件）
        QFont font = ui->textEdit->font();
        font.setPointSize(fontSize);
        ui->textEdit->setFont(font);
    }
}
/*
 * @brief MainWindow::readSettings
 * 初始化应用程序时读取并应用上次保存的字体设置。
 * 该函数在程序启动时自动调用，确保文本编辑器组件的字体大小与用户上一次设置相同。
 */
void MainWindow::readSettings()
{
    QSettings settings("MyNote","setting");

    int fontSize = settings.value("FontSize").toInt(); // 默认字体大小设为12


    QFont font = ui->textEdit->font();
    font.setPointSize(fontSize);
    ui->textEdit->setFont(font);
}

// ==================================================================================
// handleModeChange函数：模式切换
// ==================================================================================
void MainWindow::handleModeChange(const QString &newMode)
{
    bool flagSetSuccessful = false;

    if (newMode == "QSQLITE") {

        flagSetSuccessful = setupDatabase(w_mode_c->mode);
    } else if (newMode == "QMYSQL") {

        flagSetSuccessful = setupDatabase(w_sql->config);
    } else {
        qDebug() << "Unsupported mode received!";
    }
    if (flagSetSuccessful) {
        qDebug() << "Database setup was successful.";
        showNoteForItem(ui->listWidget->item(0));  // 首项显示
        // 这里可以添加更多成功后的处理逻辑
    } else {
        qDebug() << "Failed to set up the database.";
        // 可以添加错误处理逻辑
    }
}

// ==================================================================================
// setupDatabase函数：设置数据库，数据库的初始化，显示初始数据
// ==================================================================================
bool MainWindow::setupDatabase(QString* sqlconfig)
{
    QString select_mode;
    if(sqlconfig[0]=="QSQLITE")
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        // 获取当前程序的工作目录
        QString currentDir = QDir::currentPath();
        // 拼接数据库文件名，例如 "mydatabase.db"
        QString dbName = "mydatabase.db";
        // 构建完整的数据库文件路径
        QString dbFilePath = currentDir + "/" + dbName;

        db.setDatabaseName(dbFilePath);
        select_mode = "本地模式";

    }else if(sqlconfig[0]=="QMYSQL")
    {
        db.setDatabaseName(sqlconfig[1]);
        db = QSqlDatabase::addDatabase(sqlconfig[0]);
        db.setHostName(sqlconfig[1]);
        db.setPort(sqlconfig[2].toInt());
        db.setDatabaseName(sqlconfig[3]);
        db.setUserName(sqlconfig[4]);
        db.setPassword(sqlconfig[5]);
        select_mode = "云模式";
    }

    if (!db.open())
    {
        qDebug() << "数据库连接失败";
        ui->info_label->setStyleSheet("color: red;");
        ui->info_label->setText(select_mode+"：连接失败");
        ui->textEdit->setReadOnly(true);
        ui->listWidget->setEnabled(false);
        ui->textEdit->clear();
        ui->listWidget->clear();
        return false;
    }
    else
    {
        qDebug() << "数据库连接成功";
        ui->info_label->setStyleSheet("color: blue;");
        ui->info_label->setText(select_mode+"：连接正常");
        // 禁止textEdit编辑
        ui->textEdit->setReadOnly(false);

        // 禁用listWidget的交互（使其不可编辑和选择）
        ui->listWidget->setEnabled(true);

        QSqlQuery createTableQuery;
        \
            createTableQuery.prepare("CREATE TABLE IF NOT EXISTS notes("
                                     "date TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                                     "title VARCHAR(255) NOT NULL, "
                                     "note TEXT, "
                                     "id INTEGER, "
                                     "sort_order INTEGER)");

        if(!createTableQuery.exec())
        {
            qDebug() << "创建notes表失败：" << db.lastError().text();return false;
        }
        else
        {
            qDebug() << "初始化表不存在，创建notes表成功" << db.lastError().text();
        }

        QSqlQuery checkEmptyQuery;
        checkEmptyQuery.exec("SELECT COUNT(*) FROM notes");
        checkEmptyQuery.next();
        int rowCount = checkEmptyQuery.value(0).toInt();

        if(rowCount != 0){ // 如果表不为空
            is_lastDelete = false;
            loadTitlesFromDatabase();
            connect(ui->listWidget, &QListWidget::itemChanged, this, &MainWindow::onItemTextChanged_doublecliked);
        }else
        {
            ui->listWidget->clear();
            ui->textEdit->clear();
            is_lastDelete = true;
        }
        return true;
    }
}

// ==================================================================================
// 接收事件，判断事件
// ==================================================================================
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    //右键删除
    if (watched == ui->listWidget && event->type() == QEvent::ContextMenu) {
        QContextMenuEvent *contextEvent = static_cast<QContextMenuEvent*>(event);

        // 获取鼠标点击位置并转换为列表项索引
        QModelIndex index = ui->listWidget->indexAt(contextEvent->pos());

        // 如果索引有效，则弹出菜单
        if (index.isValid()) {

            rightClickMenu->exec(contextEvent->globalPos());

            return true; // 消耗事件
        }
    }

    //笔记输入框获得焦点后，实时保存输入内容
     if (watched == ui->textEdit) {
        if (event->type() == QEvent::FocusIn) {
            isTextEditFocused = true;
            // 当textEdit获得焦点时，建立textChanged的信号槽连接
            connect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextEditContentChanged);
        }
        else if (event->type() == QEvent::FocusOut)
        {
            isTextEditFocused = false;
            // 当textEdit失去焦点时，断开textChanged的信号槽连接
            disconnect(ui->textEdit, &QTextEdit::textChanged, this, &MainWindow::onTextEditContentChanged);
        }
        else if (event->type() & Qt::ControlModifier)
        {
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

// ==================================================================================
// 数据库配置界面相关功能实现
// ==================================================================================

/*
 * @brief MainWindow::onActionTriggered
 * 显示数据库配置窗口，允许用户修改数据库连接参数。
 */
void MainWindow::onActionTriggered()
{
    w_sql->setWindowModality(Qt::ApplicationModal);

    w_sql->show();
}
/*
 * @brief MainWindow::onModelChangeTriggered
 * 显示数据库模式选择窗口，允许用户在本地模式（QSQLITE）与云模式（QMYSQL）之间切换。
 */
void MainWindow::onModelChangeTriggered()
{
    w_mode_c->setWindowModality(Qt::ApplicationModal);

    w_mode_c->show();
}

/*
 * @brief MainWindow::handleDataFromFormsql
 * 当用户在数据库配置界面点击“确定修改”按钮后，此函数会被触发。
 */

void MainWindow::handleDataFromFormsql()
{

    bool flag =  setupDatabase(w_sql->config);  // 调用setupDatabase函数尝试根据用户在配置界面输入的新配置重新设置数据库连接

    if(flag)
    {    
        loadTitlesFromDatabase();  // 则重新从数据库加载所有的笔记标题到列表中
        ui->statusbar->showMessage("数据库加载成功");
    }else{
        QMessageBox::warning(this, "警告", "数据库连接失败：");  // 弹出警告框通知用户“查询数据失败”
    }
}

// ==================================================================================
// 从数据库加载标题到QListWidget，同时显示note
// ==================================================================================
void MainWindow::loadTitlesFromDatabase()
{
    ui->textEdit->clear();
    ui->listWidget->clear();
    QSqlQuery query;

    // 查询notes表中的所有记录，包括title, note和sort_order，按sort_order排序
    query.prepare("SELECT id, title, note, sort_order FROM notes ORDER BY sort_order DESC");

    if(query.exec()){
        while(query.next()){

            int idOrder = query.value(0).toInt(); // 获取id
            QString title = query.value(1).toString(); // 获取title
            QString note = query.value(2).toString(); // 获取note
            int sortOrder = query.value(3).toInt(); // 获取sort_order序号

            // 创建新的列表项并设置相关信息
            QListWidgetItem *item = new QListWidgetItem(title, ui->listWidget);
            item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            item->setData(Qt::UserRole, QVariant(sortOrder)); // 存储sort_order
            item->setData(Qt::UserRole+1, QVariant(idOrder)); // 存储id

            if(sortOrder>0)
            {
                item->setBackground(highlightedBackgroundColor);
            }else
            {
                item->setBackground(highlightedBackgroundColor2);
            }
            ui->listWidget->addItem(item);

            ui->textEdit->setText(note);
        }
    }
    else{
        QMessageBox::warning(this, "警告", "查询数据失败：" + query.lastError().text());
    }
}

// ==================================================================================
// updateNoteContent函数：当QListWidget中的项目被点击时，此函数被调用来更新右侧的文本编辑区内容。
// ==================================================================================
void MainWindow::updateNoteContent()
{

    int selectedId = ui->listWidget->currentItem()->data(Qt::UserRole+1).toInt();  // 获取当前选中列表项的笔记ID
    QString noteContent;  // 初始化一个字符串变量，用于存储即将从数据库加载的笔记内容
    QSqlQuery query;  // 创建一个新的SQL查询对象，准备执行查询语句
    query.prepare("SELECT note FROM notes WHERE id = :id");  // 准备SQL命令，根据ID选取notes表中的note列
    query.bindValue(":id", selectedId);  // 绑定参数:id为之前获取的selectedId

    if(query.exec() && query.next())  // 执行查询，如果成功并且有下一条数据（即查询到结果）
    {
        noteContent = query.value(0).toString();  // 从查询结果中取出第一列的值（即note内容），转换为字符串赋给noteContent
    }
    ui->textEdit->setText(noteContent);   // 将从数据库获取的noteContent设置到textEdit中显示
}

// ======================================================================================
// onItemTextChanged_doublecliked函数：双击修改标题后响应函数，用于更新列表项文本及对应数据库中的标题
// ======================================================================================
void MainWindow::onItemTextChanged_doublecliked(QListWidgetItem *item)
{
    if (!item) return; // 预先检查item是否有效，无效则直接返回，避免后续错误

    // 确认此次标题修改是由双击触发的
    if (isEditingDueToDoubleClick)
    {
        QString newTitle = item->text();  // 获取用户输入的新标题文本
        int itemId = item->data(Qt::UserRole+1).toInt(); // 获取当前item关联的笔记ID
        QSqlQuery updateQuery;   // 准备SQL更新语句，设定新的标题
        updateQuery.prepare("UPDATE notes SET title = :newTitle WHERE id = :itemId");
        updateQuery.bindValue(":newTitle", newTitle);  // 绑定参数
        updateQuery.bindValue(":itemId", itemId);

        if(updateQuery.exec())   // 执行更新操作
        {
            qDebug()<<"标题更新至数据库成功";
        } else
        {
            qDebug()<<"标题更新至数据库失败:"<< updateQuery.lastError().text();
        }

        // 更新操作完成后，断开itemChanged信号与当前槽函数的连接，避免非预期的多次执行
        disconnect(ui->listWidget, &QListWidget::itemChanged, this, &MainWindow::onItemTextChanged_doublecliked);

        isEditingDueToDoubleClick = false; // 重置双击编辑状态标志
    }
}

// ==================================================================================
// onTextEditContentChanged槽函数：响应QTextEdit中的内容变化，同步更新数据库中的笔记内容
// ==================================================================================
void MainWindow::onTextEditContentChanged()
{
    // 若当前未处于最后一个删除item
    if(!is_lastDelete)
    {

        QString newText = ui->textEdit->toPlainText();          // 获取QTextEdit当前的文本内容

        // 确保当前有选中项后再进行操作
        QListWidgetItem* currentItem = ui->listWidget->currentItem();
        if(currentItem)
        {

            int currentId = ui->listWidget->currentItem()->data(Qt::UserRole+1).toInt();  // 获取当前选中项目的sort_order

            // 在这里执行数据库更新操作
            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE notes SET note = :newContent WHERE id = :id");
            updateQuery.bindValue(":newContent", newText); // 绑定新的文本内容

            updateQuery.bindValue(":id", currentId); // 基于当前选中标题更新对应的笔记内容

            if(updateQuery.exec()){
                qDebug()<<"文本内容更新至数据库成功";
            } else {
                qDebug()<<"文本内容更新至数据库失败:"<< updateQuery.lastError().text();
            }
        }
    }
}


// ==================================================================================
// on_inset_note_clicked函数：添加按钮，新增笔记
// ==================================================================================
void MainWindow::on_inset_note_clicked()
{
    // 创建并初始化新项，允许编辑
    QListWidgetItem * newItem = new QListWidgetItem("点击编辑标题", ui->listWidget);

    newItem->setFlags( Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    // 清空textEdit的内容
    ui->textEdit->clear();

    ui->listWidget->setCurrentItem(newItem);

    is_newAdd= true;

    is_lastDelete = false;

    QSqlQuery maxSortOrderQuery;
    maxSortOrderQuery.prepare("SELECT MAX(id) FROM notes");
    if(maxSortOrderQuery.exec() && maxSortOrderQuery.next())
    {
        int newSortOrder = 0; // 新笔记的sort_order为当前最大值加1
        int newIdOrder = maxSortOrderQuery.value(0).toInt() + 1; // 新笔记的sort_order为当前最大值加1

        newItem->setData(Qt::UserRole, QVariant(newSortOrder));
        newItem->setData(Qt::UserRole+1, QVariant(newIdOrder));

        QSqlQuery insertQuery;
        insertQuery.prepare("INSERT INTO notes (title, note, sort_order,id) VALUES (:title, :note, :sortOrder,:id)");
        insertQuery.bindValue(":title", "点击编辑标题");
        insertQuery.bindValue(":note", "");
        insertQuery.bindValue(":sortOrder", newSortOrder);
        insertQuery.bindValue(":id", newIdOrder);

        if(insertQuery.exec()){
            qDebug()<<"已成功插入默认记录";
            ui->statusbar->showMessage("新增笔记！");
        } else {
            qDebug()<<"插入默认记录失败:"<< insertQuery.lastError().text();
        }
    }
    else
    {
        qDebug()<<"查询最大sort_order失败:"<< maxSortOrderQuery.lastError().text();
    }
}


// ==================================================================================
// deleteSelectedItem函数：右键删除item
// ==================================================================================
void MainWindow::deleteSelectedItem()
{
    QListWidgetItem *currentItem = ui->listWidget->currentItem();
    int currentRow = ui->listWidget->row(currentItem);
    if (currentItem)
    {
        int idToDelete = currentItem->data(Qt::UserRole+1).toInt();

        QSqlQuery deleteQuery;
        deleteQuery.prepare("DELETE FROM notes WHERE id = :id");
        deleteQuery.bindValue(":id", idToDelete);

        if (deleteQuery.exec())
        {

            ui->listWidget->takeItem(currentRow);
            delete currentItem;

            // QMessageBox::information(this, "成功", "笔记已成功删除！");
            ui->statusbar->showMessage("笔记已成功删除！");
            if(ui->listWidget->count()==0)
            {
                is_lastDelete = true;
                ui->textEdit->clear();
                return;
            }

            updateNoteContent();
        }
        else
        {
            QMessageBox::critical(this, "错误", "删除笔记失败：" + deleteQuery.lastError().text());
        }
    }
}

// ==================================================================================
// ask_check函数：心跳检查
// ==================================================================================
void MainWindow::ask_check()
{
    // 使用成员变量或局部静态变量来存储定时器，确保其生命周期足够长
    QTimer* heartbeatTimer = new QTimer(this); // 注意使用this指针作为父对象，以便于管理定时器的生命周期

    connect(heartbeatTimer, &QTimer::timeout, this, [this]() {
        QSqlQuery query(db);
        if (query.exec("SELECT 1")) {
            if (query.next()) {
                ui->info_label->setStyleSheet("color: blue;");
                qDebug() << "心跳检测 - 数据库连接正常";
            }
        } else {
            ui->info_label->setStyleSheet("color: red;");
            qDebug() << "心跳检测失败：" << query.lastError().text();
        }
    });
    heartbeatTimer->start(60000); // 每60秒执行一次心跳检测
}


// ==================================================================================
// exportToCSV函数：导出txt
// ==================================================================================
void MainWindow::exportToCSV()
{
    qDebug()<<"开始导出TXT";
    QStringList data;
    QSqlQuery query(db);
    QString fileData;
    int count = 1;
    query.exec("SELECT date, title, note FROM notes");

    while(query.next())
    {
        QString index = QString::number(count++);
        QString line = QString("%1\t%2\t%3\t%4")
                           .arg(index)
                           .arg(query.value("date").toString().replace("\n", " "))
                           .arg(query.value("title").toString().replace("\n", " "))
                           .arg(query.value("note").toString().replace("\n", " "));
        data << line;
    }

    // 拼接所有行数据，每行之间用换行符分隔
    fileData = data.join("\n");

    // 询问用户保存文件路径
    QString path = QFileDialog::getSaveFileName(nullptr, tr("导出数据为TXT"), "/", tr("TXT File (*.txt)"));
    if(path.isEmpty())
        return;

    QFile outFile(path);
    if(outFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream streamFileOut(&outFile);
        streamFileOut << fileData;
        streamFileOut.flush();
        outFile.close();
    }
    else
    {
        QMessageBox::critical(nullptr, tr("错误"), tr("无法打开文件进行写入"));
        return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}


// ==============================================
//                     置顶部分
// toggleTopItemAt函数：判断是置顶还是取消置顶
// onTopItemAt函数：置顶实现
// onCancelTopItemAt函数：取消置顶实现
// ==============================================
void MainWindow::toggleTopItemAt(QListWidgetItem *item)
{
    bool isTopped = item->data(Qt::UserRole).toBool();
    qDebug()<<item->data(Qt::UserRole).toInt()<<isTopped;
    if (isTopped) {
        onCancelTopItemAt(item);
    } else {
        onTopItemAt(item);
    }
}

// 置item置顶的槽函数
void MainWindow::onTopItemAt(QListWidgetItem *currentItem)
{
    // QListWidgetItem *currentItem = ui->listWidget->currentItem();
    if (!currentItem) return;

    // 移除当前item并记住它的信息
    QString title = currentItem->text();
    QString note = ui->textEdit->toPlainText();
    ui->listWidget->takeItem(ui->listWidget->row(currentItem));

    // 获取当前最大sort_order以确定新的置顶位置
    QSqlQuery maxSortOrderQuery;
    maxSortOrderQuery.prepare("SELECT MAX(sort_order) FROM notes");
    if(maxSortOrderQuery.exec() && maxSortOrderQuery.next())
    {
        int newSortOrder = maxSortOrderQuery.value(0).toInt() + 1;

        // 更新当前item的数据库记录，将其置顶
        QSqlQuery updateCurrent;
        updateCurrent.prepare("UPDATE notes SET sort_order = :newOrder WHERE id = :id");
        updateCurrent.bindValue(":newOrder", newSortOrder);
        updateCurrent.bindValue(":id", currentItem->data(Qt::UserRole+1).toInt()); // 假设id存储在UserRole+1
        updateCurrent.exec();

        // 重新插入item到列表顶部，并更新其sort_order
        currentItem->setData(Qt::UserRole, QVariant(newSortOrder));
        ui->listWidget->insertItem(0, currentItem);
        currentItem->setText(title); // 重新设置文本，以防万一
        ui->textEdit->setText(note); // 更新textEdit内容

        qDebug()<<"置顶操作成功";
        ui->statusbar->showMessage("置顶成功");

        // 置顶操作成功后，设置当前item的背景色
        currentItem->setBackground(highlightedBackgroundColor);
    }
    else
    {
        qDebug()<<"查询最大sort_order失败:"<< maxSortOrderQuery.lastError().text();
    }
}

void MainWindow::onCancelTopItemAt(QListWidgetItem *item)
{
    if (!item) return;

    // 直接将sort_order设为0表示取消置顶
    QSqlQuery updateCurrent;
    updateCurrent.prepare("UPDATE notes SET sort_order = 0 WHERE id = :id");
    updateCurrent.bindValue(":id", item->data(Qt::UserRole+1).toInt());
    updateCurrent.exec();

    item->setData(Qt::UserRole, QVariant(0)); // 更新item的UserRole数据


    qDebug()<<"取消置顶操作成功";
    ui->statusbar->showMessage("取消置顶");
    loadTitlesFromDatabase();
    showNoteForItem(ui->listWidget->item(0));  // 首项显示
}

// ==============================================
//                   工具栏-帮助
// ==============================================
void MainWindow::showInfoMessage()
{
    // QMessageBox::information(this, tr("About MyNote"), tr("MyNote v2.0\nE-mail:yeshixin@qq.com\nCopyright Reserved"));

    QString aboutMessage =
        "<html><head/><body>"
        "<center>"
        "<h2>MyNote v2.0</h2>"
        "<p>Email: <a href='mailto:yeshixin@qq.com'>yeshixin@qq.com</a></p>"
        "<p>Copyright © 2024. All Rights Reserved.</p>"
        "</center>"
        "</body></html>";

    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("关于 MyNote"));
    msgBox.setText(aboutMessage);
    // 正确的方式加载并设置图标
    QPixmap iconPixmap(":/ico/myico.ico");
    msgBox.setIconPixmap(iconPixmap);
    // msgBox.setStandardButtons(QMessageBox::Ok); // 添加一个"确定"按钮，根据需要可以调整
    msgBox.exec();
}

void MainWindow::showHelp()
{
    QString message =
        "<html><head/><body>"
        "<h2>使用说明</h2>"
        "<h2><center>数据库配置</center></h2>"
        "<ul>"
        "<li><strong>本地模式：</strong>软件默认采用SQLite数据库，数据存储于本地文件。无需特别配置，适合快速使用。转到软件的“配置” &raquo; “模式设置”&raquo;“本地模式”。</li>"
        "<li><strong>云模式：</strong>如果您希望跨设备同步笔记，可以选择MySQL等远程数据库。<ol>"
        "<li>转到软件的“配置” &raquo; “模式设置”&raquo;“云配置设置”。</li>"
        "<li>对于<strong>云数据库</strong>，请输入以下信息：<ul>"
        "<li><em>IP地址：</em>数据库服务器的IP地址。</li>"
        "<li><em>端口：</em>数据库监听的端口号，MySQL默认为3306。</li>"
        "<li><em>数据库名称：</em>您要连接的数据库名称。</li>"
        "<li><em>用户名：</em>数据库的登录用户名。</li>"
        "<li><em>密码：</em>对应的登录密码。</li></ul></li>"
        "<li>确认无误后，点击“确定修改”完成配置。配置成功后，软件将自动连接到指定的云数据库。</li></ol></li></ul>"
        "<h2><center>新增与管理笔记</center></h2>"
        "<ul>"
        "<li><strong>新增笔记：</strong>点击工具栏上的“新增笔记”按钮实现新笔记添加。</li>"
        "<li><strong>编辑笔记：</strong>双击左侧列表中的笔记标题即可进行标题编辑，在右侧文本框内可进行内容编辑。编辑完成后，内容会实时同步至数据库。</li>"
        "<li><strong>笔记排序：</strong>在“置顶/取消置顶”功能的帮助下，您可以轻松调整笔记的显示顺序。</li>"
        "<li><strong>导出笔记：</strong>通过“文件” &raquo; “导出笔记”，将笔记以txt的形式导出。</li>"
        "<li><strong>字体设置：</strong>点击工具栏上的“配置” &raquo; “字体大小”，实现字体大小修改。</li></ul>"
        "</body></html>";

    QMessageBox msgBox;
    msgBox.setWindowTitle(tr("使用说明"));
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::NoIcon); // 隐藏图标
    // msgBox.setStandardButtons(QMessageBox::Ok); // 添加一个"确定"按钮，根据需要可以调整
    msgBox.exec();
}
