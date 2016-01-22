#include "logger.h"
#include "timer.h"

QPlainTextEdit* Logger::messages = nullptr;

void Logger::setOutput(QPlainTextEdit* out)
{
    Logger::messages = out;
}

void Logger::info(const QString& message)
{
    if (Logger::messages == nullptr) return;
    Logger::messages->appendHtml(QString("<font style=\"font-style:italic\">[%1]</font> %2").arg(QString::fromStdString(timeToString(Clock::now())), message));
}

void Logger::warn(const QString &message)
{
    if (Logger::messages == nullptr) return;
    Logger::messages->appendHtml(QString("<font style=\"color:orange;\"><font style=\"font-style:italic\">[%1]</font> %2</font>").arg(QString::fromStdString(timeToString(Clock::now())), message));
}

void Logger::error(const QString& message)
{
    if (Logger::messages == nullptr) return;
    Logger::messages->appendHtml(QString("<font style=\"color:red;\"><font style=\"font-style:italic\">[%1]</font> %2</font>").arg(QString::fromStdString(timeToString(Clock::now())), message));
}
