#include <QtWidgets>
#include <QLabel>
#include <QLineEdit>
#include <QDir>
#include <QDebug>
#include <QStringList>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNode>

#include <iostream>

#include "dialog.h"
#include "compiler.h"


QTextStream Dialog::out;
bool Dialog::isContinueToCompile = true;
QString Dialog::m = "";

Dialog::Dialog()
{
	createSettingGroupBox();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(settingGroupBox);
    mainLayout->addWidget(logGroupBox);
    mainLayout->addWidget(operationGroupBox);
	
    setLayout(mainLayout);
	setWindowTitle(tr("FanFwGenerator V0.1"));


    file = new QFile("log.txt");
    if (!(*file).open(QIODevice::Append | QIODevice::Text))
        return;

    out.setDevice(file);
}


void Dialog::createSettingGroupBox()
{
	settingGroupBox = new QGroupBox(tr("Setting"));
	QHBoxLayout *layout = new QHBoxLayout;
	
	//版本号设置
    QLabel *versionLabel = new QLabel(tr("固件版本号："));
    versionValue = new QLineEdit();
    layout->addWidget(versionLabel);
    layout->addWidget(versionValue);

    // 风扇类型
    QLabel *fanTypeLabel = new QLabel(tr("风扇类型："));
//    fanTypeValue = new QLineEdit();

    QDir dir;
    dir.setPath("..\\AMC\\Parameter");

    fanType_StringList = new QStringList();
    fanType_StringList->append(tr("All"));
    for(auto a : dir.entryInfoList(QDir::Files | QDir::Hidden)) {
        if(a.fileName().endsWith(tr(".h")))
            fanType_StringList->append(a.fileName());
    }

    fanType_ComboBox = new QComboBox();
    fanType_ComboBox->addItems(*fanType_StringList);

    layout->addWidget(fanTypeLabel);
    layout->addWidget(fanType_ComboBox);

    logGroupBox = new QGroupBox("Logging");
    QHBoxLayout *layout1 = new QHBoxLayout;

    logDisplay = new QTextEdit();

    logDisplay->setReadOnly(true);
    logDisplay->setFixedSize(800, 800);

//    logDisplay->append("<p><font size='3' color='red'>Hello world</font></p>");

    layout1->addWidget(logDisplay);

    logGroupBox->setLayout(layout1);

    operationGroupBox = new QGroupBox("Command");
    QHBoxLayout *layout2 = new QHBoxLayout;
    generatorBtn = new QPushButton(tr("生成"));
    saverBtn = new QPushButton(tr("保存"));

    connect(generatorBtn, SIGNAL(clicked()), this, SLOT(generate_firmware()));
    connect(saverBtn, SIGNAL(clicked()), this, SLOT(save_settings()));

    layout2->addWidget(generatorBtn);
    layout2->addWidget(saverBtn);

    operationGroupBox->setLayout(layout2);

    settingGroupBox->setLayout(layout);	
}

void Dialog::generate_firmware(void)
{
    static QStringList *rmAll;
    static QString version;
    static Compiler* cmp;
    static QThread* t;


    if (generatorBtn->isEnabled()) {
        QFile fw("..\\Bin\\USER.code");
        if(fw.exists()) {
            fw.remove();
        }
        version = versionValue->text();
//        fanType = fanTypeValue->text();

        QRegExp rx(tr("[0-9]{2}\\.[0-9]{2}\\.[0-9]{4}"));
        if(!rx.exactMatch(version))
        {
            log_out("<b style='color:red'>version format error</b>");
            return;
        }
        log_out("version: " + version);

        modify_file("..\\AMC\\SysConfig.h",
                    "#define APP_FIRMWARE_VERSION", "#define APP_FIRMWARE_VERSION \"" + version +"\"");
        if (fanType_ComboBox->currentText() == "All")
        {
            rmAll = new QStringList(*fanType_StringList);
            rmAll->removeOne(tr("All"));
        } else {
            rmAll = new QStringList(fanType_ComboBox->currentText());
        }
        // 禁止部分组件
//        fanTypeValue->setEnabled(false);
        generatorBtn->setEnabled(false);
        versionValue->setEnabled(false);

    }

    if (rmAll->empty()) {
        log_out("done!!!");
        // 使能部分组件
//        fanTypeValue->setEnabled(true);
        generatorBtn->setEnabled(true);
        versionValue->setEnabled(true);
        return;
    }

    m = rmAll->first();
    QByteArray model = m.toLocal8Bit();
    model = model.toUpper().remove(model.indexOf(tr(".")), 2);

    log_out("generate_firmware" + m);

    modify_file("..\\AMC\\FanModel.h",
                "#define MC_WAC", model.insert(0, QString(tr("#define "))).append(' '));

    cmp = new Compiler();
    if (t && !t->isFinished())
        t->exit();
    t = new QThread();
    connect(cmp, SIGNAL(compileDone()), this, SLOT(doResult()));
    connect(this, SIGNAL(startToCompile()), cmp, SLOT(doTask()));
    cmp->moveToThread(t);
    t->start();
    emit startToCompile();
    rmAll->removeFirst();
}


 void Dialog::doResult(void)
 {
     QFile logFile("..\\build_log.txt");
     if (logFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
         QByteArray content;
         content = logFile.readAll();
         log_out(content);
         logFile.close();
     }
     QFile fw("..\\Bin\\USER.code");
     if(fw.exists()) {
         QFile locFw("..\\fw\\"+m+".code");
         if(locFw.exists()){
             if(!locFw.remove()) {
                 log_out("<b style='color:red'>remove failed</b>");
                 return;
             }
         }
         fw.rename("..\\fw\\"+m+".code");
         log_out("<b style='color:green'>" + m + " generate successed!!!" + "</b>");
     }
     else {
        log_out("<b style='color:red'>" + m + " generate failed!!!" + "</b>");
     }
     fw.close();
     generate_firmware();
 }

void Dialog::log_out(QString str)
{
    out << tr("[ ") << QDateTime::currentDateTime().toString() << tr(" ]: ") << str<< "\n";
    out.flush();
    logDisplay->append(str);
}


void Dialog::save_settings(void)
{
    log_out("save_settings");
}

void Dialog::modify_file(const QString&& name, const QString&& startStr, const QString&& newLine)
{
    QFile file(name);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)){
        log_out("<b style='color:red'>file not found!</b>");
        return;
    }
    else {
        QByteArray t ;
        while(!file.atEnd())
        {
            qint64 last_pos = file.pos();
            t = file.readLine();
            if (t.startsWith(startStr.toLocal8Bit()))
            {
//                log_out("find info" + t);
                file.seek(last_pos);
                t = t.replace(0, t.size(), newLine.toUtf8(), newLine.size());
                file.write(t);
                file.flush();
                break;
            }
        }
        file.close();
    }
}

