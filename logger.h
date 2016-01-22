#ifndef LOGGER_H
#define LOGGER_H

#include <string>

#include <QPlainTextEdit>
#include <QString>

class Logger
{
private:
    static QPlainTextEdit* messages;

public:
    static void setOutput(QPlainTextEdit* out);

    static void info(const QString& message);
    static void warn(const QString& message);
    static void error(const QString& message);
};

#endif // LOGGER_H
