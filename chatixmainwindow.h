#ifndef CHATIXMAINWINDOW_H
#define CHATIXMAINWINDOW_H
#include "llmconnector.hpp"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class ChatixMainWindow;
}
QT_END_NAMESPACE

class ChatixMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChatixMainWindow(QWidget *parent = nullptr);
    ~ChatixMainWindow() override;

private slots:

    void on_sendButton_clicked();

private:
    Ui::ChatixMainWindow *ui;

    std::unique_ptr<lmc::llm> model;

    std::string context;

    std::string genHTML();
};
#endif // CHATIXMAINWINDOW_H
