#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QTimer>
#include <QMouseEvent>
#include <QStack>
#include <QQueue>
#define lekh qDebug() <<
#define Delay delay(1)
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->workArea->setMouseTracking(true);
    ui->workArea->installEventFilter(this);

    QPixmap canvas = ui->workArea->pixmap(Qt::ReturnByValue);
    if (canvas.isNull())
    {
        canvas = QPixmap(ui->workArea->size());
        canvas.fill(Qt::white);
        ui->workArea->setPixmap(canvas);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::delay(int ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

void MainWindow::colorPoint(int x, int y, int r, int g, int b, int penwidth = 1)
{
    QPixmap canvas = ui->workArea->pixmap();
    QPainter painter(&canvas);
    QPen pen = QPen(QColor(r, g, b), penwidth);
    painter.setPen(pen);
    painter.drawPoint(x, y);
    ui->workArea->setPixmap(canvas);
}

void MainWindow::on_showAxis_clicked()
{
    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    int centerX = width / 2;
    int centerY = height / 2;

    // Draw horizontal axis (y = 0)
    for (int x = 0; x < width; ++x)
        colorPoint(x, centerY - gridOffset / 2.0, 255, 0, 0, gridOffset);

    // Draw vertical axis (x = 0)
    for (int y = 0; y < height; ++y)
        colorPoint(centerX + gridOffset / 2.0, y, 255, 0, 0, gridOffset);
}

void MainWindow::on_gridlines_clicked()
{
    int gridOffset = ui->gridOffset->value();
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    if (gridOffset <= 0)
        return;

    int centerX = width / 2;
    int centerY = height / 2;

    QPixmap canvas = ui->workArea->pixmap();
    QPainter painter(&canvas);
    painter.setPen(QColor(0, 0, 0));

    for (int i = 0; (centerX + i < width && centerX - i > 0) || (centerY + i < height && centerY - i > 0); i += gridOffset)
    {
        // Vertical lines
        if (centerX + i < width)
            painter.drawLine(QPoint(centerX + i, 0), QPoint(centerX + i, height));
        if (centerX - i > 0)
            painter.drawLine(QPoint(centerX - i, 0), QPoint(centerX - i, height));

        // Horizontal lines
        if (centerY + i < height)
            painter.drawLine(QPoint(0, centerY + i), QPoint(width, centerY + i));
        if (centerY - i > 0)
            painter.drawLine(QPoint(0, centerY - i), QPoint(width, centerY - i));
    }

    painter.end();

    ui->workArea->setPixmap(canvas);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->workArea)
    {

        if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent *cursor = static_cast<QMouseEvent *>(event);
            int x = cursor->pos().x();
            int y = cursor->pos().y();
            int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
            int width = ui->workArea->width();
            int height = ui->workArea->height();
            int centerX = width / 2;
            int centerY = height / 2;

            int gridX = floor((x - centerX) * 1.0 / gridOffset);
            int gridY = floor((centerY - y) * 1.0 / gridOffset);

            ui->x_coordinate->setText(QString::number(gridX));
            ui->y_coordinate->setText(QString::number(gridY));
            return true;
        }

        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *cursor = static_cast<QMouseEvent *>(event);
            int x = cursor->pos().x();
            int y = cursor->pos().y();
            int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
            int width = ui->workArea->width();
            int height = ui->workArea->height();
            int centerX = width / 2;
            int centerY = height / 2;

            int gridX = floor((x - centerX) * 1.0 / gridOffset);
            int gridY = floor((centerY - y) * 1.0 / gridOffset);

            clickedPoints.push_back({gridX, gridY});

            int calcX = centerX + gridX * gridOffset + gridOffset / 2;
            int calcY = centerY - gridY * gridOffset - gridOffset / 2;

            colorPoint(calcX, calcY, 0, 0, 255, gridOffset);

            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::draw_DDA_Line(float x1, float y1, float x2, float y2, int r, int g, int b)
{
    float dx, dy, xinc, yinc, steps;

    dx = x2 - x1;
    dy = y2 - y1;
    steps = std::max(abs(dx), abs(dy));

    xinc = dx / steps;
    yinc = dy / steps;

    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    int centerX = width / 2;
    int centerY = height / 2;

    float x_float = centerX + x1 * gridOffset + gridOffset / 2.0;
    float y_float = centerY - y1 * gridOffset - gridOffset / 2.0;

    int xn = static_cast<int>(x_float);
    int yn = static_cast<int>(y_float);

    QVector<QPoint> pts;
    pts.push_back(QPoint(xn, yn));

    for (int i = 0; i < steps; i++)
    {
        x_float += xinc * gridOffset;
        y_float -= yinc * gridOffset;
        int x_new = static_cast<int>(x_float);
        int y_new = static_cast<int>(y_float);

        if (x_new != xn || y_new != yn)
        {
            xn = x_new;
            yn = y_new;
            int X = floor((xn - centerX) * 1.0 / gridOffset);
            int Y = floor((centerY - yn) * 1.0 / gridOffset);
            int calcX = centerX + X * gridOffset + gridOffset / 2;
            int calcY = centerY - Y * gridOffset - gridOffset / 2;
            pts.push_back(QPoint(calcX, calcY));
        }
    }

    for (const auto &pt : pts)
    {
        colorPoint(pt.x(), pt.y(), r, g, b, gridOffset);
        Delay;
    }
}

void MainWindow::on_DDA_Button_clicked()
{
    if (clickedPoints.size() < 2)
        return;
    qint64 n = clickedPoints.size();
    auto coords1 = clickedPoints[n - 1];
    auto coords2 = clickedPoints[n - 2];
    QElapsedTimer timer;
    timer.start();
    draw_DDA_Line(coords1.x(), coords1.y(), coords2.x(), coords2.y(), 0, 255, 0);
    qint64 elapsedTime = timer.nsecsElapsed();
    ui->DDA_Time->setText(QString("Time Taken: ") + QString::number(elapsedTime) + QString(" ns"));
}

QSet<QPoint> MainWindow::make_Bresenham_Line(int x1, int y1, int x2, int y2)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    int centerX = width / 2;
    int centerY = height / 2;

    QSet<QPoint> pts;

    while (true)
    {
        int calcX = centerX + x1 * gridOffset + gridOffset / 2;
        int calcY = centerY - y1 * gridOffset - gridOffset / 2;
        pts.insert(QPoint(calcX, calcY));

        if (x1 == x2 && y1 == y2)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }

    return pts;
}

void MainWindow::draw_Bresenham_Line(int x1, int y1, int x2, int y2, int r, int g, int b)
{
    // Call make_Bresenham_Line to get the set of points that make the line
    QSet<QPoint> pts = make_Bresenham_Line(x1, y1, x2, y2);

    // Get the grid offset for coloring
    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();

    // Iterate over the points and color them
    for (const QPoint &pt : pts)
    {
        colorPoint(pt.x(), pt.y(), r, g, b, gridOffset);
        Delay;
    }
}

void MainWindow::on_Bresenham_Button_clicked()
{
    if (clickedPoints.size() < 2)
        return;
    qint64 n = clickedPoints.size();
    auto coords1 = clickedPoints[n - 1];
    auto coords2 = clickedPoints[n - 2];
    QElapsedTimer timer;
    timer.start();
    draw_Bresenham_Line(coords1.x(), coords1.y(), coords2.x(), coords2.y(), 125, 0, 255);
    qint64 elapsedTime = timer.nsecsElapsed();
    ui->Bresenham_Time->setText(QString("Time Taken: ") + QString::number(elapsedTime) + QString(" ns"));
}

void MainWindow::drawCirclePoints(int x, int y, int pixelCenterX, int pixelCenterY, int r, int g, int b, int gridOffset)
{

    QSet<QPair<int, int>> uniquePoints;

    uniquePoints.insert(qMakePair(pixelCenterX + x * gridOffset, pixelCenterY - y * gridOffset));
    uniquePoints.insert(qMakePair(pixelCenterX - x * gridOffset, pixelCenterY - y * gridOffset));
    uniquePoints.insert(qMakePair(pixelCenterX + x * gridOffset, pixelCenterY + y * gridOffset));
    uniquePoints.insert(qMakePair(pixelCenterX - x * gridOffset, pixelCenterY + y * gridOffset));
    uniquePoints.insert(qMakePair(pixelCenterX + y * gridOffset, pixelCenterY - x * gridOffset));
    uniquePoints.insert(qMakePair(pixelCenterX - y * gridOffset, pixelCenterY - x * gridOffset));
    uniquePoints.insert(qMakePair(pixelCenterX + y * gridOffset, pixelCenterY + x * gridOffset));
    uniquePoints.insert(qMakePair(pixelCenterX - y * gridOffset, pixelCenterY + x * gridOffset));

    for (const auto &point : uniquePoints)
    {
        colorPoint(point.first, point.second, r, g, b, gridOffset);
        Delay;
    }
}

void MainWindow::on_Polar_Circle_Button_clicked()
{
    QElapsedTimer timer;
    timer.start();

    int radius = ui->Circle_Radius->value();
    radius++;

    if (clickedPoints.empty())
        return;
    int gridCenterX = clickedPoints.back().x();
    int gridCenterY = clickedPoints.back().y();

    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    int centerX = width / 2;
    int centerY = height / 2;

    int pixelCenterX = centerX + gridCenterX * gridOffset + gridOffset / 2;
    int pixelCenterY = centerY - gridCenterY * gridOffset - gridOffset / 2;

    QSet<QPair<int, int>> uniquePoints;

    for (double theta = 0.01; theta <= 45.0; theta += 0.1)
    {
        double rad = qDegreesToRadians(theta);
        int x = static_cast<int>(radius * cos(rad));
        int y = static_cast<int>(radius * sin(rad));
        uniquePoints.insert(qMakePair(x, y));
    }

    for (const auto &point : uniquePoints)
    {
        int x = point.first;
        int y = point.second;
        drawCirclePoints(x, y, pixelCenterX, pixelCenterY, 0, 0, 0, gridOffset);
    }

    qint64 elapsed = timer.elapsed();
    ui->Polar_Circle_Time->setText(QString::number(elapsed) + " ms");
}

void MainWindow::on_Bresenham_Circle_Button_clicked()
{
    // Start the timer
    QElapsedTimer timer;
    timer.start();

    // Get the radius from the Circle_Radius QSpinBox
    int radius = ui->Circle_Radius->value();

    // Get the center of the circle from the last clicked point
    if (clickedPoints.empty())
        return;
    int gridCenterX = clickedPoints.back().x();
    int gridCenterY = clickedPoints.back().y();

    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    int centerX = width / 2;
    int centerY = height / 2;

    // Convert grid center to actual pixel coordinates
    int pixelCenterX = centerX + gridCenterX * gridOffset + gridOffset / 2;
    int pixelCenterY = centerY - gridCenterY * gridOffset - gridOffset / 2;

    // Use QSet to store unique points
    QSet<QPair<int, int>> uniquePoints;

    // Bresenham's Circle Algorithm
    int x = 0, y = radius;
    int d = 3 - 2 * radius;

    while (y >= x)
    {
        uniquePoints.insert(qMakePair(x, y));
        x++;
        if (d > 0)
        {
            y--;
            d = d + 4 * (x - y) + 10;
        }
        else
        {
            d = d + 4 * x + 6;
        }
        uniquePoints.insert(qMakePair(x, y));
    }

    // Draw the unique points using 8-point symmetry
    for (const auto &point : uniquePoints)
    {
        int x = point.first;
        int y = point.second;
        drawCirclePoints(x, y, pixelCenterX, pixelCenterY, 0, 255, 255, gridOffset);
    }

    qint64 elapsed = timer.elapsed();

    ui->Bresenham_Circle_Time->setText(QString::number(elapsed) + " ms");
}

void MainWindow::on_Cartesian_Circle_Button_clicked()
{
    QElapsedTimer timer;
    timer.start();

    int radius = ui->Circle_Radius->value();

    if (clickedPoints.empty())
        return;
    int gridCenterX = clickedPoints.back().x();
    int gridCenterY = clickedPoints.back().y();

    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    int centerX = width / 2;
    int centerY = height / 2;
    int pixelCenterX = centerX + gridCenterX * gridOffset + gridOffset / 2;
    int pixelCenterY = centerY - gridCenterY * gridOffset - gridOffset / 2;

    QSet<QPair<int, int>> uniquePoints;

    for (int x = 0; x <= radius; x++)
    {
        int y = static_cast<int>(sqrt(radius * radius - x * x));
        uniquePoints.insert(qMakePair(x, y));
    }

    for (const auto &point : uniquePoints)
    {
        int x = point.first;
        int y = point.second;
        drawCirclePoints(x, y, pixelCenterX, pixelCenterY, 255, 0, 255, gridOffset);
    }

    qint64 elapsed = timer.elapsed();

    ui->Cartesian_Circle_Time->setText(QString::number(elapsed) + " ms");
}

void MainWindow::on_Reset_Screen_Button_clicked()
{
    QPixmap clearPixmap(ui->workArea->size());
    clearPixmap.fill(Qt::white);
    ui->workArea->setPixmap(clearPixmap);

    clickedPoints.clear();

    ui->x_coordinate->clear();
    ui->y_coordinate->clear();

    ui->Polar_Circle_Time->clear();
    ui->Bresenham_Circle_Time->clear();
    ui->Cartesian_Circle_Time->clear();

    ui->Circle_Radius->setValue(0);

    ui->workArea->update();
}

void MainWindow::drawEllipsePoints(int x, int y, int pixelCenterX, int pixelCenterY, int r, int g, int b, int gridOffset)
{

    colorPoint(pixelCenterX + x * gridOffset, pixelCenterY - y * gridOffset, r, g, b, gridOffset);
    Delay;
    colorPoint(pixelCenterX - x * gridOffset, pixelCenterY - y * gridOffset, r, g, b, gridOffset);
    Delay;
    colorPoint(pixelCenterX + x * gridOffset, pixelCenterY + y * gridOffset, r, g, b, gridOffset);
    Delay;
    colorPoint(pixelCenterX - x * gridOffset, pixelCenterY + y * gridOffset, r, g, b, gridOffset);
    Delay;
}

void MainWindow::on_Polar_Ellipse_Button_clicked()
{
    QElapsedTimer timer;
    timer.start();

    int axisA = ui->ellipse_axis_1->value();
    int axisB = ui->ellipse_axis_2->value();
    axisA++;
    axisB++;

    if (clickedPoints.empty())
        return;
    int gridCenterX = clickedPoints.back().x();
    int gridCenterY = clickedPoints.back().y();

    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    int centerX = width / 2;
    int centerY = height / 2;

    int pixelCenterX = centerX + gridCenterX * gridOffset + gridOffset / 2;
    int pixelCenterY = centerY - gridCenterY * gridOffset - gridOffset / 2;

    QSet<QPair<int, int>> ellipsePoints;

    for (double theta = 0.01; theta <= 90.0; theta += 0.1)
    {
        double rad = qDegreesToRadians(theta);
        int x = static_cast<int>(axisA * cos(rad));
        int y = static_cast<int>(axisB * sin(rad));

        ellipsePoints.insert({x, y});
    }

    for (const auto &point : ellipsePoints)
    {
        drawEllipsePoints(point.first, point.second, pixelCenterX, pixelCenterY, 85, 85, 85, gridOffset);
    }

    qint64 elapsed = timer.elapsed();
    ui->Polar_Ellipse_Time->setText(QString::number(elapsed) + " ms");
}

void MainWindow::on_Bresenham_Ellipse_Button_clicked()
{
    QElapsedTimer timer;
    timer.start();

    int a = ui->ellipse_axis_1->value();
    int b = ui->ellipse_axis_2->value();

    if (clickedPoints.empty())
        return;
    int gridCenterX = clickedPoints.back().x();
    int gridCenterY = clickedPoints.back().y();

    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    int centerX = width / 2;
    int centerY = height / 2;

    int pixelCenterX = centerX + gridCenterX * gridOffset + gridOffset / 2;
    int pixelCenterY = centerY - gridCenterY * gridOffset - gridOffset / 2;

    QSet<QPair<int, int>> ellipsePoints;

    int x = 0, y = b;
    long long a2 = a * a;
    long long b2 = b * b;
    long long d1 = b2 - (a2 * b) + (0.25 * a2);
    long long dx = 2 * b2 * x;
    long long dy = 2 * a2 * y;

    while (dx < dy)
    {
        ellipsePoints.insert({x, y});
        if (d1 < 0)
        {
            x++;
            dx = dx + 2 * b2;
            d1 = d1 + dx + b2;
        }
        else
        {
            x++;
            y--;
            dx = dx + 2 * b2;
            dy = dy - 2 * a2;
            d1 = d1 + dx - dy + b2;
        }
    }

    long long d2 = b2 * (x + 0.5) * (x + 0.5) + a2 * (y - 1) * (y - 1) - a2 * b2;
    while (y >= 0)
    {
        ellipsePoints.insert({x, y});
        if (d2 > 0)
        {
            y--;
            dy = dy - 2 * a2;
            d2 = d2 + a2 - dy;
        }
        else
        {
            y--;
            x++;
            dx = dx + 2 * b2;
            dy = dy - 2 * a2;
            d2 = d2 + dx - dy + a2;
        }
    }

    for (const QPair<int, int> &p : ellipsePoints)
    {
        drawEllipsePoints(p.first, p.second, pixelCenterX, pixelCenterY, 170, 170, 170, gridOffset);
    }

    qint64 elapsed = timer.elapsed();
    ui->Bresenham_Ellipse_Time->setText(QString::number(elapsed) + " ms");
}

QSet<QPoint> MainWindow::make_polygon(int n)
{
    allPolygonPoints.clear();
    QSet<QPoint> distinctPoints;
    QList<QPoint> orderedPoints;

    // Gather last 'n' distinct points from clickedPoints
    for (auto it = clickedPoints.rbegin(); it != clickedPoints.rend() && distinctPoints.size() < n; ++it)
    {
        if (!distinctPoints.contains(*it))
        {
            distinctPoints.insert(*it);
            orderedPoints.push_front(*it); // Keep the points in reverse order
        }
    }

    // If there are not enough distinct points, return an empty set
    if (distinctPoints.size() < n)
    {
        return QSet<QPoint>();
    }

    // Iterate through the points to create the polygon lines
    for (int i = 0; i < orderedPoints.size(); ++i)
    {
        QPoint p1 = orderedPoints[i];
        QPoint p2 = orderedPoints[(i + 1) % orderedPoints.size()]; // Connect last point to the first one
        QSet<QPoint> linePoints = make_Bresenham_Line(p1.x(), p1.y(), p2.x(), p2.y());
        allPolygonPoints.unite(linePoints); // Add the line points to the overall polygon points
    }

    return allPolygonPoints;
}

void MainWindow::on_Polygon_Button_clicked()
{
    QElapsedTimer timer;
    timer.start();

    int n = ui->Polygon_Side_Count->value();

    // Get the set of points that make the polygon
    QSet<QPoint> polygonPoints = make_polygon(n);

    // If the set is empty, it means we don't have enough points
    if (polygonPoints.isEmpty())
    {
        ui->Polygon_Label->setText("Not enough points");
        return;
    }

    // Color the polygon points
    int r = 255, g = 165, b = 0; // Color Orange
    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();

    for (const QPoint &pt : polygonPoints)
    {
        colorPoint(pt.x(), pt.y(), r, g, b, gridOffset); // Simply color the points
        Delay;
    }

    // Display the elapsed time in ms
    qint64 elapsed = timer.elapsed();
    ui->Polygon_Label->setText(QString::number(elapsed) + " ms");
}

void MainWindow::on_Polygon_Scanline_Fill_clicked()
{
}

void MainWindow::on_Flood_Fill_clicked()
{
    QElapsedTimer timer;
    timer.start();

    int r = ui->Seed_Color_R->value();
    int g = ui->Seed_Color_G->value();
    int b = ui->Seed_Color_B->value();
    int c = ui->Connected->value();
    if (c != 4 && c != 8)
    {
        ui->Flood_Fill_Time->setText("Wrong connectedness");
        return;
    }
    if (clickedPoints.empty())
    {
        ui->Flood_Fill_Time->setText("Not enough points");
        return;
    }
    QPoint seedPoint = clickedPoints.back();

    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    int centerX = width / 2;
    int centerY = height / 2;

    QStack<QPoint> stack;
    stack.push(seedPoint);
    // lekh stack;
    QSet<QPoint> visited = allPolygonPoints;
    // lekh allPolygonPoints;
    while (!stack.isEmpty())
    {
        QPoint point = stack.pop();
        int x = point.x();
        int y = point.y();
        int pixelX = centerX + x * gridOffset + gridOffset / 2;
        int pixelY = centerY - y * gridOffset - gridOffset / 2;

        if (pixelX < 0 || pixelY < 0 || pixelX >= width || pixelY >= height)
        {
            continue;
        }
        if (!visited.contains({pixelX, pixelY}))
        {

            colorPoint(pixelX, pixelY, r, g, b, gridOffset);
            visited.insert({pixelX, pixelY});
            Delay;

            stack.push(QPoint(x + 1, y));
            stack.push(QPoint(x - 1, y));
            stack.push(QPoint(x, y + 1));
            stack.push(QPoint(x, y - 1));
            if (c == 8)
            {
                stack.push(QPoint(x + 1, y + 1));
                stack.push(QPoint(x - 1, y - 1));
                stack.push(QPoint(x - 1, y + 1));
                stack.push(QPoint(x + 1, y - 1));
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    ui->Flood_Fill_Time->setText(QString::number(elapsed) + " ms");
}

QColor MainWindow::getPixelColor(int x, int y)
{
    QImage image = ui->workArea->pixmap(Qt::ReturnByValue).toImage();
    return image.pixelColor(x, y);
}

void MainWindow::on_Boundary_Fill_clicked()
{
    QElapsedTimer timer;
    timer.start();

    int r = ui->Seed_Color_R->value();
    int g = ui->Seed_Color_G->value();
    int b = ui->Seed_Color_B->value();
    int c = ui->Connected->value();
    if (c != 4 && c != 8)
    {
        ui->Flood_Fill_Time->setText("Wrong connectedness");
        return;
    }
    if (clickedPoints.empty())
    {
        ui->Boundary_Fill_Time->setText("Not enough points");
        return;
    }

    QPoint seedPoint = clickedPoints.back();

    int gridOffset = (ui->gridOffset->value() == 0) ? 1 : ui->gridOffset->value();
    int width = ui->workArea->width();
    int height = ui->workArea->height();
    int centerX = width / 2;
    int centerY = height / 2;

    QColor boundaryColor(255, 165, 0);
    QSet<QPoint> visited;
    QQueue<QPoint> queue;
    queue.enqueue(seedPoint);

    while (!queue.isEmpty())
    {
        QPoint point = queue.dequeue();
        int x = point.x();
        int y = point.y();
        int pixelX = centerX + x * gridOffset + gridOffset / 2;
        int pixelY = centerY - y * gridOffset - gridOffset / 2;

        if (pixelX < 0 || pixelY < 0 || pixelX >= width || pixelY >= height)
        {
            continue;
        }
        QColor currentColor = getPixelColor(pixelX, pixelY);

        if (currentColor == QColor(r, g, b) || currentColor == boundaryColor)
        {
            continue;
        }
        colorPoint(pixelX, pixelY, r, g, b, gridOffset);
        visited.insert(QPoint(x, y));
        Delay;

        // Enqueue neighbors
        queue.enqueue(QPoint(x + 1, y));
        queue.enqueue(QPoint(x - 1, y));
        queue.enqueue(QPoint(x, y + 1));
        queue.enqueue(QPoint(x, y - 1));
        if (c == 8)
        {
            queue.enqueue(QPoint(x + 1, y + 1));
            queue.enqueue(QPoint(x - 1, y - 1));
            queue.enqueue(QPoint(x - 1, y + 1));
            queue.enqueue(QPoint(x + 1, y - 1));
        }
    }

    qint64 elapsed = timer.elapsed();
    ui->Boundary_Fill_Time->setText(QString::number(elapsed) + " ms");
}

// colors used:(255, 0, 0), (0, 255, 0), (0, 0, 255), (0, 0, 0), (255, 255, 255), (125, 0, 255), (0, 255, 255), (255, 0, 255), (85, 85, 85), (170, 170, 170) (265, 165, 0)