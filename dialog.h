#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QDialog>

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog();
	
private:
    void createSettingGroupBox(void);
    void modify_file(const QString&& name, const QString&& startStr, const QString&& newLine);

	QGroupBox *settingGroupBox;
    QGroupBox *logGroupBox;
    QGroupBox *operationGroupBox;
    QStringList *fanType_StringList;
    QTextEdit *logDisplay;
//    QLineEdit *fanTypeValue;
    QComboBox *fanType_ComboBox;
    QLineEdit *versionValue;

    QPushButton* generatorBtn;
    QPushButton* saverBtn;

    QFile* file;

    static QTextStream out;
    static bool isContinueToCompile;
    static QString m;

signals:
    void startToCompile(void);


public slots:
      void generate_firmware(void);
      void save_settings(void);
      void doResult(void);

private:
      void log_out(QString str);
};


#endif // MAINWINDOW_H
