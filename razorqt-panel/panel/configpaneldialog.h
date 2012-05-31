/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Marat "Morion" Talipov <morion.self@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#ifndef CONFIGPANELDIALOG_H
#define CONFIGPANELDIALOG_H

#include "razorpanel.h"
#include <razorqt/razorsettings.h>

#define CFG_PANEL_GROUP     "panel"

#define CFG_KEY_SCREENNUM   "desktop"
#define CFG_KEY_POSITION    "position"
#define CFG_KEY_PLUGINS     "plugins"
#define CFG_KEY_HEIGHT      "height"
#define CFG_KEY_WIDTH       "width"
#define CFG_KEY_PERCENT     "width-percent"
#define CFG_KEY_ALIGNMENT   "alignment"
#define CFG_KEY_THEMESIZE   "theme-size"
#define CFG_KEY_OPACITY        "opacity"

#define CFG_FULLKEY_PLUGINS "panel/plugins"

struct RazorPanelConfigData {
    bool percent;
    bool useThemeSize;
    int height;
    int width;
    double opacity;
    RazorPanel::Alignment alignment;
};

namespace Ui {
class ConfigPanelDialog;
}

class ConfigPanelDialog : public QWidget
{
    Q_OBJECT
    
public:
    explicit ConfigPanelDialog(int hDefault, int wMax, RazorSettings *settings, QWidget *parent = 0);
    ~ConfigPanelDialog();

signals:
    void configChanged(RazorPanelConfigData data);
    void positionChanged(int screen, RazorPanel::Position);

public slots:
    void reset();
    void save();

private slots:
    void spinBoxHeightValueChanged(int q);
    void spinBoxWidthValueChanged(int q);
    void spinBoxOpacityValueChanged(double q);
    void comboBoxWidthTypeIndexChanged(int q);
    void comboBoxAlignmentIndexChanged(int q);
    void comboBoxPositionIndexChanged(int q);
    void checkBoxUseThemeSizeChanged(bool state);

private:
    Ui::ConfigPanelDialog *ui;
    QString mConfigFile;
    RazorSettings* mSettings;
    int mSizeDefault;
    int mLengthMax;
    int mScreenNum;
    RazorPanel::Position mPosition;
    RazorPanelConfigData mConfigData;

    void addPosition(const QString& name, int screen, RazorPanel::Position position);
};

#endif // CONFIGPANELDIALOG_H
