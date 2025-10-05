#include "gui_shell.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QProcess>
#include <QString>
#include <QStringList>

ShellWindow::ShellWindow(QWidget *parent) : QMainWindow(parent) {
    QWidget *central = new QWidget;
    setCentralWidget(central);

    QVBoxLayout *layout = new QVBoxLayout(central);
    output = new QTextEdit;
    output->setReadOnly(true);
    output->append("Welcome to GUI Shell!\n");
    input = new QLineEdit;
    input->setPlaceholderText("Enter command...");
    layout->addWidget(output);
    layout->addWidget(input);

    connect(input, &QLineEdit::returnPressed, this, &ShellWindow::executeCommand);

    setWindowTitle("My Shell GUI");
    resize(600, 400);
}

void ShellWindow::executeCommand() {
    QString cmd = input->text().trimmed();
    input->clear();

    if (cmd == "clear") {
        output->clear();
        output->append("Welcome to GUI Shell!\n");
        return;
    } else if (cmd == "exit") {
        QApplication::quit();
        return;
    }

    output->append("Executing: " + cmd + "\n");

    QProcess *process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, process, cmd](int exitCode, QProcess::ExitStatus exitStatus) {
        QString outputText = QString::fromUtf8(process->readAllStandardOutput());
        QString errorText = QString::fromUtf8(process->readAllStandardError());
        if (!outputText.isEmpty()) {
            this->output->append("Output:\n" + outputText);
        }
        if (!errorText.isEmpty()) {
            this->output->append("Error:\n" + errorText);
        }
        if (exitCode != 0) {
            this->output->append("Exit code: " + QString::number(exitCode) + "\n");
        }
        process->deleteLater();
    });
    // For now, execute via bash. To integrate your custom shell, replace with your parsing logic.
    process->start("/bin/bash", QStringList() << "-c" << cmd);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ShellWindow window;
    window.show();
    return app.exec();
}