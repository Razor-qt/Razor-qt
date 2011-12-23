/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Marat "Morion" Talipov <morion-self@mail.ru>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "razorpanelplugin_p.h"
#include <razorqt/razorplugininfo.h>

#ifndef CONFIGPANELDIALOG_H
#define CONFIGPANELDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigPanelDialog;
}

class ConfigPanelDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConfigPanelDialog(int hDefault, int wMax, QWidget *parent = 0);
    ~ConfigPanelDialog();

public slots:
    void saveSettings();
    void spinBoxHeightValueChanged(int value);
    void spinBoxWidthValueChanged(int value);
    void comboBoxWidthTypeIndexChanged(int index);
    void comboBoxAlignmentIndexChanged(int index);
    void radioButtonColorToggled(bool state);
    void radioButtonImageToggled(bool state);
    void radioButtonSystemToggled(bool state);
    void pushButtonColorClicked();
    void pushButtonImageClicked();
    //void SliderTransperentValueChanged(int value);
    
private:
    Ui::ConfigPanelDialog *ui;
    QString mConfigFile;
    RazorSettings* mSettings;
    int mHeightDefault;
    int mHeight;
    int mWidthMax;
    int mWidthType;
    int mWidth;
    int mAlignment;
    int mBackgroundType;
    int mTransperent;
    QString mImagePath;
    QColor mColor;
    int strToAlignment(const QString& str);
    QString alignmentToStr(int align);
    QString widthTypeToStr(int type);
    int strToWidthType(const QString& str);
    QString backgroundTypeToStr(int type);
    int strToBackgroundType(const QString& str);

};

#endif // CONFIGPANELDIALOG_H
