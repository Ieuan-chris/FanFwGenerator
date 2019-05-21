#include <QProcess>
#include <QDebug>

#include "compiler.h"

Compiler::Compiler(QObject *parent) : QObject(parent)
{
}

//void Compiler::processOk(void)
//{
//    qDebug() << "Compile Failed" << endl;
//}

void Compiler::doTask(void)
{
    QProcess compileCmd;
    QStringList parameter;

//    connect(&compileCmd, SIGNAL(errorOccurred(QProess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
//    connect(&compileCmd, SIGNAL(started()), this, SLOT(processOk()));
    compileCmd.setWorkingDirectory("..");
    parameter << "-b" << "-j0" << "RT7100.uvprojx" << "-o" << ".\\build_log.txt";
    compileCmd.start("D:\\Keil_v5\\UV4\\UV4.exe", parameter);
    if(compileCmd.waitForFinished(-1)) {
        qDebug() << "complete!!!" << endl;
        emit compileDone();
    }
    else {
        qDebug() << "Error!!!" << endl;
    }
}

//void Compiler::processError(QProcess::ProcessError error)
//{
//    qDebug() << error << endl;
//}
