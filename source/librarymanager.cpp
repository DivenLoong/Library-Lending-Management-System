#include "librarymanager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRandomGenerator>
#include <QDate>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>

LibraryManager::LibraryManager(QObject *parent) :
    QObject(parent),
    settings("LibrarySystem", "BookManagement"),
    useCustomTime(false)
{
    // 程序启动时自动加载上次的数据
    loadSettings();
}

LibraryManager::~LibraryManager()
{
    // 程序关闭时自动保存数据
    saveSettings();

    // 清理内存
    qDeleteAll(books);
    qDeleteAll(readers);
    books.clear();
    readers.clear();
}

// 图书管理函数
bool LibraryManager::addBook(const Book &book)
{
    if (books.contains(book.getId())) {
        return false;  // ID已存在
    }

    Book *newBook = new Book(book);
    books.insert(newBook->getId(), newBook);
    emit dataChanged();
    return true;
}

bool LibraryManager::removeBook(const QString &id)
{
    if (books.contains(id)) {
        Book *book = books.take(id);
        delete book;
        emit dataChanged();
        return true;
    }
    return false;
}

bool LibraryManager::updateBook(const Book &book)
{
    if (books.contains(book.getId())) {
        Book *existingBook = books[book.getId()];
        *existingBook = book;
        emit dataChanged();
        return true;
    }
    return false;
}

Book* LibraryManager::findBook(const QString &id)
{
    return books.value(id, nullptr);
}

QVector<Book*> LibraryManager::searchBooks(const QString &keyword,
                                            BookCategory category,
                                            bool searchByTitle,
                                            bool searchByAuthor)
{
    QVector<Book*> results;

    for (Book *book : books) {
        bool matches = false;

        // 类别筛选
        if (category != OTHER && book->getCategory() != category) {
            continue;
        }

        // 关键词搜索
        if (!keyword.isEmpty()) {
            if (searchByTitle && book->getTitle().contains(keyword, Qt::CaseInsensitive)) {
                matches = true;
            }
            if (searchByAuthor && book->getAuthor().contains(keyword, Qt::CaseInsensitive)) {
                matches = true;
            }
        } else {
            matches = true;
        }

        if (matches) {
            results.append(book);
        }
    }

    return results;
}

QVector<Book*> LibraryManager::getAllBooks() const
{
    QVector<Book*> allBooks;
    for (Book *book : books) {
        allBooks.append(book);
    }
    return allBooks;
}

// 读者管理函数
bool LibraryManager::addReader(const Reader &reader)
{
    if (readers.contains(reader.getId())) {
        return false;  // ID已存在
    }

    Reader *newReader = new Reader(reader);
    readers.insert(newReader->getId(), newReader);
    emit dataChanged();
    return true;
}

bool LibraryManager::removeReader(const QString &id)
{
    if (readers.contains(id)) {
        Reader *reader = readers.take(id);
        delete reader;
        emit dataChanged();
        return true;
    }
    return false;
}

bool LibraryManager::updateReader(const Reader &reader)
{
    if (readers.contains(reader.getId())) {
        Reader *existingReader = readers[reader.getId()];
        *existingReader = reader;
        emit dataChanged();
        return true;
    }
    return false;
}

Reader* LibraryManager::findReader(const QString &id)
{
    return readers.value(id, nullptr);
}

QVector<Reader*> LibraryManager::getAllReaders() const
{
    QVector<Reader*> allReaders;
    for (Reader *reader : readers) {
        allReaders.append(reader);
    }
    return allReaders;
}

QVector<Reader*> LibraryManager::searchReaders(const QString &keyword)
{
    QVector<Reader*> results;

    for (Reader *reader : readers) {
        if (keyword.isEmpty() ||
            reader->getId().contains(keyword, Qt::CaseInsensitive) ||
            reader->getName().contains(keyword, Qt::CaseInsensitive) ||
            reader->getDept().contains(keyword, Qt::CaseInsensitive) ||
            reader->getPhone().contains(keyword, Qt::CaseInsensitive)) {
            results.append(reader);
        }
    }

    return results;
}

// 借阅管理函数
bool LibraryManager::borrowBook(const QString &readerId, const QString &bookId, QDate borrowDate)
{
    Book *book = findBook(bookId);
    if (book && book->borrowBook()) {
        QDate currentDate = getCurrentDate();
        if (!borrowDate.isValid()) {
            borrowDate = currentDate;
        }

        BorrowRecord record(readerId, bookId,
                            borrowDate,
                            borrowDate.addDays(30));
        borrowRecords.append(record);
        emit dataChanged();
        return true;
    }
    return false;
}

bool LibraryManager::returnBook(const QString &readerId, const QString &bookId, QDate returnDate)
{
    Book *book = findBook(bookId);
    if (book && book->returnBook()) {
        // 找到对应的借阅记录并更新
        for (BorrowRecord &record : borrowRecords) {
            if (record.getReaderId() == readerId &&
                record.getBookId() == bookId &&
                !record.isReturned()) {

                if (!returnDate.isValid()) {
                    returnDate = getCurrentDate();
                }
                record.setReturnDate(returnDate);
                emit dataChanged();
                return true;
            }
        }
    }
    return false;
}

bool LibraryManager::reserveBook(const QString &readerId, const QString &bookId)
{
    Book *book = findBook(bookId);
    if (book && book->reserveBook()) {
        // 记录预定信息
        reservations.append(qMakePair(readerId, bookId));
        emit dataChanged();
        return true;
    }
    return false;
}

// 查询功能
QVector<BorrowRecord> LibraryManager::getBorrowRecordsByBook(const QString &bookId) const
{
    QVector<BorrowRecord> records;
    for (const BorrowRecord &record : borrowRecords) {
        if (record.getBookId() == bookId) {
            records.append(record);
        }
    }
    return records;
}

QVector<BorrowRecord> LibraryManager::getBorrowRecordsByReader(const QString &readerId) const
{
    QVector<BorrowRecord> records;
    for (const BorrowRecord &record : borrowRecords) {
        if (record.getReaderId() == readerId) {
            records.append(record);
        }
    }
    return records;
}

QVector<BorrowRecord> LibraryManager::getOverdueRecords() const
{
    QVector<BorrowRecord> overdue;
    QDate today = getCurrentDate();

    for (const BorrowRecord &record : borrowRecords) {
        if (!record.isReturned() && record.getDueDate() < today) {
            overdue.append(record);
        }
    }
    return overdue;
}

QVector<QPair<QString, QString>> LibraryManager::getReservations() const
{
    QVector<QPair<QString, QString>> result;
    for (const auto &reservation : reservations) {
        result.append(reservation);
    }
    return result;
}

QVector<QString> LibraryManager::getReservatorsByBook(const QString &bookId) const
{
    QVector<QString> reservators;
    for (const auto &reservation : reservations) {
        if (reservation.second == bookId) {
            reservators.append(reservation.first);
        }
    }
    return reservators;
}

// 统计功能
int LibraryManager::getTotalBookCount() const
{
    int total = 0;
    for (Book *book : books) {
        total += book->getTotalCopies();
    }
    return total;
}

int LibraryManager::getAvailableBookCount() const
{
    int available = 0;
    for (Book *book : books) {
        available += book->getAvailableCopies();
    }
    return available;
}

int LibraryManager::getBorrowedBookCount() const
{
    return getTotalBookCount() - getAvailableBookCount();
}

QMap<BookCategory, int> LibraryManager::getCategoryStatistics() const
{
    QMap<BookCategory, int> stats;

    for (BookCategory cat = SCIENCE; cat <= OTHER;
         cat = static_cast<BookCategory>(cat + 1)) {
        stats[cat] = 0;
    }

    for (Book *book : books) {
        stats[book->getCategory()] += book->getTotalCopies();
    }

    return stats;
}

int LibraryManager::getCategoryCount() const
{
    QMap<BookCategory, int> stats = getCategoryStatistics();
    int count = 0;
    for (BookCategory cat = SCIENCE; cat <= OTHER;
         cat = static_cast<BookCategory>(cat + 1)) {
        if (stats[cat] > 0) {
            count++;
        }
    }
    return count;
}

int LibraryManager::getTotalReaderCount() const
{
    return readers.size();
}

// 时间管理函数
QDate LibraryManager::getCurrentDate() const
{
    return useCustomTime ? customCurrentDate : QDate::currentDate();
}

void LibraryManager::setCurrentDate(const QDate &date)
{
    if (date.isValid()) {
        customCurrentDate = date;
        useCustomTime = true;
        emit currentDateChanged(date);
        emit dataChanged();
        qDebug() << "系统时间已设置为：" << date.toString("yyyy-MM-dd");
    }
}

void LibraryManager::resetToRealTime()
{
    useCustomTime = false;
    emit currentDateChanged(QDate::currentDate());
    emit dataChanged();
    qDebug() << "已恢复使用系统实时时间";
}

bool LibraryManager::isUsingCustomTime() const
{
    return useCustomTime;
}

// ============== 添加的补充函数 ==============

// 添加借阅记录
void LibraryManager::addBorrowRecord(const BorrowRecord &record)
{
    borrowRecords.append(record);
    emit dataChanged();
}

// 获取所有借阅记录
QVector<BorrowRecord> LibraryManager::getAllBorrowRecords() const
{
    QVector<BorrowRecord> allRecords;
    for (const BorrowRecord &record : borrowRecords) {
        allRecords.append(record);
    }
    return allRecords;
}

// 清空所有数据
void LibraryManager::clearAllData()
{
    qDeleteAll(books);
    books.clear();

    qDeleteAll(readers);
    readers.clear();

    borrowRecords.clear();
    reservations.clear();

    useCustomTime = false;
    customCurrentDate = QDate();

    emit dataChanged();
    qDebug() << "所有数据已清空";
}

// 保存所有数据到指定文件
bool LibraryManager::saveToFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件进行写入：" << filename;
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    try {
        // 保存图书数据
        out << "#BOOKS\n";
        out << books.size() << "\n";
        for (Book *book : books) {
            book->saveToStream(out);
        }

        // 保存读者数据
        out << "#READERS\n";
        out << readers.size() << "\n";
        for (Reader *reader : readers) {
            reader->saveToStream(out);
        }

        // 保存借阅记录
        out << "#BORROWS\n";
        out << borrowRecords.size() << "\n";
        for (const BorrowRecord &record : borrowRecords) {
            record.saveToStream(out);
        }

        // 保存预定记录
        out << "#RESERVATIONS\n";
        out << reservations.size() << "\n";
        for (const auto &reservation : reservations) {
            out << reservation.first << "," << reservation.second << "\n";
        }

        // 保存时间设置
        out << "#SETTINGS\n";
        out << (useCustomTime ? "1" : "0") << "\n";
        if (useCustomTime) {
            out << customCurrentDate.toString("yyyy-MM-dd") << "\n";
        }

        file.close();
        qDebug() << "数据已保存到：" << filename;
        return true;
    } catch (...) {
        file.close();
        qDebug() << "保存文件时发生错误：" << filename;
        return false;
    }
}

// 从指定文件加载数据
bool LibraryManager::loadFromFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件进行读取：" << filename;
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    try {
        // 清空现有数据
        clearAllData();

        QString line;
        QString section;

        while (!in.atEnd()) {
            line = in.readLine().trimmed();

            if (line.isEmpty()) continue;

            if (line == "#BOOKS") {
                int count = in.readLine().toInt();
                for (int i = 0; i < count; ++i) {
                    Book *book = new Book();
                    book->loadFromStream(in);
                    books.insert(book->getId(), book);
                }
            }
            else if (line == "#READERS") {
                int count = in.readLine().toInt();
                for (int i = 0; i < count; ++i) {
                    Reader *reader = new Reader();
                    reader->loadFromStream(in);
                    readers.insert(reader->getId(), reader);
                }
            }
            else if (line == "#BORROWS") {
                int count = in.readLine().toInt();
                for (int i = 0; i < count; ++i) {
                    BorrowRecord record;
                    record.loadFromStream(in);
                    borrowRecords.append(record);

                    // 如果图书已借出，更新图书状态
                    if (!record.isReturned()) {
                        Book *book = findBook(record.getBookId());
                        if (book) {
                            book->borrowBook();
                        }
                    }
                }
            }
            else if (line == "#RESERVATIONS") {
                int count = in.readLine().toInt();
                for (int i = 0; i < count; ++i) {
                    QString recordLine = in.readLine();
                    QStringList parts = recordLine.split(",");
                    if (parts.size() >= 2) {
                        reservations.append(qMakePair(parts[0], parts[1]));
                        // 更新图书预定状态
                        Book *book = findBook(parts[1]);
                        if (book && book->getAvailableCopies() == 0) {
                            book->reserveBook();
                        }
                    }
                }
            }
            else if (line == "#SETTINGS") {
                useCustomTime = (in.readLine().toInt() == 1);
                if (useCustomTime) {
                    QString dateStr = in.readLine();
                    customCurrentDate = QDate::fromString(dateStr, "yyyy-MM-dd");
                }
            }
        }

        file.close();
        emit dataChanged();
        qDebug() << "数据已从文件加载：" << filename;
        return true;
    } catch (...) {
        file.close();
        qDebug() << "加载文件时发生错误：" << filename;
        return false;
    }
}

// 保存数据到默认位置
bool LibraryManager::saveAllData()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(dataDir);
    }

    QString filename = dataDir + "/library_data.lib";
    return saveToFile(filename);
}

// 从默认位置加载数据
bool LibraryManager::loadAllData()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);

    QString filename = dataDir + "/library_data.lib";
    if (QFile::exists(filename)) {
        return loadFromFile(filename);
    } else {
        // 文件不存在，生成默认数据
        generateRandomData(10, 5);
        return true;
    }
}

// 保存设置
bool LibraryManager::saveSettings()
{
    settings.setValue("System/UseCustomTime", useCustomTime);
    if (useCustomTime) {
        settings.setValue("System/CustomDate", customCurrentDate);
    }
    settings.sync();
    qDebug() << "程序设置已保存";
    return true;
}

// 加载设置
bool LibraryManager::loadSettings()
{
    if (settings.contains("System/UseCustomTime")) {
        useCustomTime = settings.value("System/UseCustomTime").toBool();
        if (useCustomTime && settings.contains("System/CustomDate")) {
            customCurrentDate = settings.value("System/CustomDate").toDate();
        }
    }

    // 加载数据
    loadAllData();
    qDebug() << "程序设置已加载";
    return true;
}

// ============== 原有函数保持不变 ==============

// 随机生成数据（测试用）
void LibraryManager::generateRandomData(int bookCount, int readerCount)
{
    qDebug() << "生成随机数据：图书" << bookCount << "本，读者" << readerCount << "位";

    QDate currentDate = getCurrentDate();
    QRandomGenerator *rg = QRandomGenerator::global();

    // 生成读者数据
    QStringList names = {"张三", "李四", "王五", "赵六", "钱七", "孙八", "周九", "吴十"};
    QStringList depts = {"计算机学院", "数学学院", "物理学院", "化学学院", "文学院", "法学院", "医学院", "商学院"};

    for (int i = 0; i < readerCount; ++i) {
        QString id = QString("R%1").arg(1001 + i, 4, 10, QChar('0'));
        QString name = names[rg->bounded(names.size())];
        QString dept = depts[rg->bounded(depts.size())];
        QString phone = QString("138%1").arg(rg->bounded(10000000, 99999999));

        Reader reader(id, name, dept, phone);
        addReader(reader);
    }

    // 生成图书数据
    QStringList titles = {
        "C++ Primer", "Qt编程入门", "数据结构与算法",
        "深入理解计算机系统", "设计模式", "Python编程",
        "机器学习实战", "计算机网络", "操作系统原理",
        "数据库系统概念", "编译原理", "人工智能导论",
        "Java核心技术", "C#入门经典", "Web开发实战",
        "移动应用开发", "大数据处理", "云计算基础"
    };

    QStringList authors = {
        "李华", "张明", "王芳", "刘伟", "陈静",
        "赵强", "杨阳", "周涛", "吴敏", "郑浩",
        "孙亮", "周敏", "吴昊", "郑洁", "王磊"
    };

    for (int i = 0; i < bookCount; ++i) {
        QString id = QString("B%1").arg(i + 1001, 4, 10, QChar('0'));
        QString title = titles[rg->bounded(titles.size())];
        QString author = authors[rg->bounded(authors.size())];
        BookCategory category = static_cast<BookCategory>(rg->bounded(6));
        int totalCopies = rg->bounded(1, 10);
        int availableCopies = rg->bounded(0, totalCopies + 1);

        Book book(id, title, author, category, totalCopies, availableCopies);
        addBook(book);
    }

    // 生成借阅记录
    for (int i = 0; i < bookCount / 2; ++i) {
        QString readerId = QString("R%1").arg(1001 + rg->bounded(readerCount), 4, 10, QChar('0'));
        QString bookId = QString("B%1").arg(1001 + rg->bounded(bookCount), 4, 10, QChar('0'));

        if (findBook(bookId) && findReader(readerId)) {
            // 随机设置借阅日期（过去30天内）
            QDate borrowDate = currentDate.addDays(-rg->bounded(30));
            QDate dueDate = borrowDate.addDays(30);

            Book *book = findBook(bookId);
            if (book->borrowBook()) {
                BorrowRecord record(readerId, bookId, borrowDate, dueDate);

                // 随机设置是否已归还
                if (rg->bounded(2) == 0) {
                    QDate returnDate = borrowDate.addDays(rg->bounded(1, 40));
                    record.setReturnDate(returnDate);
                    book->returnBook(); // 归还后更新图书状态
                }

                borrowRecords.append(record);
            }
        }
    }

    // 生成预定记录
    for (int i = 0; i < bookCount / 4; ++i) {
        QString readerId = QString("R%1").arg(1001 + rg->bounded(readerCount), 4, 10, QChar('0'));
        QString bookId = QString("B%1").arg(1001 + rg->bounded(bookCount), 4, 10, QChar('0'));

        if (findBook(bookId) && findReader(readerId)) {
            Book *book = findBook(bookId);
            if (book->getAvailableCopies() == 0 && !book->getStatusString().contains("预定")) {
                if (book->reserveBook()) {
                    reservations.append(qMakePair(readerId, bookId));
                }
            }
        }
    }

    qDebug() << "随机数据生成完成";
    emit dataChanged();
}
