#ifndef WESLEY_H
#define WESLEY_H

#include <QCommandLineParser>
#include <QDir>
#include <QThread>

class QProcess;

class Wesley : public QThread
{

    Q_OBJECT
public:
    explicit Wesley(QObject *parent = 0);
    ~Wesley();

    enum RunMode {
        Quick,
        Phat,
        Overnight
    };

protected:
    void run();

signals:

public slots:
    void writeError();
    void writeRegular();

private:
    void usage();
    void writeToLog(QString message);
    void writeLog();
    bool checkArgsAreCool(QStringList arguments);

    QStringList buildConfigureLine(QStringList arguments);
    QStringList processSourceDirectory();
    QString checkAnArgument(QStringList arguments, QString argument);
    QString grabAnArgument(QStringList arguments, int key);

    // 'yup' is the list of qt parts that I WANT to build.
    QStringList         m_yupped;
    QDir                m_source;
    QDir                m_build;
    QDir                m_prefix;
    QDir                m_installation;
    QCommandLineParser  m_parser;

    QProcess           *m_elProc;

    RunMode             m_runMode;
};

#endif // WESLEY_H
