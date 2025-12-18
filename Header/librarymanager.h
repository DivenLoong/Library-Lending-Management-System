#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QList>
#include <QString>
#include <QPair>
#include <QSettings>
#include <QDate>
#include <QTime>
#include "book.h"
#include "borrowrecord.h"
#include "reader.h"

class LibraryManager : public QObject
{
    Q_OBJECT

public:
    explicit LibraryManager(QObject *parent = nullptr);
    ~LibraryManager();

    // 图书管理
    bool addBook(const Book &book);
    bool removeBook(const QString &id);
    bool updateBook(const Book &book);
    Book* findBook(const QString &id);
    QVector<Book*> searchBooks(const QString &keyword,
                                BookCategory category = OTHER,
                                bool searchByTitle = true,
                                bool searchByAuthor = true);
    QVector<Book*> getAllBooks() const;

    // 读者管理
    bool addReader(const Reader &reader);
    bool removeReader(const QString &id);
    bool updateReader(const Reader &reader);
    Reader* findReader(const QString &id);
    QVector<Reader*> getAllReaders() const;
    QVector<Reader*> searchReaders(const QString &keyword);

    // 借阅管理
    bool borrowBook(const QString &readerId, const QString &bookId, QDate borrowDate = QDate());
    bool returnBook(const QString &readerId, const QString &bookId, QDate returnDate = QDate());
    bool reserveBook(const QString &readerId, const QString &bookId);

    // 查询功能
    QVector<BorrowRecord> getBorrowRecordsByBook(const QString &bookId) const;
    QVector<BorrowRecord> getBorrowRecordsByReader(const QString &readerId) const;
    QVector<BorrowRecord> getOverdueRecords() const;
    QVector<QPair<QString, QString>> getReservations() const;
    QVector<QString> getReservatorsByBook(const QString &bookId) const;

    // 统计功能
    int getTotalBookCount() const;
    int getAvailableBookCount() const;
    int getBorrowedBookCount() const;
    int getCategoryCount() const;
    QMap<BookCategory, int> getCategoryStatistics() const;
    int getTotalReaderCount() const;

    // 时间管理
    QDate getCurrentDate() const;
    void setCurrentDate(const QDate &date);
    void resetToRealTime();
    bool isUsingCustomTime() const;

    // ============== 新增的函数声明 ==============
    // 借阅记录管理
    void addBorrowRecord(const BorrowRecord &record);
    QVector<BorrowRecord> getAllBorrowRecords() const;

    // 数据管理
    void clearAllData();
    bool saveToFile(const QString &filename);      // 新增
    bool loadFromFile(const QString &filename);    // 新增
    bool saveAllData();
    bool loadAllData();
    bool saveSettings();
    bool loadSettings();

    // 随机生成数据（测试用）
    void generateRandomData(int bookCount, int readerCount = 5);

signals:
    void dataChanged();
    void currentDateChanged(const QDate &newDate);

private:
    QMap<QString, Book*> books;                // 图书
    QMap<QString, Reader*> readers;            // 读者
    QList<BorrowRecord> borrowRecords;         // 借阅记录
    QList<QPair<QString, QString>> reservations; // 预定记录
    QSettings settings;                         // 配置文件
    QDate customCurrentDate;                    // 自定义当前日期
    bool useCustomTime;                         // 是否使用自定义时间
};

#endif // LIBRARYMANAGER_H
