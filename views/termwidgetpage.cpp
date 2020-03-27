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
    setProperty("TAB_IDENTIFIER_PROPERTY", QUuid::createUuid().toString());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    DSplitter *splitter = new DSplitter(Qt::Horizontal, this);
    splitter->setFocusPolicy(Qt::NoFocus);
    setSplitStyle(splitter);

    // Init find bar.
    m_findBar->move(this->x() - 100, this->y() - 100);
    connect(m_findBar, &PageSearchBar::findNext, this, &TermWidgetPage::handleFindNext);
    connect(m_findBar, &PageSearchBar::findPrev, this, &TermWidgetPage::handleFindPrev);
    connect(
    m_findBar, &PageSearchBar::keywordChanged, this, [ = ](QString keyword) { handleUpdateSearchKeyword(keyword); });
    // connect(m_findBar, &PageSearchBar::sigFindbarClose, this, &TermWidgetPage::slotFindbarClose,
    // Qt::QueuedConnection);
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, [ = ]() { applyTheme(); });

    TermWidgetWrapper *w = createTerm(properties);
    splitter->addWidget(w);

    layout->addWidget(splitter);

    m_currentTerm = w;

    setLayout(layout);
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

TermWidgetWrapper *TermWidgetPage::currentTerminal()
{
    return m_currentTerm;
}

TermWidgetWrapper *TermWidgetPage::split(Qt::Orientation orientation)
{
    return split(currentTerminal(), orientation);
}

TermWidgetWrapper *TermWidgetPage::split(TermWidgetWrapper *term, Qt::Orientation orientation)
{
    QSplitter *parent = qobject_cast<QSplitter *>(term->parent());
    Q_CHECK_PTR(parent);

    int index = parent->indexOf(term);
    QList<int> parentSizes = parent->sizes();

    TermProperties properties(term->workingDirectory());
    // term->toggleShowSearchBar();
    // term->update();

    TermWidgetWrapper *w = createTerm(properties);

    if (1 == parent->count()) {
        parent->insertWidget(0, term);
        parent->insertWidget(1, w);
        parent->setSizes({ 1, 1 });

        setSplitStyle(parent);

    } else {

        DSplitter *subSplit = new DSplitter(orientation, this);
        subSplit->setFocusPolicy(Qt::NoFocus);
        subSplit->insertWidget(0, term);
        subSplit->insertWidget(1, w);
        subSplit->setSizes({ 1, 1 });

        setSplitStyle(subSplit);

        parent->insertWidget(index, subSplit);
        parent->setSizes(parentSizes);

        setSplitStyle(parent);
    }

    w->setFocus(Qt::OtherFocusReason);
    // w->toggleShowSearchBar();
    // this->repaint();
    return w;
}

void TermWidgetPage::closeSplit(TermWidgetWrapper *term)
{
    if (term->hasRunningProcess() && !Utils::showExitConfirmDialog()) {
        return;
    }

    qDebug() << "TermWidgetPage::closeSplit:" << term->getCurrSessionId();
    QSplitter *parent = qobject_cast<QSplitter *>(term->parent());
    Q_CHECK_PTR(parent);

    term->setParent(nullptr);
    term->deleteLater();

    QWidget *nextFocus = nullptr;

    // Collapse splitters containing a single element, excluding the top one.
    if (parent->count() == 1) {
        QSplitter *uselessSplitterParent = qobject_cast<QSplitter *>(parent->parent());
        if (uselessSplitterParent != nullptr) {
            int index = uselessSplitterParent->indexOf(parent);
            Q_ASSERT(index != -1);
            QWidget *singleHeir = parent->widget(0);
            uselessSplitterParent->insertWidget(index, singleHeir);
            if (qobject_cast<TermWidgetWrapper *>(singleHeir)) {
                nextFocus = singleHeir;
            } else {
                nextFocus = singleHeir->findChild<TermWidgetWrapper *>();
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
    QList<TermWidgetWrapper *> termList = findChildren<TermWidgetWrapper *>();

    int currSessionId = m_currentTerm->getCurrSessionId();
    //exit protection
    for (TermWidgetWrapper *term : qAsConst(termList)) {
        if ((term->getCurrSessionId() != currSessionId) && term->hasRunningProcess() && !Utils::showExitConfirmDialog()) {
            qDebug() << "here are processes running in this terminal split... " << endl;
            return;
        }
    }

    for (TermWidgetWrapper *term : qAsConst(termList)) {
        if (term->getCurrSessionId() != currSessionId) {
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

// 待删除
static CoordinateRect getNormalizedCoordinateRect(QWidget *w, Qt::Edge navigationDirection)
{
    CoordinateRect nd;
    nd.topLeft = w->mapTo(w->window(), QPoint(0, 0));
    nd.middle = w->mapTo(w->window(), QPoint(w->width() / 2, w->height() / 2));
    nd.bottomRight = w->mapTo(w->window(), QPoint(w->width(), w->height()));
    normalizeToRight(&nd, navigationDirection);
    return nd;
}

/******** Modify by n014361 wangpeili 2020-01-07:  修改了计算上下左右判断的方法 ********×****/
void TermWidgetPage::focusNavigation(Qt::Edge dir)
{
    QPoint comparPoint = GetComparePoint(currentTerminal(), dir);
    TermWidgetWrapper *dst = nullptr;
    QList<TermWidgetWrapper *> termList = findChildren<TermWidgetWrapper *>();
    QMap<TermWidgetWrapper *, QRect> mapTermRect;
    for (TermWidgetWrapper *term : qAsConst(termList)) {
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
    return findChildren<TermWidgetWrapper *>().size();
}
/********************* Modify by n014361 wangpeili End ************************/

/*******************************************************************************
 1. @函数:  QRect TermWidgetPage::GetRect(TermWidgetWrapper *term)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:  获取标签页内所有子窗体的区域
*******************************************************************************/
QRect TermWidgetPage::GetRect(TermWidgetWrapper *term)
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
QPoint TermWidgetPage::GetComparePoint(TermWidgetWrapper *term, Qt::Edge dir)
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

TermProperties TermWidgetPage::createCurrentTerminalProperties()
{
    TermProperties properties;

    TermWidgetWrapper *term = currentTerminal();
    if (term) {
        properties[WorkingDir] = currentTerminal()->workingDirectory();
    }

    return properties;
}

void TermWidgetPage::setTerminalOpacity(qreal opacity)
{
    QList<TermWidgetWrapper *> termList = findChildren<TermWidgetWrapper *>();
    for (TermWidgetWrapper *term : termList) {
        term->setTerminalOpacity(opacity);
    }
}

void TermWidgetPage::setColorScheme(const QString &name)
{
    QList<TermWidgetWrapper *> termList = findChildren<TermWidgetWrapper *>();
    for (TermWidgetWrapper *term : termList) {
        term->setColorScheme(name);
    }
}

void TermWidgetPage::sendTextToCurrentTerm(const QString &text)
{
    TermWidgetWrapper *term = currentTerminal();
    if (term) {
        term->sendText(text);
    }
}

void TermWidgetPage::copyClipboard()
{
    TermWidgetWrapper *term = currentTerminal();
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
    TermWidgetWrapper *term = currentTerminal();
    if (term) {
        term->pasteClipboard();
    }
}

void TermWidgetPage::toggleShowSearchBar()
{
    TermWidgetWrapper *term = currentTerminal();
    if (term) {
        term->toggleShowSearchBar();
    }
}

void TermWidgetPage::zoomInCurrentTierminal()
{
    TermWidgetWrapper *term = currentTerminal();
    if (term) {
        term->zoomIn();
    }
}

void TermWidgetPage::zoomOutCurrentTerminal()
{
    TermWidgetWrapper *term = currentTerminal();
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
    QList<TermWidgetWrapper *> termList = findChildren<TermWidgetWrapper *>();
    for (TermWidgetWrapper *term : termList) {
        term->setTerminalFontSize(fontSize);
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
    QList<TermWidgetWrapper *> termList = findChildren<TermWidgetWrapper *>();
    for (TermWidgetWrapper *term : termList) {
        term->setTerminalFont(fontName);
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
    TermWidgetWrapper *term = currentTerminal();
    if (term) {
        term->selectAll();
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
    TermWidgetWrapper *term = currentTerminal();
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
    TermWidgetWrapper *term = currentTerminal();
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
    QList<TermWidgetWrapper *> termList = findChildren<TermWidgetWrapper *>();
    for (TermWidgetWrapper *term : termList) {
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
    QList<TermWidgetWrapper *> termList = findChildren<TermWidgetWrapper *>();
    for (TermWidgetWrapper *term : termList) {
        term->setCursorBlinking(enable);
    }
}

void TermWidgetPage::setPressingScroll(bool enable)
{
    QList<TermWidgetWrapper *> termList = findChildren<TermWidgetWrapper *>();
    for (TermWidgetWrapper *term : termList) {
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
    TermWidgetWrapper *term = qobject_cast<TermWidgetWrapper *>(sender());
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
    TermWidgetWrapper *term = qobject_cast<TermWidgetWrapper *>(sender());
    if (m_currentTerm == term) {
        emit termTitleChanged(title);
    }
}

void TermWidgetPage::onTermGetFocus()
{
    TermWidgetWrapper *term = qobject_cast<TermWidgetWrapper *>(sender());
    setCurrentTerminal(term);
    m_currentTerm->setFocus(Qt::OtherFocusReason);
    emit termGetFocus();
    m_findBar->hide();
}

void TermWidgetPage::onTermClosed()
{
    TermWidgetWrapper *w = qobject_cast<TermWidgetWrapper *>(sender());
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

void TermWidgetPage::setCurrentTerminal(TermWidgetWrapper *term)
{
    TermWidgetWrapper *oldTerm = m_currentTerm;
    m_currentTerm = term;
    if (oldTerm != m_currentTerm) {
        if (m_currentTerm->isTitleChanged()) {
            emit termTitleChanged(m_currentTerm->title());
        } else {
            emit termTitleChanged(windowTitle());
        }
    }
}

TermWidgetWrapper *TermWidgetPage::createTerm(TermProperties properties)
{
    TermWidgetWrapper *term = new TermWidgetWrapper(properties, this);

    connect(term, &TermWidgetWrapper::termRequestSplit, this, &TermWidgetPage::onTermRequestSplit);
    connect(term, &TermWidgetWrapper::termRequestRenameTab, this, &TermWidgetPage::onTermRequestRenameTab);
    connect(term, &TermWidgetWrapper::termRequestOpenSettings, this, &TermWidgetPage::termRequestOpenSettings);
    connect(term, &TermWidgetWrapper::termTitleChanged, this, &TermWidgetPage::onTermTitleChanged);
    connect(term, &TermWidgetWrapper::termGetFocus, this, &TermWidgetPage::onTermGetFocus);
    connect(term, &TermWidgetWrapper::termClosed, this, &TermWidgetPage::onTermClosed);
    connect(term, &TermWidgetWrapper::termRequestUploadFile, this, &TermWidgetPage::termRequestUploadFile);
    connect(term, &TermWidgetWrapper::termRequestDownloadFile, this, &TermWidgetPage::termRequestDownloadFile);
    return term;
}

void TermWidgetPage::setTextCodec(QTextCodec *codec)
{
    TermWidgetWrapper *term = currentTerminal();
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
