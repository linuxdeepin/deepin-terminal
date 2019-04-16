#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <QVBoxLayout>
#include <QStackedWidget>

DWIDGET_USE_NAMESPACE

class TabBar;
class TermWidgetPage;
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void addTab(bool activeTab = false);
    void closeTab(const QString &identifier);
    void focusTab(const QString &identifier);

protected slots:
    void onTermTitleChanged(QString title);

private:
    void initTitleBar();
    void setNewTermPage(TermWidgetPage *termPage, bool activePage = true);

    TabBar *m_tabbar;
    QWidget *m_centralWidget;
    QVBoxLayout *m_centralLayout;
    QStackedWidget *m_termStackWidget;
};

#endif // MAINWINDOW_H
