/**
 * GUI Shell Header
 * Qt-based graphical shell interface
 */

#ifndef GUI_SHELL_H
#define GUI_SHELL_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>

class ShellWindow : public QMainWindow {
    Q_OBJECT
public:
    ShellWindow(QWidget *parent = nullptr);

private slots:
    void executeCommand();

private:
    QTextEdit *output;
    QLineEdit *input;
};

#endif // GUI_SHELL_H