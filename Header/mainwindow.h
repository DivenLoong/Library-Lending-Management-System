#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QCloseEvent>
#include "librarymanager.h"

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

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件操作
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();

    // 图书管理
    void on_addBookButton_clicked();
    void on_editBookButton_clicked();
    void on_deleteBookButton_clicked();
    void on_searchButton_clicked();
    void on_clearSearchButton_clicked();
    void on_booksTable_itemDoubleClicked(QTableWidgetItem *item);
    void on_showBookHistoryButton_clicked();

    // 读者管理
    void on_addReaderButton_clicked();
    void on_editReaderButton_clicked();
    void on_deleteReaderButton_clicked();
    void on_searchReaderButton_clicked();
    void on_clearReaderSearchButton_clicked();
    void on_readersTable_itemDoubleClicked(QTableWidgetItem *item);
    void on_showReaderRecordsButton_clicked();

    // 借阅管理
    void on_borrowButton_clicked();
    void on_returnButton_clicked();
    void on_reserveButton_clicked();

    // 时间管理
    void on_setCustomDateButton_clicked();
    void on_resetTimeButton_clicked();

    // 统计功能
    void on_showStatisticsButton_clicked();
    void on_showBorrowRecordsButton_clicked();
    void on_showOverdueButton_clicked();
    void on_showReservationsButton_clicked();

    // 工具
    void on_generateDataButton_clicked();

    // Tab切换
    void on_tabWidget_currentChanged(int index);

    // 更新显示
    void updateBooksTable();
    void updateReadersTable();
    void updateStatistics();
    void updateTimeDisplay();

private:
    Ui::MainWindow *ui;
    LibraryManager *libraryManager;
    QString currentFileName;

    // 文件操作辅助函数
    bool saveDataToFile(const QString &fileName);
    bool loadDataFromFile(const QString &fileName);

    void setupTables();
    void showBookDetails(Book *book);
    void showReaderDetails(Reader *reader);
    Book getBookFromForm();
    Reader getReaderFromForm();
    void setBookToForm(const Book &book);
    void setReaderToForm(const Reader &reader);
    void clearBookForm();
    void clearReaderForm();
    bool validateBookForm();
    bool validateReaderForm();
    void saveWindowState();
    void restoreWindowState();
};

#endif // MAINWINDOW_H
