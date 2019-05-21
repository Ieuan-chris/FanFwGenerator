#ifndef COMPILER_H
#define COMPILER_H

#include <QObject>
#include <QProcess>

class Compiler : public QObject
{
    Q_OBJECT

public:
    explicit Compiler(QObject *parent = nullptr);

signals:
    void compileDone(void);

public slots:
    void doTask(void);
//    void processError(QProcess::ProcessError error);
//    void processOk(void);
};

#endif // COMPILER_H
