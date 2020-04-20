#include "termwidgetpage.h"
#include "termwidget.h"
#include "settings.h"
#include "utils.h"

#include <DLog>

#include <QUuid>
#include <QVBoxLayout>
#include <QApplication>

TermWidgetPage::TermWidgetPage(TermProperties properties, QWidget *parent)
    : QWidget(parent), m_findBar(new PageSearchBar(this))
{
    setFocusPolicy(Qt::NoFocus);
    setProperty("TAB_CUSTOM_NAME_PROPERTY", false);
    // 生成唯一 pageID
    setProperty("TAB_IDENTIFIER_PROPERTY", QUuid::createUuid().toString());

    TermWidget *w = createTerm(properties);
    DSplitter *splitter = new DSplitter(Qt::Horizontal, this);
    splitter->setFocusPolicy(Qt::NoFocus);
    setSplitStyle(splitter);
    splitter->addWidget(w);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(splitter);
    setLayout(layout);

    // Init find bar.
    m_findBar->move(this->x() - 100, this->y() - 100);
    connect(m_findBar, &PageSearchBar::findNext, this, &TermWidgetPage::handleFindNext);
    connect(m_findBar, &PageSearchBar::findPrev, this, &TermWidgetPage::handleFindPrev);
    connect(m_findBar, &PageSearchBar::keywordChanged, this, [ = ](QString keyword) {
        handleUpdateSearchKeyword(keyword);
    });
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, [ = ]() {
        applyTheme();
    });

    m_currentTerm = w;
}

void TermWidgetPage::setSplitStyle(DSplitter *splitter)
{
    splitter->setHandleWidth(1);
    QSplitterHandle *handle = splitter->handle(1);

    if (handle) {
        //分割线颜色暂时设置为Highlight颜色，需要和UI确认下
        DPalette pa = DApplicationHelper::instance()->palette(handle);
        QColor splitBrush = pa.color(DPalette::Highlight);
        pa.setBrush(DPalette::Background, splitBrush);
        handle->setPalette(pa);
        handle->setBackgroundRole(QPalette::Background);
        handle->setAutoFillBackground(true);
    }
}

TermWidget *TermWidgetPage::currentTerminal()
{
    return m_currentTerm;
}

TermWidget *TermWidgetPage::split(Qt::Orientation orientation)
{
    return split(currentTerminal(), orientation);
}

TermWidget *TermWidgetPage::split(TermWidget *term, Qt::Orientation orientation)
{
    emit pageRequestShowPlugin(MainWindow::PLUGIN_TYPE_NONE);
    QSplitter *parent = qobject_cast<QSplitter *>(term->parent());
    Q_CHECK_PTR(parent);

    int index = parent->indexOf(term);
    QList<int> parentSizes = parent->sizes();

    TermProperties properties(term->workingDirectory());
    TermWidget *w = createTerm(properties);
    // 意义与名称是相反的
    DSplitter *subSplit = new DSplitter(orientation == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal, this);
    subSplit->setFocusPolicy(Qt::NoFocus);
    subSplit->insertWidget(0, term);
    subSplit->insertWidget(1, w);
    subSplit->setSizes({ 1, 1 });
    setSplitStyle(subSplit);

    parent->insertWidget(index, subSplit);
    parent->setSizes(parentSizes);

    setSplitStyle(parent);
    w->setFocus(Qt::OtherFocusReason);
    return w;
}

void TermWidgetPage::closeSplit(TermWidget *term)
{
    if (term->hasRunningProcess() && !Utils::showExitConfirmDialog()) {
        return;
    }

    qDebug() << "TermWidgetPage::closeSplit:" << term->getSessionId();
    QSplitter *parent = qobject_cast<QSplitter *>(term->parent());
    Q_CHECK_PTR(parent);

    term->setParent(nullptr);
    //--midified by qinyaning(nyq) to solve that Create a number of workspaces in the thunder window,
    /*right click to close the window will close all workspaces, time: 2020.04.17 15:00
    */
    static QList<TermWidget *> terms;
    terms.push_back(term);
    if(term->getSessionId() == 1) {//当剩下最后一个工作区时
        for(int i = 0; i < terms.size(); i++)
            terms[i]->deleteLater();
    }
    //---------------------------------------------------------//

    QWidget *nextFocus = nullptr;

    // Collapse splitters containing a single element, excluding the top one.
    if (parent->count() == 1) {
        QSplitter *uselessSplitterParent = qobject_cast<QSplitter *>(parent->parent());
        if (uselessSplitterParent != nullptr) {
            int index = uselessSplitterParent->indexOf(parent);
            Q_ASSERT(index != -1);
            QWidget *singleHeir = parent->widget(0);
            uselessSplitterParent->insertWidget(index, singleHeir);
            setSplitStyle(uselessSplitterParent);
            if (qobject_cast<TermWidget *>(singleHeir)) {
                nextFocus = singleHeir;
            } else {
                nextFocus = singleHeir->findChild<TermWidget *>();
            }
            parent->setParent(nullptr);
            parent->deleteLater();
            // Make sure there's no access to the removed parent
            parent = uselessSplitterParent;
        }
    }

    if (parent->count() > 0) {
        if (nextFocus) {
            nextFocus->setFocus(Qt::OtherFocusReason);
        } else {
            parent->widget(0)->setFocus(Qt::OtherFocusReason);
        }
        parent->update();
    } else {
        emit lastTermClosed(identifier());
    }
}

const QString TermWidgetPage::identifier()
{
    return property("TAB_IDENTIFIER_PROPERTY").toString();
}

void TermWidgetPage::focusCurrentTerm()
{
    m_currentTerm->setFocus();
}

void TermWidgetPage::closeOtherTerminal()
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();

    int currSessionId = m_currentTerm->getSessionId();
    //exit protection
    for (TermWidget *term : qAsConst(termList)) {
        if (term->getSessionId() != currSessionId) {
            closeSplit(term);
        }
    }
}

// 待删除
typedef struct {
    QPoint topLeft;
    QPoint middle;
    QPoint bottomRight;
} CoordinateRect;

// 待删除
static void transpose(QPoint *point)
{
    int x = point->x();
    point->setX(point->y());
    point->setY(x);
}

// 待删除
static void transposeTransform(CoordinateRect *point)
{
    transpose(&point->topLeft);
    transpose(&point->middle);
    transpose(&point->bottomRight);
}

// 待删除
static void flipTransform(CoordinateRect *point)
{
    QPoint oldTopLeft = point->topLeft;
    point->topLeft = -(point->bottomRight);
    point->bottomRight = -(oldTopLeft);
    point->middle = -(point->middle);
}

// 待删除
static void normalizeToRight(CoordinateRect *point, Qt::Edge dir)
{
    switch (dir) {
    case Qt::LeftEdge:
        flipTransform(point);
        break;
    case Qt::RightEdge:
        // No-op
        break;
    case Qt::TopEdge:
        flipTransform(point);
        transposeTransform(point);
        break;
    case Qt::BottomEdge:
        transposeTransform(point);
        break;
    default:
        qFatal("Invalid navigation");
        return;
    }
}

/******** Modify by n014361 wangpeili 2020-01-07:  修改了计算上下左右判断的方法 ********×****/
void TermWidgetPage::focusNavigation(Qt::Edge dir)
{
    QPoint comparPoint = GetComparePoint(currentTerminal(), dir);
    TermWidget *dst = nullptr;
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    QMap<TermWidget *, QRect> mapTermRect;
    for (TermWidget *term : qAsConst(termList)) {
        if (GetRect(term).contains(comparPoint)) {
            qDebug() << "yes!" << comparPoint.x() << comparPoint.y();
            dst = term;
            break;
        }
    }
    if (dst) {
        dst->setFocus();
    }
}
/*******************************************************************************
 1. @函数:    getTerminalCount
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-21
 4. @说明:    获取当前页的终端个数
*******************************************************************************/
int TermWidgetPage::getTerminalCount()
{
    return findChildren<TermWidget *>().size();
}
/********************* Modify by n014361 wangpeili End ************************/

/*******************************************************************************
 1. @函数:  QRect TermWidgetPage::GetRect(TermWidgetWrapper *term)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:  获取标签页内所有子窗体的区域
*******************************************************************************/
QRect TermWidgetPage::GetRect(TermWidget *term)
{

    QPoint leftTop = term->mapTo(term->window(), QPoint(0, 0));
    QPoint rightBottom = term->mapTo(term->window(), QPoint(term->width(), term->height()));
    QRect rec(leftTop, rightBottom);
    qDebug() << "leftTop: " << leftTop.x() << leftTop.y() << "rightBottom: " << rightBottom.x() << rightBottom.y();
    return rec;
}

/*******************************************************************************
 1. @函数: QPoint TermWidgetPage::GetComparePoint(TermWidgetWrapper *term, Qt::Edge dir)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-08
 4. @说明:     获取窗口上下左右键需要判断的点位信息
*******************************************************************************/
QPoint TermWidgetPage::GetComparePoint(TermWidget *term, Qt::Edge dir)
{
    QPoint leftTop = term->mapTo(term->window(), QPoint(0, 0));
    QPoint leftBottom = term->mapTo(term->window(), QPoint(0, term->height()));
    QPoint rightTop = term->mapTo(term->window(), QPoint(term->width(), 0));
    QPoint ret;
    switch (dir) {
    case Qt::LeftEdge:
        ret = leftTop + QPoint(-1, 1);
        break;
    case Qt::RightEdge:
        ret = rightTop + QPoint(1, 1);
        break;
    case Qt::TopEdge:
        ret = leftTop + QPoint(1, -1);
        break;
    case Qt::BottomEdge:
        ret = leftBottom + QPoint(1, 1);
        break;
    default:
        qFatal("Invalid navigation");
        break;
    }
    return ret;
}

bool TermWidgetPage::hasRunningProcess()
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        if (term->hasRunningProcess()) {
            return true;
        }
    }
    return false;
}

TermProperties TermWidgetPage::createCurrentTerminalProperties()
{
    TermProperties properties;

    TermWidget *term = currentTerminal();
    if (term) {
        properties[WorkingDir] = currentTerminal()->workingDirectory();
    }

    return properties;
}

void TermWidgetPage::setTerminalOpacity(qreal opacity)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setTermOpacity(opacity);
    }
}

void TermWidgetPage::setColorScheme(const QString &name)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setColorScheme(name);
    }
}

void TermWidgetPage::sendTextToCurrentTerm(const QString &text)
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->sendText(text);
    }
}

void TermWidgetPage::copyClipboard()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->copyClipboard();
    }
}

/*******************************************************************************
 1. @函数: void TermWidgetPage::pasteClipboard()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明: 用户粘贴时候，直接粘贴剪切板内容
*******************************************************************************/
void TermWidgetPage::pasteClipboard()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->pasteClipboard();
    }
}

void TermWidgetPage::toggleShowSearchBar()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->toggleShowSearchBar();
    }
}

void TermWidgetPage::zoomInCurrentTierminal()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->zoomIn();
    }
}

void TermWidgetPage::zoomOutCurrentTerminal()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->zoomOut();
    }
}

/*******************************************************************************
 1. @函数:   void TermWidgetPage::setFontSize(int fontSize)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:    设置字体大小
*******************************************************************************/
void TermWidgetPage::setFontSize(int fontSize)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setTermFontSize(fontSize);
    }
}

/*******************************************************************************
 1. @函数:   void TermWidgetPage::setFont(QString fontName)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:    设置字体
*******************************************************************************/
void TermWidgetPage::setFont(QString fontName)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setTermFont(fontName);
    }
}

/*******************************************************************************
 1. @函数:   void TermWidgetPage::selectAll()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:    全选
*******************************************************************************/
void TermWidgetPage::selectAll()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->setSelectionAll();
    }
}

/*******************************************************************************
 1. @函数:   void TermWidgetPage::skipToNextCommand()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:    跳转到下一命令
*******************************************************************************/
void TermWidgetPage::skipToNextCommand()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->skipToNextCommand();
    }
}

/*******************************************************************************
 1. @函数:   void TermWidgetPage::skipToPreCommand()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:    跳转到前一命令
*******************************************************************************/
void TermWidgetPage::skipToPreCommand()
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->skipToPreCommand();
    }
}

/*******************************************************************************
 1. @函数:   void TermWidgetPage::setcursorShape()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:    设置鼠标形状
*******************************************************************************/
void TermWidgetPage::setcursorShape(int shape)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setCursorShape(shape);
    }
}

/*******************************************************************************
 1. @函数:    setBlinkingCursor(bool enable)
 2. @作者:    王培利n014361
 3. @日期:    2020-01-10
 4. @说明:    设置鼠标闪烁
*******************************************************************************/
void TermWidgetPage::setBlinkingCursor(bool enable)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setBlinkingCursor(enable);
    }
}

void TermWidgetPage::setPressingScroll(bool enable)
{
    QList<TermWidget *> termList = findChildren<TermWidget *>();
    for (TermWidget *term : termList) {
        term->setPressingScroll(enable);
    }
}
/*******************************************************************************
 1. @函数:    showSearchBar(bool enable)
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-24
 4. @说明:    是否显示搜索框
*******************************************************************************/
void TermWidgetPage::showSearchBar(bool enable)
{
    if (enable) {
        m_findBar->raise();
        m_findBar->show();
        m_findBar->move(width() - 382, 0);
        QTimer::singleShot(10, this, [ = ] { m_findBar->focus(); });
    } else {
        m_findBar->hide();
    }
}

// void TermWidgetPage::setOutputtingScroll(bool enable)
//{
//    QList<TermWidgetWrapper *> termList = findChildren<TermWidgetWrapper *>();
//    for (TermWidgetWrapper *term : termList) {
//        term->setOutputtingScroll(enable);
//    }
//}

void TermWidgetPage::onTermRequestSplit(Qt::Orientation ori)
{
    TermWidget *term = qobject_cast<TermWidget *>(sender());
    if (term) {
        // toggleShowSearchBar();
        split(term, ori);
    }
}

void TermWidgetPage::onTermRequestRenameTab(QString newTabName)
{
    if (newTabName.isEmpty()) {
        setProperty("TAB_CUSTOM_NAME_PROPERTY", false);
        emit termTitleChanged(m_currentTerm->title());
    } else {
        // Mark it as we renamed it.
        setProperty("TAB_CUSTOM_NAME_PROPERTY", true);
        // Yeah, TermWidgetPage doesn't store the tab name, only the tab bar did it.
        emit tabTitleChanged(newTabName);
    }
}

void TermWidgetPage::onTermTitleChanged(QString title) const
{
    TermWidget *term = qobject_cast<TermWidget *>(sender());
    if (m_currentTerm == term) {
        emit termTitleChanged(title);
    }
}

void TermWidgetPage::onTermGetFocus()
{
    TermWidget *term = qobject_cast<TermWidget *>(sender());
    setCurrentTerminal(term);
    m_currentTerm->setFocus(Qt::OtherFocusReason);
    emit termGetFocus();
    m_findBar->hide();
}

void TermWidgetPage::onTermClosed()
{
    TermWidget *w = qobject_cast<TermWidget *>(sender());
    if (!w) {
        qDebug() << "TermWidgetPage::onTermClosed: Unknown object to handle" << w;
        Q_ASSERT(0);
    }
    closeSplit(w);
}
/*******************************************************************************
 1. @函数:    handleFindNext
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-24
 4. @说明:    查找下一个接口
*******************************************************************************/
void TermWidgetPage::handleFindNext()
{
    qDebug() << m_findBar->searchKeytxt();
    setMismatchAlert(false);
    m_currentTerm->search(m_findBar->searchKeytxt(), true, true);
}

/*******************************************************************************
 1. @函数:    handleFindPrev
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-24
 4. @说明:    查找上一个接口
*******************************************************************************/
void TermWidgetPage::handleFindPrev()
{
    setMismatchAlert(false);
    m_currentTerm->search(m_findBar->searchKeytxt(), false, false);
}

void TermWidgetPage::slotFindbarClose()
{
    //    EditWrapper *wrapper = currentWrapper();
    //    if (wrapper->bottomBar()->isHidden())
    //    {
    //        wrapper->bottomBar()->show();
    //    }
}

void TermWidgetPage::handleRemoveSearchKeyword()
{
    //    currentWrapper()->textEditor()->removeKeywords();
}
/*******************************************************************************
 1. @函数:    handleUpdateSearchKeyword
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-24
 4. @说明:    更新key(没有输入回车情况),自动查找接口
*******************************************************************************/
void TermWidgetPage::handleUpdateSearchKeyword(const QString &keyword)
{
    setMismatchAlert(false);
    if (keyword.isEmpty()) {
        m_currentTerm->clearSelection();
    } else {
        // 输入时直接查找，被禁用
        // m_currentTerm->search(m_findBar->SearchKeytxt(), true, false);
    }
}
/*******************************************************************************
 1. @函数:    applyTheme
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-011
 4. @说明:    非DTK控件手动匹配系统主题的修改
*******************************************************************************/
void TermWidgetPage::applyTheme()
{
    updateSplitStyle();
}

void TermWidgetPage::updateSplitStyle()
{
    QList<DSplitter *> splitList = findChildren<DSplitter *>();
    for (DSplitter *splitter : qAsConst(splitList)) {
        setSplitStyle(splitter);
    }
}

void TermWidgetPage::setCurrentTerminal(TermWidget *term)
{
    TermWidget *oldTerm = m_currentTerm;
    m_currentTerm = term;
    if (oldTerm != m_currentTerm) {
        if (m_currentTerm->isTitleChanged()) {
            emit termTitleChanged(m_currentTerm->title());
        } else {
            emit termTitleChanged(windowTitle());
        }
    }
}

TermWidget *TermWidgetPage::createTerm(TermProperties properties)
{
    TermWidget *term = new TermWidget(properties, this);

    connect(term, &TermWidget::termRequestSplit, this, &TermWidgetPage::onTermRequestSplit);
    connect(term, &TermWidget::termRequestRenameTab, this, &TermWidgetPage::onTermRequestRenameTab);
    connect(term, &TermWidget::termRequestOpenSettings, this, &TermWidgetPage::termRequestOpenSettings);
    connect(term, &TermWidget::termTitleChanged, this, &TermWidgetPage::onTermTitleChanged);
    connect(term, &TermWidget::termGetFocus, this, &TermWidgetPage::onTermGetFocus);
    connect(term, &TermWidget::finished, this, &TermWidgetPage::onTermClosed);
    connect(term, &TermWidget::termRequestUploadFile, this, &TermWidgetPage::termRequestUploadFile);
    connect(term, &TermWidget::termRequestDownloadFile, this, &TermWidgetPage::termRequestDownloadFile);
    return term;
}

void TermWidgetPage::setTextCodec(QTextCodec *codec)
{
    TermWidget *term = currentTerminal();
    if (term) {
        term->setTextCodec(codec);
    }
}
/*******************************************************************************
 1. @函数:    setMismatchAlert
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-24
 4. @说明:    是否将输入框标为警告
*******************************************************************************/
void TermWidgetPage::setMismatchAlert(bool alert)
{
    m_findBar->setNoMatchAlert(alert);
}
/*******************************************************************************
 1. @函数:    resizeEvent
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-24
 4. @说明:    重绘时候，调整查找框的位置
*******************************************************************************/
void TermWidgetPage::resizeEvent(QResizeEvent *event)
{
    qDebug() << "resizeEvent" << x() << y();
    this->m_findBar->move(width() - 382, 0);
}
