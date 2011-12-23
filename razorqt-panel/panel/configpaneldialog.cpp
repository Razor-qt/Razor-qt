/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL3+
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
 * version 3 of the License, or (at your option) any later version.
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


#include "configpaneldialog.h"
#include "ui_configpaneldialog.h"
#include "razorpanel.h"

#include <QtGui/QColorDialog>
#include <QtGui/QFileDialog>

#define CFG_PANEL_GROUP             "panel"
#define CFG_KEY_HEIGHT              "height"
#define CFG_KEY_WIDTH               "width"
#define CFG_KEY_WIDTH_TYPE          "widthType"
#define CFG_KEY_ALIGNMENT           "alignment"
#define CFG_KEY_BACKGROUND          "background"
#define CFG_KEY_BACKGROUND_TYPE     "backgroundType"
//#define CFG_KEY_TRANSPERENT         "transperent"

ConfigPanelDialog::ConfigPanelDialog(int hDefault, int wMax, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigPanelDialog)
{
    ui->setupUi(this);
    ui->horizontalSlider_transperent->setVisible(false);
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(saveSettings()));
    connect(ui->spinBox_height, SIGNAL(valueChanged(int)),this, SLOT(spinBoxHeightValueChanged(int)));
    connect(ui->comboBox_widthType, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxWidthTypeIndexChanged(int)));
    connect(ui->comboBox_alignment, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxAlignmentIndexChanged(int)));
    connect(ui->spinBox_width, SIGNAL(valueChanged(int)),this, SLOT(spinBoxWidthValueChanged(int)));
    connect(ui->radioButton_image, SIGNAL(toggled(bool)), this, SLOT(radioButtonImageToggled(bool)));
    connect(ui->radioButton_system, SIGNAL(toggled(bool)), this, SLOT(radioButtonSystemToggled(bool)));
    connect(ui->radioButton_color, SIGNAL(toggled(bool)), this, SLOT(radioButtonColorToggled(bool)));
    connect(ui->pushButton_color, SIGNAL(clicked()), this, SLOT(pushButtonColorClicked()));
    connect(ui->pushButton_image, SIGNAL(clicked()), this, SLOT(pushButtonImageClicked()));
    //connect(ui->horizontalSlider_transperent, SIGNAL(valueChanged(int)), this, SLOT(SliderTransperentValueChanged(int)));
    mHeightDefault=hDefault;
    mWidthMax=wMax;

    // FIXME
    // I think, need use something like this
    // int mHeight = settings().value(CFG_KEY_HEIGHT, 37).toInt();

    mConfigFile = "panel";
    if (qApp->arguments().count() > 1)
    {
        mConfigFile = qApp->arguments().at(1);
        if (mConfigFile.endsWith(".conf"))
            mConfigFile.chop(5);
    }

    mSettings = new RazorSettings("razor-panel/" + mConfigFile, this);
    mSettings->beginGroup(CFG_PANEL_GROUP);
    mHeight = mSettings->value(CFG_KEY_HEIGHT, mHeightDefault).toInt();
    QString mWidthTypeStr = mSettings->value(CFG_KEY_WIDTH_TYPE, 0).toString();
    QString mAlignmentStr = mSettings->value(CFG_KEY_ALIGNMENT, 2).toString();
    QString mBackgroundTypeStr = mSettings->value(CFG_KEY_BACKGROUND_TYPE, 0).toString();
    //mTransperent = mSettings->value(CFG_KEY_TRANSPERENT, 0).toInt();

    mAlignment = strToAlignment(mAlignmentStr);
    mWidthType = strToWidthType(mWidthTypeStr);
    mBackgroundType=strToBackgroundType(mBackgroundTypeStr);

    if (mWidthType==0)      // size in percents
    {
        mWidth = mSettings->value(CFG_KEY_WIDTH, 100).toInt();
        ui->spinBox_width->setMaximum(100);
    }
    else                    // size in pixels
    {
        ui->spinBox_width->setMaximum(mWidthMax);
        mWidth = mSettings->value(CFG_KEY_WIDTH, mWidthMax).toInt();
    }

    switch (mBackgroundType)
    {
        case 0:
            ui->radioButton_system->setChecked(true);
            break;
        case 1:
            ui->radioButton_color->setChecked(true);
            mColor = mSettings->value(CFG_KEY_BACKGROUND, "#ffffff").toString();
            ui->pushButton_color->setPalette(QPalette(mColor));
            ui->pushButton_color->setAutoFillBackground(true);
            break;
        case 2:
            ui->radioButton_image->setChecked(true);
            mImagePath=mSettings->value(CFG_KEY_BACKGROUND, "none").toString();
            QString mImageRelativePath=mImagePath;
            mImageRelativePath = mImageRelativePath.remove(mImageRelativePath.indexOf("/"), mImageRelativePath.lastIndexOf("/")+1);           // for ui->button
            ui->pushButton_image->setText(mImageRelativePath);
            break;
    }
    mSettings->endGroup();


    ui->spinBox_height->setValue(mHeight);
    ui->comboBox_widthType->setCurrentIndex(mWidthType);
    ui->spinBox_width->setValue(mWidth);
    ui->comboBox_alignment->setCurrentIndex(mAlignment);
    //ui->horizontalSlider_transperent->setValue(mTransperent);

}

ConfigPanelDialog::~ConfigPanelDialog()
{
    delete ui;
}

void ConfigPanelDialog::saveSettings()
{
    mSettings->beginGroup(CFG_PANEL_GROUP);
    mSettings->setValue(CFG_KEY_WIDTH, mWidth);
    mSettings->setValue(CFG_KEY_WIDTH_TYPE, widthTypeToStr(mWidthType));
    mSettings->setValue(CFG_KEY_HEIGHT, mHeight);
    mSettings->setValue(CFG_KEY_ALIGNMENT, alignmentToStr(mAlignment));
    mSettings->setValue(CFG_KEY_BACKGROUND_TYPE, backgroundTypeToStr(mBackgroundType));
    //mSettings->setValue(CFG_KEY_TRANSPERENT,mTransperent);

    switch (mBackgroundType)
    {
        case 0: mSettings->setValue(CFG_KEY_BACKGROUND, "default"); break;
        case 1: mSettings->setValue(CFG_KEY_BACKGROUND, mColor.name()); break;
        case 2: mSettings->setValue(CFG_KEY_BACKGROUND, mImagePath); break;
    }
    mSettings->endGroup();

    RazorPanel *parent = qobject_cast<RazorPanel*>(this->parentWidget());
    parent->show();
    this->close();
}

void ConfigPanelDialog::spinBoxWidthValueChanged(int value)
{
    mWidth=value;
    // if panel width not max, user can plased it on left/rigth/center
    if ((mWidthType==0 && ui->spinBox_width->value()<100) || (mWidthType==1 && ui->spinBox_width->value()<mWidthMax))
        ui->comboBox_alignment->setEnabled(true);
    else
       ui->comboBox_alignment->setEnabled(false);
}

void ConfigPanelDialog::comboBoxWidthTypeIndexChanged(int index)
{
    mWidthType=index;
    if (mWidthType==0)  // %
        //ui->spinBox_width->setValue(mWidth*100/mWidthMax);        // px to %
        ui->spinBox_width->setMaximum(100);
    else                // px
        //ui->spinBox_width->setValue((mWidth*mWidthMax)/100);        // % to px
        ui->spinBox_width->setMaximum(mWidthMax);
}

void ConfigPanelDialog::comboBoxAlignmentIndexChanged(int index)
{
    mAlignment=index;
}

void ConfigPanelDialog::spinBoxHeightValueChanged(int value)
{
    mHeight=value;
}

void ConfigPanelDialog::radioButtonImageToggled(bool state)
{
    mBackgroundType=2;
    ui->pushButton_image->setEnabled(state);
}

void ConfigPanelDialog::radioButtonColorToggled(bool state)
{
    mBackgroundType=1;
    //ui->horizontalSlider_transperent->setEnabled(state);
    ui->pushButton_color->setEnabled(state);
}

void ConfigPanelDialog::radioButtonSystemToggled(bool state)
{
    mBackgroundType=0;
}

void ConfigPanelDialog::pushButtonColorClicked()
{
    mColor = QColorDialog::getColor(Qt::white, this);
    if (mColor.isValid())
    {
        ui->pushButton_color->setPalette(QPalette(mColor));
        ui->pushButton_color->setAutoFillBackground(true);
    }
}

void ConfigPanelDialog::pushButtonImageClicked()
{
    mImagePath = QFileDialog::getOpenFileName(this,tr("Choose the file"),"~", tr("Images (*.png *.jpg)"));
    QString mImageRelativePath=mImagePath;
    mImageRelativePath = mImageRelativePath.remove(mImageRelativePath.indexOf("/"), mImageRelativePath.lastIndexOf("/")+1);           // for ui->button
    ui->pushButton_image->setText(mImageRelativePath);
}

//void ConfigPanelDialog::SliderTransperentValueChanged(int value)
//{
//    mTransperent=value;
//}


/************************************************
 ************************************************/
QString ConfigPanelDialog::alignmentToStr(int align)
{
    if (align==0)   return QString("Left");
    if (align==1)   return QString("Right");
    if (align==2)   return QString("Center");
    return QString("Center");
}

int ConfigPanelDialog::strToAlignment(const QString& str)
{
    if (str.toUpper() == "LEFT")    return 0;
    if (str.toUpper() == "RIGHT")   return 1;
    if (str.toUpper() == "CENTER")  return 2;
    return 0;
}

/************************************************
 ************************************************/
QString ConfigPanelDialog::widthTypeToStr(int type)
{
    if (type==0)   return QString("percent");
    else           return QString("pixel");
    return QString("percent");
}

int ConfigPanelDialog::strToWidthType(const QString& str)
{
    if (str.toUpper() == "PERCENT")     return 0;
    if (str.toUpper() == "PIXEL")       return 1;
    return 0;
}

/************************************************
 ************************************************/
QString ConfigPanelDialog::backgroundTypeToStr(int type)
{
    if (type==0)   return QString("System");
    if (type==1)   return QString("Color");
    if (type==2)   return QString("Image");
    return QString("System");
}

int ConfigPanelDialog::strToBackgroundType(const QString& str)
{
    if (str.toUpper() == "SYSTEM")      return 0;
    if (str.toUpper() == "COLOR")       return 1;
    if (str.toUpper() == "IMAGE")       return 2;
    return 0;
}
