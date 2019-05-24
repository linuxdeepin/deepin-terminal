#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <QVBoxLayout>
#include <QStackedWidget>

DWIDGET_USE_NAMESPACE

class TabBar;
class ThemePanel;
class TermWidgetPage;
class TermProperties;
class MainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(TermProperties properties, QWidget *parent = nullptr);
    ~MainWindow() override;

    void addTab(TermProperties properties, bool activeTab = false);
    void closeTab(const QString &identifier);
    void focusTab(const QString &identifier);
    TermWidgetPage *currentTab();

protected:
    void closeEvent(QCloseEvent *event) override;

protected slots:
    void onTermTitleChanged(QString title);
    void onTabTitleChanged(QString title);

private:
    void initWindow();
    void initShortcuts();
    void initTitleBar();
    void setNewTermPage(TermWidgetPage *termPage, bool activePage = true);

    QMenu *m_menu;
    TabBar *m_tabbar;
    ThemePanel *m_themePanel;
    QWidget *m_centralWidget;
    QVBoxLayout *m_centralLayout;
    QStackedWidget *m_termStackWidget;
};

#endif // MAINWINDOW_H
