#include "book.h"
#include <QDebug>

Book::Book() :
    id(""),
    title(""),
    author(""),
    category(OTHER),
    totalCopies(0),
    availableCopies(0),
    status(AVAILABLE)
{
}

Book::Book(QString id, QString title, QString author,
           BookCategory category, int totalCopies, int availableCopies) :
    id(id),
    title(title),
    author(author),
    category(category),
    totalCopies(totalCopies),
    availableCopies(availableCopies)
{
    status = (availableCopies > 0) ? AVAILABLE : BORROWED;
}

QString Book::getCategoryString() const
{
    switch(category) {
    case SCIENCE: return "科学";
    case TECHNOLOGY: return "技术";
    case LITERATURE: return "文学";
    case HISTORY: return "历史";
    case ART: return "艺术";
    default: return "其他";
    }
}

QString Book::getStatusString() const
{
    switch(status) {
    case AVAILABLE: return "可借";
    case BORROWED: return "已借出";
    case RESERVED: return "已预定";
    case LOST: return "丢失";
    default: return "未知";
    }
}

bool Book::borrowBook()
{
    if (availableCopies > 0) {
        availableCopies--;
        status = (availableCopies > 0) ? AVAILABLE : BORROWED;
        return true;
    }
    return false;
}

bool Book::returnBook()
{
    if (availableCopies < totalCopies) {
        availableCopies++;
        status = AVAILABLE;
        return true;
    }
    return false;
}

bool Book::reserveBook()
{
    if (availableCopies == 0 && status != RESERVED) {
        status = RESERVED;
        return true;
    }
    return false;
}

void Book::saveToStream(QTextStream &out) const
{
    out << id << ","
        << title << ","
        << author << ","
        << static_cast<int>(category) << ","
        << totalCopies << ","
        << availableCopies << ","
        << static_cast<int>(status) << "\n";
}

void Book::loadFromStream(QTextStream &in)
{
    QString line = in.readLine();
    QStringList fields = line.split(",");

    if (fields.size() >= 7) {
        id = fields[0];
        title = fields[1];
        author = fields[2];
        category = static_cast<BookCategory>(fields[3].toInt());
        totalCopies = fields[4].toInt();
        availableCopies = fields[5].toInt();
        status = static_cast<BookStatus>(fields[6].toInt());
    }
}
