#include "wesley.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QProcess>

#include <stdio.h>



Wesley::Wesley(QObject *parent) : QThread(parent)
{

}

Wesley::~Wesley()
{
    this->exit();
}

void Wesley::run()
{
    qDebug() << "Make it so.";

    // Step 0.6: read the arguments. Need to have source directory. Build directory is an option, install directory is an option.
    QStringList args = QCoreApplication::arguments();

    bool flagOK = false;
    if (args.length() == 1)
    {
        if (!(qgetenv("SRC").isEmpty() && qgetenv("BLD").isEmpty()))
        {
            flagOK = true;
        }
    }
    else
        flagOK = true;


    if (!flagOK || args.contains("-help") || args.contains("-wat") || args.contains("-dafuq")) {
        this->usage();
    }
    else if (checkArgsAreCool(args)) {
        // TODO #fwindows check
        QString configure = "configure";
        QFileInfo configureInfo(m_source.path() + QDir::separator() + configure);
        if (!configureInfo.exists()) {
            writeToLog("No configure file in the source directory!");
        }
        else {
            QStringList configArgs = buildConfigureLine(args);

            // IT WOULD BE BETTER JUST TO BUILD THE CONFIG LINE AS A QSTRIGNLIST
            QString configure = m_source.path() + QDir::separator() + "configure";

            m_elProc = new QProcess();
            m_elProc->setWorkingDirectory(m_build.path());
            m_elProc->setArguments(configArgs);

            connect(m_elProc, &QProcess::readyReadStandardError, this, &Wesley::writeError);
            connect(m_elProc, &QProcess::readyReadStandardOutput, this, &Wesley::writeRegular);

            writeToLog("Starting the process.");

            QString args = configArgs.join(" ");

            configure += " ";
            configure += args;
            configure += " -v";
            writeToLog(configure);

            m_elProc->start(configure);

            m_elProc->waitForFinished((10 * 60 * 1000));

        }
    }

    QCoreApplication::exit();
}

QStringList Wesley::buildConfigureLine(QStringList arguments)
{
    QStringList theStripLine;

    if (arguments.contains("-release")) {
        theStripLine << " -release";
    }
    else {
        theStripLine << " -debug";
#ifdef Q_OS_MAC
        theStripLine << " -no-framework";
#endif
    }

    if (arguments.contains("-static")) {
        theStripLine << " -static";
    }

    if (arguments.contains("-com")) {
        theStripLine << " -commercial";
    }
    else {
        theStripLine << " -opensource";
    }
    theStripLine << " -confirm-license";

    // Check the mode first.
    switch (m_runMode) {
    case Wesley::Quick:
        m_yupped << "qtdeclarative" << "qtsvg" << "qtxmlpatterns";
        theStripLine << processSourceDirectory();
    case Wesley::Phat:
        theStripLine << " -nomake examples -nomake tests";
        break;
    case Wesley::Overnight:
        break;
    default:
        break;
    }

    if (!arguments.contains("-properInstall")) {
        theStripLine << " -prefix " + m_prefix.path();
    }

    return theStripLine;
}

QStringList Wesley::processSourceDirectory() {
    QStringList ticket;
    QStringList qtDirectories = m_source.entryList(QStringList() << "qt*", QDir::Dirs | QDir::NoDotAndDotDot);

    QStringList innerYups;
    foreach (QString yup, m_yupped) {
        if (!yup.startsWith("qt")) {
            yup.prepend("qt");
        }
        innerYups << yup;
    }

    foreach(QString innerYup, innerYups) {
        if (qtDirectories.contains(innerYup))
            qtDirectories.removeAll(innerYup);
    }

    foreach (QString nope, qtDirectories) {
        ticket << " -skip " << nope;
    }

    return ticket;
}

void Wesley::usage()
{
    qWarning(" ");
    qWarning("The Wesley Qt Configuration Tool");
    qWarning("Crushing it");
    qWarning(" ");
    qWarning("Because configuring Qt is a chore");
    qWarning("And they keep adding to it all the time");
    qWarning("And sometimes you just want to get work done");
    qWarning(" ");
    qWarning("Arguments");
    qWarning(" ");
    qWarning("-src              The source code directory. You can also use the SRC environment variable.");
    qWarning("-bld              The build directory. Y'all can also use $BLD");
    qWarning("-add              Add this module. By default we build just enough to run QtQuick, nothing more.");
    qWarning("-prefix           Where y'all are installing it. Default is $PWD-install, the shadow install approach.");
    qWarning("-quick            Just enough to build QtQuick applications (like, default, plus any adds).");
    qWarning("-phat             Build a phat version of Qt with too much crap.");
    qWarning("-overnight        Like, actually build EVERYYTHING in the Qt install.");
    qWarning("-properInstall    Install to /usr/local/Qt-whatever");
    qWarning("-com              Build the commercial version.");
    qWarning("-release          Build the release version. Default is debug.");
    qWarning("-static           Link it up statically.");
    qWarning(" ");
    qWarning("Got get 'em, tiger.");
    qWarning(" ");
}

void Wesley::writeError()
{
    QString str = m_elProc->readAllStandardError();
    writeToLog(str);
}

void Wesley::writeRegular()
{
    QString str = m_elProc->readAllStandardOutput();
    printf("%s", str.toLatin1().data());
    fflush(stdout);
}


void Wesley::writeToLog(QString message)
{
    static QFile logFile(m_build.path() + QDir::separator() + "Wesley.txt");

    if (logFile.open(QIODevice::WriteOnly|QIODevice::Append)) {
        logFile.write(message.toLatin1());
        logFile.write("\n");
        logFile.flush();
        logFile.close();
    }
    qDebug() << message;
}

// Sending the arguments in here is pointless. Wai?
bool Wesley::checkArgsAreCool(QStringList arguments)
{
    bool ticket = false;
    m_runMode = Wesley::Quick;

    QString sauceEnv = qgetenv("SRC");
    QString billedEnd = qgetenv("BLD");
    if (!sauceEnv.isEmpty() || arguments.contains("-src"))
    {
        QDir tmp;
        if (sauceEnv.isEmpty())
            tmp = QDir(checkAnArgument(arguments, "-src"));
        else
            tmp = QDir(sauceEnv);
        if (tmp.exists())
            m_source = tmp;
        else
            m_source = "";

        m_build = QDir::current();
        if (!billedEnd.isEmpty() || arguments.contains("-bld"))
        {
            if (billedEnd.isEmpty())
                tmp = QDir(checkAnArgument(arguments, "-bld"));
            else
                tmp = QDir(billedEnd);

            if (tmp.exists())
                m_build = tmp;
        }
        if (m_build == m_source)
        {
            writeToLog("This app only allows shadow builds.");
        }
        else
        {
            QString configure = m_source.path() + QDir::separator() + "configure";
            QFileInfo confinfo(configure);
            if (confinfo.exists())
            {
                // Everything that matters is here. You may proceed to the less important things.
                ticket = true;

                // Let's löook for adds.

                m_yupped << "qtbase";

                for (int i = 0; i < arguments.length(); i++) {
                    QString argument = arguments.at(i);
                    if (argument == "-add") {
                        QString grTmp = grabAnArgument(arguments, i);
                        if (!grTmp.isEmpty()) {
                            m_yupped << grTmp;
                        }
                    }
                }
                QString prefix = checkAnArgument(arguments, "-prefix");
                if (!prefix.isEmpty()) {
                    m_prefix = QDir(prefix);
                }
                else {
                    m_prefix = QDir(m_build.path() + "-install");
                }

                if (arguments.contains("-phat")) {
                    m_runMode = Wesley::Phat;
                }
                if (arguments.contains("-quick")) {
                    m_runMode = Wesley::Quick;
                }
                if (arguments.contains("-phat")) {
                    m_runMode = Wesley::Phat;
                }
                if (arguments.contains("-overnight")) {
                    m_runMode = Wesley::Overnight;
                }
            }
            else {
                writeToLog("There is no configure file in the source directory. We can not do this.");
            }
        }
    }
    return ticket;
}

QString Wesley::grabAnArgument(QStringList arguments, int key)
{
    if (key < 0) return "";
    int value = key + 1;
    if (value == arguments.length()) return "";
    return arguments[value];
}

QString Wesley::checkAnArgument(QStringList arguments, QString argument)
{
    int key = arguments.indexOf(argument);
    if (key < 0) return "";
    int value = key + 1;
    if (value == arguments.length()) return "";
    return arguments[value];
}

