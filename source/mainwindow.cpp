#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>
#include <QCloseEvent>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , libraryManager(new LibraryManager(this))
    , currentFileName("")
{
    ui->setupUi(this);
    setupTables();

    // 连接数据变化信号
    connect(libraryManager, &LibraryManager::dataChanged,
            this, &MainWindow::updateBooksTable);
    connect(libraryManager, &LibraryManager::dataChanged,
            this, &MainWindow::updateReadersTable);
    connect(libraryManager, &LibraryManager::dataChanged,
            this, &MainWindow::updateStatistics);

    // 连接时间变化信号
    connect(libraryManager, &LibraryManager::currentDateChanged,
            this, &MainWindow::updateTimeDisplay);

    // 连接清空表单按钮
    connect(ui->clearBookButton, &QPushButton::clicked,
            this, &MainWindow::clearBookForm);
    connect(ui->clearReaderButton, &QPushButton::clicked,
            this, &MainWindow::clearReaderForm);
    connect(ui->clearReaderSearchButton, &QPushButton::clicked, [this]() {
        ui->searchReaderEdit->clear();
        updateReadersTable();
    });

    // 初始化界面
    updateBooksTable();
    updateReadersTable();
    updateStatistics();
    updateTimeDisplay();

    // 设置窗口标题
    setWindowTitle("图书借阅管理系统");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 询问是否保存当前文件
    if (!currentFileName.isEmpty() && currentFileName != "未命名") {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "退出",
                                                                  "是否保存当前文件？",
                                                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (reply == QMessageBox::Yes) {
            if (!libraryManager->saveToFile(currentFileName)) {
                QMessageBox::critical(this, "错误", "保存失败！");
                event->ignore();
                return;
            }
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    event->accept();
}

void MainWindow::saveWindowState()
{
    // 保存窗口大小和位置
    libraryManager->saveSettings();
}

void MainWindow::restoreWindowState()
{
    // 恢复窗口状态
    // 这里可以添加窗口状态恢复代码
}

void MainWindow::setupTables()
{
    // 图书表格
    QStringList bookHeaders = {"图书编号", "书名", "作者", "类别",
                               "总册数", "可借册数", "状态"};
    ui->booksTable->setColumnCount(bookHeaders.size());
    ui->booksTable->setHorizontalHeaderLabels(bookHeaders);
    ui->booksTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->booksTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->booksTable->horizontalHeader()->setStretchLastSection(true);
    ui->booksTable->setSortingEnabled(true);

    // 读者表格
    QStringList readerHeaders = {"读者编号", "姓名", "院系", "电话", "注册日期"};
    ui->readersTable->setColumnCount(readerHeaders.size());
    ui->readersTable->setHorizontalHeaderLabels(readerHeaders);
    ui->readersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->readersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->readersTable->horizontalHeader()->setStretchLastSection(true);
    ui->readersTable->setSortingEnabled(true);
}

void MainWindow::updateBooksTable()
{
    ui->booksTable->setRowCount(0);

    QVector<Book*> books = libraryManager->getAllBooks();
    for (int i = 0; i < books.size(); ++i) {
        Book *book = books[i];
        ui->booksTable->insertRow(i);

        ui->booksTable->setItem(i, 0, new QTableWidgetItem(book->getId()));
        ui->booksTable->setItem(i, 1, new QTableWidgetItem(book->getTitle()));
        ui->booksTable->setItem(i, 2, new QTableWidgetItem(book->getAuthor()));
        ui->booksTable->setItem(i, 3, new QTableWidgetItem(book->getCategoryString()));
        ui->booksTable->setItem(i, 4, new QTableWidgetItem(QString::number(book->getTotalCopies())));
        ui->booksTable->setItem(i, 5, new QTableWidgetItem(QString::number(book->getAvailableCopies())));
        ui->booksTable->setItem(i, 6, new QTableWidgetItem(book->getStatusString()));
    }
}

void MainWindow::updateReadersTable()
{
    ui->readersTable->setRowCount(0);

    QVector<Reader*> readers = libraryManager->getAllReaders();
    for (int i = 0; i < readers.size(); ++i) {
        Reader *reader = readers[i];
        ui->readersTable->insertRow(i);

        ui->readersTable->setItem(i, 0, new QTableWidgetItem(reader->getId()));
        ui->readersTable->setItem(i, 1, new QTableWidgetItem(reader->getName()));
        ui->readersTable->setItem(i, 2, new QTableWidgetItem(reader->getDept()));
        ui->readersTable->setItem(i, 3, new QTableWidgetItem(reader->getPhone()));
        ui->readersTable->setItem(i, 4, new QTableWidgetItem(reader->getRegisterDate().toString("yyyy-MM-dd")));
    }
}

void MainWindow::updateStatistics()
{
    ui->totalBooksLabel->setText(QString::number(libraryManager->getTotalBookCount()));
    ui->availableBooksLabel->setText(QString::number(libraryManager->getAvailableBookCount()));
    ui->borrowedBooksLabel->setText(QString::number(libraryManager->getBorrowedBookCount()));

    // 计算图书种类数
    QMap<BookCategory, int> stats = libraryManager->getCategoryStatistics();
    int categoryCount = 0;
    for (BookCategory cat = SCIENCE; cat <= OTHER;
         cat = static_cast<BookCategory>(cat + 1)) {
        if (stats[cat] > 0) {
            categoryCount++;
        }
    }
    ui->categoryCountLabel->setText(QString::number(categoryCount));

    // 读者总数
    ui->readerCountLabel->setText(QString::number(libraryManager->getTotalReaderCount()));
}

// 添加时间显示更新函数
void MainWindow::updateTimeDisplay()
{
    QDate currentDate = libraryManager->getCurrentDate();
    bool isCustom = libraryManager->isUsingCustomTime();

    // 更新状态栏
    QString timeText = QString("当前时间: %1").arg(currentDate.toString("yyyy年MM月dd日"));
    if (isCustom) {
        timeText += " [自定义时间]";
    } else {
        timeText += " [实时时间]";
    }
    ui->statusbar->showMessage(timeText);

    // 如果存在currentDateLabel，更新它
    QLabel* currentDateLabel = findChild<QLabel*>("currentDateLabel");
    if (currentDateLabel) {
        currentDateLabel->setText(currentDate.toString("yyyy-MM-dd"));
        if (isCustom) {
            currentDateLabel->setStyleSheet("color: red; font-weight: bold;");
        } else {
            currentDateLabel->setStyleSheet("color: green;");
        }
    }
}

Book MainWindow::getBookFromForm()
{
    Book book;
    book.setId(ui->idEdit->text());
    book.setTitle(ui->titleEdit->text());
    book.setAuthor(ui->authorEdit->text());
    book.setCategory(static_cast<BookCategory>(ui->categoryCombo->currentIndex()));
    book.setTotalCopies(ui->totalCopiesSpin->value());
    book.setAvailableCopies(ui->availableCopiesSpin->value());

    return book;
}

Reader MainWindow::getReaderFromForm()
{
    Reader reader;
    reader.setId(ui->readerIdEdit->text());
    reader.setName(ui->readerNameEdit->text());
    reader.setDept(ui->readerDeptEdit->text());
    reader.setPhone(ui->readerPhoneEdit->text());
    return reader;
}

void MainWindow::setBookToForm(const Book &book)
{
    ui->idEdit->setText(book.getId());
    ui->titleEdit->setText(book.getTitle());
    ui->authorEdit->setText(book.getAuthor());
    ui->categoryCombo->setCurrentIndex(static_cast<int>(book.getCategory()));
    ui->totalCopiesSpin->setValue(book.getTotalCopies());
    ui->availableCopiesSpin->setValue(book.getAvailableCopies());
}

void MainWindow::setReaderToForm(const Reader &reader)
{
    ui->readerIdEdit->setText(reader.getId());
    ui->readerNameEdit->setText(reader.getName());
    ui->readerDeptEdit->setText(reader.getDept());
    ui->readerPhoneEdit->setText(reader.getPhone());
}

void MainWindow::clearBookForm()
{
    ui->idEdit->clear();
    ui->titleEdit->clear();
    ui->authorEdit->clear();
    ui->categoryCombo->setCurrentIndex(0);
    ui->totalCopiesSpin->setValue(1);
    ui->availableCopiesSpin->setValue(1);
}

void MainWindow::clearReaderForm()
{
    ui->readerIdEdit->clear();
    ui->readerNameEdit->clear();
    ui->readerDeptEdit->clear();
    ui->readerPhoneEdit->clear();
}

bool MainWindow::validateBookForm()
{
    if (ui->idEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "图书编号不能为空！");
        return false;
    }
    if (ui->titleEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "书名不能为空！");
        return false;
    }
    if (ui->availableCopiesSpin->value() > ui->totalCopiesSpin->value()) {
        QMessageBox::warning(this, "警告", "可借册数不能大于总册数！");
        return false;
    }
    return true;
}

bool MainWindow::validateReaderForm()
{
    if (ui->readerIdEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "读者编号不能为空！");
        return false;
    }
    if (ui->readerNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, "警告", "读者姓名不能为空！");
        return false;
    }
    return true;
}

void MainWindow::showBookDetails(Book *book)
{
    if (!book) return;

    QString details = QString("图书编号: %1\n"
                              "书名: %2\n"
                              "作者: %3\n"
                              "类别: %4\n"
                              "总册数: %5\n"
                              "可借册数: %6\n"
                              "状态: %7")
                          .arg(book->getId())
                          .arg(book->getTitle())
                          .arg(book->getAuthor())
                          .arg(book->getCategoryString())
                          .arg(book->getTotalCopies())
                          .arg(book->getAvailableCopies())
                          .arg(book->getStatusString());

    QMessageBox::information(this, "图书详情", details);
}

void MainWindow::showReaderDetails(Reader *reader)
{
    if (!reader) return;

    QString details = QString("读者编号: %1\n"
                              "姓名: %2\n"
                              "院系: %3\n"
                              "电话: %4\n"
                              "注册日期: %5\n"
                              "状态: %6")
                          .arg(reader->getId())
                          .arg(reader->getName())
                          .arg(reader->getDept())
                          .arg(reader->getPhone())
                          .arg(reader->getRegisterDate().toString("yyyy-MM-dd"))
                          .arg(reader->getIsValid() ? "有效" : "无效");

    QMessageBox::information(this, "读者详情", details);
}

// ============== 文件操作槽函数 ==============

void MainWindow::on_actionNew_triggered()
{
    // 先询问是否保存当前文件
    if (!currentFileName.isEmpty() && currentFileName != "未命名") {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "新建",
                                                                  "是否保存当前文件？",
                                                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (reply == QMessageBox::Yes) {
            on_actionSave_triggered();
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    // 创建新的 LibraryManager（空白文件）
    delete libraryManager;
    libraryManager = new LibraryManager(this);
    libraryManager->clearAllData();  // 清空所有数据

    currentFileName = "未命名";

    // 更新界面
    updateBooksTable();
    updateReadersTable();
    updateStatistics();
    updateTimeDisplay();
    clearBookForm();
    clearReaderForm();

    // 清空搜索条件
    ui->searchEdit->clear();
    ui->searchCategoryCombo->setCurrentIndex(0);
    ui->searchByTitleCheck->setChecked(true);
    ui->searchByAuthorCheck->setChecked(true);
    ui->searchReaderEdit->clear();

    setWindowTitle("图书借阅管理系统 - 未命名");
    QMessageBox::information(this, "新建", "已创建新文件！");
}

void MainWindow::on_actionOpen_triggered()
{
    // 先询问是否保存当前文件
    if (!currentFileName.isEmpty() && currentFileName != "未命名") {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "打开",
                                                                  "是否保存当前文件？",
                                                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (reply == QMessageBox::Yes) {
            on_actionSave_triggered();
        } else if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    QString fileName = QFileDialog::getOpenFileName(this, "打开文件",
                                                    ".", "图书馆数据文件 (*.lib);;所有文件 (*.*)");
    if (!fileName.isEmpty()) {
        // 创建新的管理器
        LibraryManager* newManager = new LibraryManager(this);

        // 尝试加载文件
        if (newManager->loadFromFile(fileName)) {
            // 删除旧的管理器
            delete libraryManager;
            libraryManager = newManager;

            currentFileName = fileName;
            setWindowTitle(QString("图书借阅管理系统 - %1").arg(QFileInfo(fileName).fileName()));

            // 更新界面
            updateBooksTable();
            updateReadersTable();
            updateStatistics();
            updateTimeDisplay();

            QMessageBox::information(this, "成功", "文件加载成功！");
        } else {
            delete newManager;
            QMessageBox::critical(this, "错误", "无法打开文件！");
        }
    }
}

void MainWindow::on_actionSave_triggered()
{
    if (currentFileName.isEmpty() || currentFileName == "未命名") {
        on_actionSaveAs_triggered();
    } else {
        if (libraryManager->saveToFile(currentFileName)) {
            QMessageBox::information(this, "成功", QString("文件已保存到：%1").arg(currentFileName));
        } else {
            QMessageBox::critical(this, "错误", "保存失败！");
        }
    }
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "另存为",
                                                    "library_data.lib", "图书馆数据文件 (*.lib)");
    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".lib", Qt::CaseInsensitive)) {
            fileName += ".lib";
        }

        if (libraryManager->saveToFile(fileName)) {
            currentFileName = fileName;
            setWindowTitle(QString("图书借阅管理系统 - %1").arg(QFileInfo(fileName).fileName()));
            QMessageBox::information(this, "成功", QString("文件已另存为：%1").arg(fileName));
        } else {
            QMessageBox::critical(this, "错误", "保存失败！");
        }
    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "关于",
                       "图书借阅信息管理系统\n\n"
                       "版本：3.0\n\n"
                       "开发：丁文龙，李星翰，罗明昊，谢庆涛\n");
}

// ============== 图书管理槽函数 ==============

void MainWindow::on_addBookButton_clicked()
{
    if (!validateBookForm()) return;

    Book book = getBookFromForm();
    if (libraryManager->addBook(book)) {
        QMessageBox::information(this, "成功", "图书添加成功！");
        clearBookForm();
    } else {
        QMessageBox::warning(this, "警告", "图书编号已存在！");
    }
}

void MainWindow::on_editBookButton_clicked()
{
    if (!validateBookForm()) return;

    Book book = getBookFromForm();
    if (libraryManager->updateBook(book)) {
        QMessageBox::information(this, "成功", "图书更新成功！");
        clearBookForm();
    } else {
        QMessageBox::warning(this, "警告", "图书不存在！");
    }
}

void MainWindow::on_deleteBookButton_clicked()
{
    QString id = ui->idEdit->text();
    if (id.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入要删除的图书编号！");
        return;
    }

    if (QMessageBox::question(this, "确认", "确定要删除这本图书吗？")
        == QMessageBox::Yes) {
        if (libraryManager->removeBook(id)) {
            QMessageBox::information(this, "成功", "图书删除成功！");
            clearBookForm();
        } else {
            QMessageBox::warning(this, "警告", "图书不存在！");
        }
    }
}

void MainWindow::on_searchButton_clicked()
{
    QString keyword = ui->searchEdit->text();
    int categoryIndex = ui->searchCategoryCombo->currentIndex();
    BookCategory category = OTHER; // 默认所有类别

    if (categoryIndex > 0) {
        category = static_cast<BookCategory>(categoryIndex - 1);
    }

    bool searchByTitle = ui->searchByTitleCheck->isChecked();
    bool searchByAuthor = ui->searchByAuthorCheck->isChecked();

    // 如果两个复选框都没选中，默认都搜索
    if (!searchByTitle && !searchByAuthor) {
        searchByTitle = true;
        searchByAuthor = true;
    }

    QVector<Book*> results = libraryManager->searchBooks(keyword, category,
                                                          searchByTitle, searchByAuthor);

    ui->booksTable->setRowCount(0);
    for (int i = 0; i < results.size(); ++i) {
        Book *book = results[i];
        ui->booksTable->insertRow(i);

        ui->booksTable->setItem(i, 0, new QTableWidgetItem(book->getId()));
        ui->booksTable->setItem(i, 1, new QTableWidgetItem(book->getTitle()));
        ui->booksTable->setItem(i, 2, new QTableWidgetItem(book->getAuthor()));
        ui->booksTable->setItem(i, 3, new QTableWidgetItem(book->getCategoryString()));
        ui->booksTable->setItem(i, 4, new QTableWidgetItem(QString::number(book->getTotalCopies())));
        ui->booksTable->setItem(i, 5, new QTableWidgetItem(QString::number(book->getAvailableCopies())));
        ui->booksTable->setItem(i, 6, new QTableWidgetItem(book->getStatusString()));
    }

    // 显示搜索结果统计
    if (!keyword.isEmpty() || categoryIndex > 0) {
        ui->statusbar->showMessage(QString("找到 %1 本符合条件的图书").arg(results.size()), 3000);
    }
}

void MainWindow::on_clearSearchButton_clicked()
{
    ui->searchEdit->clear();
    ui->searchCategoryCombo->setCurrentIndex(0);
    ui->searchByTitleCheck->setChecked(true);
    ui->searchByAuthorCheck->setChecked(true);
    updateBooksTable();
    ui->statusbar->showMessage("已清空搜索条件", 2000);
}

void MainWindow::on_booksTable_itemDoubleClicked(QTableWidgetItem *item)
{
    int row = item->row();
    QString bookId = ui->booksTable->item(row, 0)->text();
    Book *book = libraryManager->findBook(bookId);

    if (book) {
        setBookToForm(*book);
        showBookDetails(book);
    }
}

// ============== 读者管理槽函数 ==============

void MainWindow::on_addReaderButton_clicked()
{
    if (!validateReaderForm()) return;

    Reader reader = getReaderFromForm();
    if (libraryManager->addReader(reader)) {
        QMessageBox::information(this, "成功", "读者添加成功！");
        clearReaderForm();
    } else {
        QMessageBox::warning(this, "警告", "读者编号已存在！");
    }
}

void MainWindow::on_editReaderButton_clicked()
{
    if (!validateReaderForm()) return;

    Reader reader = getReaderFromForm();
    if (libraryManager->updateReader(reader)) {
        QMessageBox::information(this, "成功", "读者更新成功！");
        clearReaderForm();
    } else {
        QMessageBox::warning(this, "警告", "读者不存在！");
    }
}

void MainWindow::on_deleteReaderButton_clicked()
{
    QString id = ui->readerIdEdit->text();
    if (id.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入要删除的读者编号！");
        return;
    }

    if (QMessageBox::question(this, "确认", "确定要删除这位读者吗？")
        == QMessageBox::Yes) {
        if (libraryManager->removeReader(id)) {
            QMessageBox::information(this, "成功", "读者删除成功！");
            clearReaderForm();
        } else {
            QMessageBox::warning(this, "警告", "读者不存在！");
        }
    }
}

void MainWindow::on_searchReaderButton_clicked()
{
    QString keyword = ui->searchReaderEdit->text();
    QVector<Reader*> results = libraryManager->searchReaders(keyword);

    ui->readersTable->setRowCount(0);
    for (int i = 0; i < results.size(); ++i) {
        Reader *reader = results[i];
        ui->readersTable->insertRow(i);

        ui->readersTable->setItem(i, 0, new QTableWidgetItem(reader->getId()));
        ui->readersTable->setItem(i, 1, new QTableWidgetItem(reader->getName()));
        ui->readersTable->setItem(i, 2, new QTableWidgetItem(reader->getDept()));
        ui->readersTable->setItem(i, 3, new QTableWidgetItem(reader->getPhone()));
        ui->readersTable->setItem(i, 4, new QTableWidgetItem(reader->getRegisterDate().toString("yyyy-MM-dd")));
    }

    // 显示搜索结果统计
    if (!keyword.isEmpty()) {
        ui->statusbar->showMessage(QString("找到 %1 位符合条件的读者").arg(results.size()), 3000);
    }
}

void MainWindow::on_clearReaderSearchButton_clicked()
{
    ui->searchReaderEdit->clear();
    updateReadersTable();
    ui->statusbar->showMessage("已清空搜索条件", 2000);
}

void MainWindow::on_readersTable_itemDoubleClicked(QTableWidgetItem *item)
{
    int row = item->row();
    QString readerId = ui->readersTable->item(row, 0)->text();
    Reader *reader = libraryManager->findReader(readerId);

    if (reader) {
        setReaderToForm(*reader);
        showReaderDetails(reader);
    }
}

void MainWindow::on_showBookHistoryButton_clicked()
{
    QString bookId = ui->idEdit->text();
    if (bookId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择或输入图书！");
        return;
    }

    QVector<BorrowRecord> records = libraryManager->getBorrowRecordsByBook(bookId);
    QVector<QString> reservators = libraryManager->getReservatorsByBook(bookId);
    Book *book = libraryManager->findBook(bookId);

    if (!book) {
        QMessageBox::warning(this, "警告", "图书不存在！");
        return;
    }

    QString history = QString("图书【%1 - %2】的历史记录：\n\n")
                          .arg(bookId)
                          .arg(book->getTitle());

    if (records.isEmpty() && reservators.isEmpty()) {
        history += "暂无历史记录。";
    }

    if (!records.isEmpty()) {
        history += "借阅记录：\n";
        for (int i = 0; i < records.size(); ++i) {
            const BorrowRecord &record = records[i];
            Reader *reader = libraryManager->findReader(record.getReaderId());
            QString readerName = reader ? reader->getName() : "未知读者";

            history += QString("%1. 读者: %2 (%3), 借阅日期: %4, 应还日期: %5, %6\n")
                           .arg(i + 1)
                           .arg(record.getReaderId())
                           .arg(readerName)
                           .arg(record.getBorrowDate().toString("yyyy-MM-dd"))
                           .arg(record.getDueDate().toString("yyyy-MM-dd"))
                           .arg(record.isReturned() ?
                                    QString("归还日期: %1").arg(record.getReturnDate().toString("yyyy-MM-dd")) :
                                    "未归还");
        }
    }

    if (!reservators.isEmpty()) {
        history += "\n预定记录：\n";
        for (int i = 0; i < reservators.size(); ++i) {
            Reader *reader = libraryManager->findReader(reservators[i]);
            QString readerName = reader ? reader->getName() : "未知读者";

            history += QString("%1. 读者: %2 (%3)\n")
                           .arg(i + 1)
                           .arg(reservators[i])
                           .arg(readerName);
        }
    }

    QMessageBox::information(this, "图书历史记录", history);
}

void MainWindow::on_showReaderRecordsButton_clicked()
{
    QString readerId = ui->readerIdEdit->text();
    if (readerId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择或输入读者！");
        return;
    }

    QVector<BorrowRecord> records = libraryManager->getBorrowRecordsByReader(readerId);
    Reader *reader = libraryManager->findReader(readerId);

    if (!reader) {
        QMessageBox::warning(this, "警告", "读者不存在！");
        return;
    }

    QString recordInfo = QString("读者【%1 - %2】的借阅记录：\n\n")
                             .arg(readerId)
                             .arg(reader->getName());

    if (records.isEmpty()) {
        recordInfo += "该读者没有借阅记录。";
    } else {
        for (int i = 0; i < records.size(); ++i) {
            const BorrowRecord &record = records[i];
            Book *book = libraryManager->findBook(record.getBookId());
            QString bookTitle = book ? book->getTitle() : "未知图书";

            recordInfo += QString("%1. 图书: %2 (%3), 借阅日期: %4, 应还日期: %5, %6\n")
                              .arg(i + 1)
                              .arg(record.getBookId())
                              .arg(bookTitle)
                              .arg(record.getBorrowDate().toString("yyyy-MM-dd"))
                              .arg(record.getDueDate().toString("yyyy-MM-dd"))
                              .arg(record.isReturned() ?
                                       QString("归还日期: %1").arg(record.getReturnDate().toString("yyyy-MM-dd")) :
                                       "未归还");
        }
    }

    QMessageBox::information(this, "读者借阅记录", recordInfo);
}

// ============== 借阅管理槽函数 ==============

void MainWindow::on_borrowButton_clicked()
{
    QString bookId = ui->idEdit->text();
    if (bookId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择或输入图书！");
        return;
    }

    bool ok;
    QString readerId = QInputDialog::getText(this, "借阅图书",
                                             "请输入读者ID：",
                                             QLineEdit::Normal,
                                             "", &ok);

    if (ok && !readerId.isEmpty()) {
        Reader *reader = libraryManager->findReader(readerId);
        if (!reader) {
            QMessageBox::warning(this, "警告", "读者不存在！");
            return;
        }

        // 询问是否使用自定义借阅日期
        QDate borrowDate = libraryManager->getCurrentDate();
        if (libraryManager->isUsingCustomTime()) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "借阅日期",
                                                                      QString("当前使用自定义时间: %1\n是否使用此日期作为借阅日期？").arg(borrowDate.toString("yyyy-MM-dd")),
                                                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

            if (reply == QMessageBox::No) {
                // 让用户输入自定义日期
                QString dateStr = QInputDialog::getText(this, "自定义借阅日期",
                                                        "请输入借阅日期 (格式: yyyy-MM-dd):",
                                                        QLineEdit::Normal,
                                                        borrowDate.toString("yyyy-MM-dd"),
                                                        &ok);

                if (ok && !dateStr.isEmpty()) {
                    QDate customDate = QDate::fromString(dateStr, "yyyy-MM-dd");
                    if (customDate.isValid()) {
                        borrowDate = customDate;
                    } else {
                        QMessageBox::warning(this, "错误", "日期格式不正确，请使用 yyyy-MM-dd 格式");
                        return;
                    }
                } else if (ok) {
                    QMessageBox::warning(this, "错误", "请输入日期");
                    return;
                } else {
                    return; // 用户取消
                }
            } else if (reply == QMessageBox::Cancel) {
                return;
            }
        }

        if (libraryManager->borrowBook(readerId, bookId, borrowDate)) {
            QMessageBox::information(this, "成功",
                                     QString("借阅成功！\n读者：%1\n图书：%2\n借阅日期：%3\n应还日期：%4")
                                         .arg(reader->getName())
                                         .arg(libraryManager->findBook(bookId)->getTitle())
                                         .arg(borrowDate.toString("yyyy-MM-dd"))
                                         .arg(borrowDate.addDays(30).toString("yyyy-MM-dd")));
        } else {
            QMessageBox::warning(this, "警告", "借阅失败！图书可能不可借。");
        }
    }
}

void MainWindow::on_returnButton_clicked()
{
    QString bookId = ui->idEdit->text();
    if (bookId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择或输入图书！");
        return;
    }

    bool ok;
    QString readerId = QInputDialog::getText(this, "归还图书",
                                             "请输入读者ID：",
                                             QLineEdit::Normal,
                                             "", &ok);

    if (ok && !readerId.isEmpty()) {
        // 询问是否使用自定义归还日期
        QDate returnDate = libraryManager->getCurrentDate();
        if (libraryManager->isUsingCustomTime()) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, "归还日期",
                                                                      QString("当前使用自定义时间: %1\n是否使用此日期作为归还日期？").arg(returnDate.toString("yyyy-MM-dd")),
                                                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

            if (reply == QMessageBox::No) {
                // 让用户输入自定义日期
                QString dateStr = QInputDialog::getText(this, "自定义归还日期",
                                                        "请输入归还日期 (格式: yyyy-MM-dd):",
                                                        QLineEdit::Normal,
                                                        returnDate.toString("yyyy-MM-dd"),
                                                        &ok);

                if (ok && !dateStr.isEmpty()) {
                    QDate customDate = QDate::fromString(dateStr, "yyyy-MM-dd");
                    if (customDate.isValid()) {
                        returnDate = customDate;
                    } else {
                        QMessageBox::warning(this, "错误", "日期格式不正确，请使用 yyyy-MM-dd 格式");
                        return;
                    }
                } else if (ok) {
                    QMessageBox::warning(this, "错误", "请输入日期");
                    return;
                } else {
                    return; // 用户取消
                }
            } else if (reply == QMessageBox::Cancel) {
                return;
            }
        }

        if (libraryManager->returnBook(readerId, bookId, returnDate)) {
            QMessageBox::information(this, "成功",
                                     QString("归还成功！\n归还日期：%1").arg(returnDate.toString("yyyy-MM-dd")));
        } else {
            QMessageBox::warning(this, "警告", "归还失败！");
        }
    }
}

void MainWindow::on_reserveButton_clicked()
{
    QString bookId = ui->idEdit->text();
    if (bookId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择或输入图书！");
        return;
    }

    bool ok;
    QString readerId = QInputDialog::getText(this, "预定图书",
                                             "请输入读者ID：",
                                             QLineEdit::Normal,
                                             "", &ok);

    if (ok && !readerId.isEmpty()) {
        Reader *reader = libraryManager->findReader(readerId);
        if (!reader) {
            QMessageBox::warning(this, "警告", "读者不存在！");
            return;
        }

        if (libraryManager->reserveBook(readerId, bookId)) {
            QMessageBox::information(this, "成功",
                                     QString("预定成功！\n读者：%1\n图书：%2")
                                         .arg(reader->getName())
                                         .arg(libraryManager->findBook(bookId)->getTitle()));
        } else {
            QMessageBox::warning(this, "警告", "预定失败！");
        }
    }
}

// ============== 时间管理槽函数 ==============

void MainWindow::on_setCustomDateButton_clicked()
{
    bool ok;
    QString dateStr = QInputDialog::getText(this, "设置自定义日期",
                                            "请输入日期 (格式: yyyy-MM-dd):\n例如: 2024-01-15",
                                            QLineEdit::Normal,
                                            libraryManager->getCurrentDate().toString("yyyy-MM-dd"),
                                            &ok);

    if (ok && !dateStr.isEmpty()) {
        QDate date = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (date.isValid()) {
            libraryManager->setCurrentDate(date);
            QMessageBox::information(this, "成功",
                                     QString("系统时间已设置为: %1").arg(date.toString("yyyy年MM月dd日")));
        } else {
            QMessageBox::warning(this, "错误", "日期格式不正确，请使用 yyyy-MM-dd 格式");
        }
    }
}

void MainWindow::on_resetTimeButton_clicked()
{
    if (QMessageBox::question(this, "确认", "确定要恢复使用系统实时时间吗？") == QMessageBox::Yes) {
        libraryManager->resetToRealTime();
        QMessageBox::information(this, "成功", "已恢复使用系统实时时间");
    }
}

// ============== 统计功能槽函数 ==============

void MainWindow::on_showStatisticsButton_clicked()
{
    QMap<BookCategory, int> stats = libraryManager->getCategoryStatistics();

    QString statistics = "图书分类统计：\n\n";
    int totalBooks = 0;

    for (BookCategory cat = SCIENCE; cat <= OTHER;
         cat = static_cast<BookCategory>(cat + 1)) {
        QString categoryName;
        switch(cat) {
        case SCIENCE: categoryName = "科学"; break;
        case TECHNOLOGY: categoryName = "技术"; break;
        case LITERATURE: categoryName = "文学"; break;
        case HISTORY: categoryName = "历史"; break;
        case ART: categoryName = "艺术"; break;
        default: categoryName = "其他"; break;
        }

        statistics += QString("%1: %2册\n").arg(categoryName).arg(stats[cat]);
        totalBooks += stats[cat];
    }

    statistics += QString("\n总计: %1册\n").arg(totalBooks);
    statistics += QString("读者总数: %1人\n").arg(libraryManager->getTotalReaderCount());
    statistics += QString("当前时间: %1").arg(libraryManager->getCurrentDate().toString("yyyy年MM月dd日"));

    QMessageBox::information(this, "统计信息", statistics);
}

void MainWindow::on_showBorrowRecordsButton_clicked()
{
    bool ok;
    QString readerId = QInputDialog::getText(this, "查询借阅记录",
                                             "请输入读者ID：",
                                             QLineEdit::Normal,
                                             "", &ok);

    if (ok && !readerId.isEmpty()) {
        QVector<BorrowRecord> records = libraryManager->getBorrowRecordsByReader(readerId);
        Reader *reader = libraryManager->findReader(readerId);

        if (!reader) {
            QMessageBox::warning(this, "警告", "读者不存在！");
            return;
        }

        if (records.isEmpty()) {
            QMessageBox::information(this, "借阅记录",
                                     QString("读者【%1 - %2】没有借阅记录。")
                                         .arg(readerId)
                                         .arg(reader->getName()));
            return;
        }

        QString recordInfo = QString("读者【%1 - %2】的借阅记录：\n\n").arg(readerId).arg(reader->getName());

        for (int i = 0; i < records.size(); ++i) {
            const BorrowRecord &record = records[i];
            Book *book = libraryManager->findBook(record.getBookId());
            QString bookTitle = book ? book->getTitle() : "未知图书";

            recordInfo += QString("%1. 图书: %2 (%3), 借阅日期: %4, 应还日期: %5, %6\n")
                              .arg(i + 1)
                              .arg(record.getBookId())
                              .arg(bookTitle)
                              .arg(record.getBorrowDate().toString("yyyy-MM-dd"))
                              .arg(record.getDueDate().toString("yyyy-MM-dd"))
                              .arg(record.isReturned() ?
                                       QString("归还日期: %1").arg(record.getReturnDate().toString("yyyy-MM-dd")) :
                                       "未归还");
        }

        QMessageBox::information(this, "借阅记录", recordInfo);
    }
}

void MainWindow::on_showOverdueButton_clicked()
{
    QVector<BorrowRecord> overdue = libraryManager->getOverdueRecords();

    if (overdue.isEmpty()) {
        QMessageBox::information(this, "逾期记录", "当前没有逾期记录。");
        return;
    }

    QString overdueInfo = "逾期记录：\n\n";
    for (int i = 0; i < overdue.size(); ++i) {
        const BorrowRecord &record = overdue[i];
        Reader *reader = libraryManager->findReader(record.getReaderId());
        Book *book = libraryManager->findBook(record.getBookId());
        QString readerName = reader ? reader->getName() : "未知读者";
        QString bookTitle = book ? book->getTitle() : "未知图书";

        int overdueDays = record.getDueDate().daysTo(libraryManager->getCurrentDate());

        overdueInfo += QString("%1. 读者: %2 (%3), 图书: %4 (%5), \n   应还日期: %6, 逾期天数: %7天\n")
                           .arg(i + 1)
                           .arg(record.getReaderId())
                           .arg(readerName)
                           .arg(record.getBookId())
                           .arg(bookTitle)
                           .arg(record.getDueDate().toString("yyyy-MM-dd"))
                           .arg(overdueDays);
    }

    QMessageBox::warning(this, "逾期记录", overdueInfo);
}

void MainWindow::on_showReservationsButton_clicked()
{
    QVector<QPair<QString, QString>> reservations = libraryManager->getReservations();

    if (reservations.isEmpty()) {
        QMessageBox::information(this, "预定记录", "当前没有预定记录。");
        return;
    }

    QString reservationInfo = "预定记录：\n\n";
    for (int i = 0; i < reservations.size(); ++i) {
        Reader *reader = libraryManager->findReader(reservations[i].first);
        Book *book = libraryManager->findBook(reservations[i].second);
        QString readerName = reader ? reader->getName() : "未知读者";
        QString bookTitle = book ? book->getTitle() : "未知图书";

        reservationInfo += QString("%1. 读者: %2 (%3), 图书: %4 (%5)\n")
                               .arg(i + 1)
                               .arg(reservations[i].first)
                               .arg(readerName)
                               .arg(reservations[i].second)
                               .arg(bookTitle);
    }

    QMessageBox::information(this, "预定记录", reservationInfo);
}

// ============== 工具 ==============

void MainWindow::on_generateDataButton_clicked()
{
    bool ok;
    int bookCount = QInputDialog::getInt(this, "生成测试数据",
                                         "请输入要生成的图书数量：",
                                         10, 1, 100, 1, &ok);

    if (ok) {
        int readerCount = QInputDialog::getInt(this, "生成测试数据",
                                               "请输入要生成的读者数量：",
                                               5, 1, 20, 1, &ok);

        if (ok) {
            libraryManager->generateRandomData(bookCount, readerCount);
            QMessageBox::information(this, "成功",
                                     QString("测试数据生成完成！\n生成图书：%1本\n生成读者：%2位")
                                         .arg(bookCount)
                                         .arg(readerCount));
        }
    }
}

// ============== Tab切换 ==============

void MainWindow::on_tabWidget_currentChanged(int index)
{
    if (index == 1) { // 读者管理标签
        updateReadersTable();
    } else if (index == 2) { // 统计分析标签
        updateStatistics();
        updateTimeDisplay();
    }
}
