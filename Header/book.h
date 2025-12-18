#ifndef BOOK_H
#define BOOK_H

#include <QString>
#include <QDate>
#include <QTextStream>

// 图书状态枚举
enum BookStatus {
    AVAILABLE,      // 可借阅
    BORROWED,       // 已借出
    RESERVED,       // 已预定
    LOST           // 丢失
};

// 图书类别枚举
enum BookCategory {
    SCIENCE,        // 科学
    TECHNOLOGY,     // 技术
    LITERATURE,     // 文学
    HISTORY,        // 历史
    ART,           // 艺术
    OTHER          // 其他
};

class Book
{
public:
    // 构造函数
    Book();
    Book(QString id, QString title, QString author, BookCategory category,
         int totalCopies, int availableCopies);

    // 获取图书信息
    QString getId() const { return id; }
    QString getTitle() const { return title; }
    QString getAuthor() const { return author; }
    BookCategory getCategory() const { return category; }
    QString getCategoryString() const;
    int getTotalCopies() const { return totalCopies; }
    int getAvailableCopies() const { return availableCopies; }
    BookStatus getStatus() const { return status; }
    QString getStatusString() const;

    // 设置图书信息
    void setId(QString id) { this->id = id; }
    void setTitle(QString title) { this->title = title; }
    void setAuthor(QString author) { this->author = author; }
    void setCategory(BookCategory category) { this->category = category; }
    void setTotalCopies(int copies) { totalCopies = copies; }
    void setAvailableCopies(int copies) { availableCopies = copies; }
    void setStatus(BookStatus status) { this->status = status; }

    // 借阅相关操作
    bool borrowBook();
    bool returnBook();
    bool reserveBook();

    // 文件操作
    void saveToStream(QTextStream &out) const;
    void loadFromStream(QTextStream &in);

private:
    QString id;             // 图书编号
    QString title;          // 书名
    QString author;         // 作者
    BookCategory category;  // 类别
    int totalCopies;        // 总册数
    int availableCopies;    // 可借册数
    BookStatus status;      // 状态
};

#endif // BOOK_H
