#include "borrowrecord.h"

BorrowRecord::BorrowRecord() :
    readerId(""),
    bookId(""),
    borrowDate(QDate::currentDate()),
    dueDate(borrowDate.addDays(30))
{
}

BorrowRecord::BorrowRecord(QString readerId, QString bookId,
                           QDate borrowDate, QDate dueDate, QDate returnDate) :
    readerId(readerId),
    bookId(bookId),
    borrowDate(borrowDate),
    dueDate(dueDate),
    returnDate(returnDate)
{
}

void BorrowRecord::saveToStream(QTextStream &out) const
{
    out << readerId << ","
        << bookId << ","
        << borrowDate.toString("yyyy-MM-dd") << ","
        << dueDate.toString("yyyy-MM-dd") << ","
        << returnDate.toString("yyyy-MM-dd") << "\n";
}

void BorrowRecord::loadFromStream(QTextStream &in)
{
    QString line = in.readLine();
    QStringList fields = line.split(",");

    if (fields.size() >= 5) {
        readerId = fields[0];
        bookId = fields[1];
        borrowDate = QDate::fromString(fields[2], "yyyy-MM-dd");
        dueDate = QDate::fromString(fields[3], "yyyy-MM-dd");
        returnDate = QDate::fromString(fields[4], "yyyy-MM-dd");
    }
}
