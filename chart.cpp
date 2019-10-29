/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "chart.h"
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QValueAxis>
#include <QtCore/QRandomGenerator>
#include <QtCore/QDebug>
#include <QTime>

Chart::Chart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    QChart(QChart::ChartTypeCartesian, parent, wFlags),
    m_x(0),
    m_y{0},
    m_series{nullptr}
{
    legend()->hide();

    setMargins(QMargins(0, 0, 0, 0));
    setAnimationOptions(QChart::GridAxisAnimations);
}

Chart::~Chart()
{

}

void Chart::addChartSeries(int _series, QRgb _color, QString name)
{
    if (nullptr != m_series[_series])
    {
        return;
    }

    QPen m_pen;
    switch (_color) {
    case Qt::red:
        m_pen.setColor(Qt::red);
        break;

    case Qt::blue:
        m_pen.setColor(Qt::blue);
        break;

    case Qt::green:
        m_pen.setColor(Qt::green);
        break;

    default:
        m_pen.setColor(_color);
        break;
    }

    m_series[_series] = new QLineSeries(this);
    m_pen.setWidth(2);
    m_series[_series]->setPen(m_pen);
    m_series[_series]->append(m_x, m_y[0]);

    addSeries(m_series[_series]);
    m_series[_series]->attachAxis(m_axisX);
    m_series[_series]->attachAxis(m_axisY);

    m_series[_series]->setName(name);
}


void Chart::setAxis(int m_xAxisMax,
                    int m_xAxisMin,
                    int m_yAxisMax,
                    int m_yAxisMin,
                    int m_xAxisStep,
                    int m_yAxisStep)
{
    xAxisMax = m_xAxisMax;
    xAxisMin = m_xAxisMin;
    yAxisMax = m_yAxisMax;
    yAxisMin = m_yAxisMin;
    xAxisStep = m_xAxisStep;
    yAxisStep = m_yAxisStep;

    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();

    addAxis(m_axisX,Qt::AlignBottom);
    addAxis(m_axisY,Qt::AlignLeft);
    m_axisX->setRange(static_cast<qreal>(xAxisMin), static_cast<qreal>(xAxisMax));
    m_axisX->setTickCount((xAxisMax - xAxisMin) / xAxisStep + 1);
    m_axisX->setLabelFormat("%d");
//    m_axisX->setTitleText("Time(secs)");
    m_axisY->setRange(static_cast<qreal>(yAxisMin), static_cast<qreal>(yAxisMax));
    m_axisY->setTickCount((yAxisMax - yAxisMin) / yAxisStep + 1);
    m_axisY->setLabelFormat("%d");
//    m_axisY->setTitleText("Yaw(degrees)");
}

void Chart::seriesAppendData(qreal _m_y[], qreal _d_x)
{
    m_x += _d_x;
    for (int i = 0; i < seriesCount; i++)
    {
        if (nullptr != m_series[i])
        {
            m_series[i]->append(m_x, _m_y[i]);
        }
    }


    if (m_x > m_axisX->max())
    {
        scroll(plotArea().width() / (m_axisX->tickCount() - 1), 0);
    }
}

void Chart::stringToHtml(QString &str, QColor _color)
{
    // html filter
    str.replace("&","&amp;");
    str.replace(">","&gt;");
    str.replace("<","&lt;");
    str.replace("\"","&quot;");
    str.replace("\'","&#39;");
    str.replace(" ","&nbsp;");
    str.replace("\n","<br>");
    str.replace("\r","<br>");

    QByteArray array;
    array.append(static_cast<char>(_color.red()));
    array.append(static_cast<char>(_color.green()));
    array.append(static_cast<char>(_color.blue()));
    QString strC(array.toHex());

    str = QString("<span style=\" color:#%1;\">%2</span>").arg(strC).arg(str);
}

YawRollChart::YawRollChart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    Chart(parent, wFlags)
{
    setAxis(70, 0, 180, -180, 10, 90);

    addChartSeries(0, Qt::red, QString("Yaw"));
    addChartSeries(1, Qt::green, QString("Roll"));

    legendText[0] = new QGraphicsTextItem();
    QString str = "Yaw/deg.";
    stringToHtml(str, Qt::red);
    legendText[0]->setHtml(str);
    legendText[0]->setPos(QPointF (50, 15));

    legendText[1] = new QGraphicsTextItem();
    str = "Roll/deg.";
    stringToHtml(str, Qt::green);
    legendText[1]->setHtml(str);
    legendText[1]->setPos(QPointF (50, 30));
}

YawRollChart::~YawRollChart()
{

}


PitchChart::PitchChart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    Chart(parent, wFlags)
{
    setAxis(70, 0, 90, -90, 10, 45);

    addChartSeries(0, Qt::blue, QString("Pitch"));

    legendText[0] = new QGraphicsTextItem();
    QString str = "Pitch/deg.";
    stringToHtml(str, Qt::blue);
    legendText[0]->setHtml(str);
    legendText[0]->setPos(QPointF (40, 15));
}

PitchChart::~PitchChart()
{

}


DepthChart::DepthChart(QGraphicsItem *parent, Qt::WindowFlags wFlags):
    Chart(parent, wFlags)
{
    setAxis(70, 0, 1, -3, 10, 1);

    addChartSeries(0, Qt::blue, QString("Depth"));


    legendText[0] = new QGraphicsTextItem();
    QString str = "Depth/m";
    stringToHtml(str, Qt::blue);
    legendText[0]->setHtml(str);
    legendText[0]->setPos(QPointF (35, 15));
}

DepthChart::~DepthChart()
{

}
