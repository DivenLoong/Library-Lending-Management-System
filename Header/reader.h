#ifndef READER_H
#define READER_H

#include <QString>
#include <QDate>
#include <QTextStream>

class Reader
{
public:
    Reader();
    Reader(QString id, QString name, QString dept, QString phone);

    // 获取读者信息
    QString getId() const { return id; }
    QString getName() const { return name; }
    QString getDept() const { return dept; }
    QString getPhone() const { return phone; }
    QDate getRegisterDate() const { return registerDate; }
    bool getIsValid() const { return isValid; }

    // 设置读者信息
    void setId(QString id) { this->id = id; }
    void setName(QString name) { this->name = name; }
    void setDept(QString dept) { this->dept = dept; }
    void setPhone(QString phone) { this->phone = phone; }
    void setValid(bool valid) { isValid = valid; }

    // 文件操作
    void saveToStream(QTextStream &out) const;
    void loadFromStream(QTextStream &in);

private:
    QString id;           // 读者ID
    QString name;         // 姓名
    QString dept;         // 院系/部门
    QString phone;        // 电话
    QDate registerDate;   // 注册日期
    bool isValid;         // 是否有效
};

#endif // READER_H
