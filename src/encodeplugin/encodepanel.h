// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENCODEPANEL_H
#define ENCODEPANEL_H

#include "rightpanel.h"

/*******************************************************************************
 1. @类名:    EncodePanel
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class EncodeListView;
class EncodePanel : public RightPanel
{
    Q_OBJECT
public:
    explicit EncodePanel(QWidget *parent = nullptr);
    /******** Add by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 Begin***************/
    /**
     * @brief 处理显示(设置m_encodeView的大小)
     * @author nt001000 renfeixiang
     */
    void show();
    /******** Add by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 End***************/
    /******** Modify by ut000610 daizhengwen 2020-05-29: 根据焦点变换更新编码****************/
    /**
     * @brief 更新编码
     * @author ut001121 zhangmeng
     * @param encode 编码
     */
    void updateEncode(QString encode);
    /********************* Modify by ut000610 daizhengwen End ************************/

signals:
private:
    EncodeListView *m_encodeView = nullptr;
};

#endif  // THEMEPANEL_H
