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

#ifndef CHART_H
#define CHART_H

#include <QtCharts/QChart>
#include <QtCore/QTimer>
#include <QtCharts/QLineSeries>

QT_CHARTS_BEGIN_NAMESPACE
class QSplineSeries;
class QValueAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE


class Chart: public QChart
{
    Q_OBJECT
public:
    Chart(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = nullptr);
    virtual ~Chart();
    void seriesAppendData(qreal _m_y[], qreal _d_x);


protected:
    void setAxis(int m_xAxisMax,
                 int m_xAxisMin,
                 int m_yAxisMax,
                 int m_yAxisMin,
                 int m_xAxisStep,
                 int m_yAxisStep);
    void addChartSeries(int _series, QRgb _color, QString name);

    void stringToHtml(QString &str, QColor _color);

private:
    const static int seriesCount = 3;
    qreal m_x;
    qreal m_y[seriesCount];
    int xAxisMax;
    int xAxisMin;
    int yAxisMax;
    int yAxisMin;
    int xAxisStep;
    int yAxisStep;
    QLineSeries *m_series[seriesCount];
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;

public:
    QGraphicsTextItem *legendText[seriesCount];
};

class YawRollChart: public Chart
{
    Q_OBJECT
public:
    YawRollChart(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = nullptr);
    virtual ~YawRollChart();
};

class PitchChart: public Chart
{
    Q_OBJECT
public:
    PitchChart(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = nullptr);
    virtual ~PitchChart();
};

class DepthChart: public  Chart
{
    Q_OBJECT
public:
    DepthChart(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = nullptr);
    virtual ~DepthChart();
};

#endif /* CHART_H */
