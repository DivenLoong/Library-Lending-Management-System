#ifndef BORROWRECORD_H
#define BORROWRECORD_H

#include <QString>
#include <QDate>
#include <QTextStream>

class BorrowRecord
{
public:
    BorrowRecord();
    BorrowRecord(QString readerId, QString bookId, QDate borrowDate,
                 QDate dueDate, QDate returnDate = QDate());

    // 获取信息
    QString getReaderId() const { return readerId; }
    QString getBookId() const { return bookId; }
    QDate getBorrowDate() const { return borrowDate; }
    QDate getDueDate() const { return dueDate; }
    QDate getReturnDate() const { return returnDate; }
    bool isReturned() const { return !returnDate.isNull(); }

    // 设置信息
    void setReaderId(QString id) { readerId = id; }
    void setBookId(QString id) { bookId = id; }
    void setBorrowDate(QDate date) { borrowDate = date; }
    void setDueDate(QDate date) { dueDate = date; }
    void setReturnDate(QDate date) { returnDate = date; }

    // 操作
    void returnBook() { returnDate = QDate::currentDate(); }

    // 文件操作
    void saveToStream(QTextStream &out) const;
    void loadFromStream(QTextStream &in);

private:
    QString readerId;       // 读者ID
    QString bookId;         // 图书ID
    QDate borrowDate;       // 借阅日期
    QDate dueDate;          // 应还日期
    QDate returnDate;       // 归还日期
};

#endif // BORROWRECORD_H
