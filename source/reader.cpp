#include "reader.h"
#include <QDate>

Reader::Reader() :
    id(""),
    name(""),
    dept(""),
    phone(""),
    registerDate(QDate::currentDate()),
    isValid(true)
{
}

Reader::Reader(QString id, QString name, QString dept, QString phone) :
    id(id),
    name(name),
    dept(dept),
    phone(phone),
    registerDate(QDate::currentDate()),
    isValid(true)
{
}

void Reader::saveToStream(QTextStream &out) const
{
    out << id << ","
        << name << ","
        << dept << ","
        << phone << ","
        << registerDate.toString("yyyy-MM-dd") << ","
        << (isValid ? "1" : "0") << "\n";
}

void Reader::loadFromStream(QTextStream &in)
{
    QString line = in.readLine();
    QStringList fields = line.split(",");

    if (fields.size() >= 6) {
        id = fields[0];
        name = fields[1];
        dept = fields[2];
        phone = fields[3];
        registerDate = QDate::fromString(fields[4], "yyyy-MM-dd");
        isValid = (fields[5] == "1");
    }
}
