#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QTimer>
#include <QDateTime>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QCoreApplication>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QDir>
#include <QPainter>
#include <QSettings>

// Custom widget for a smooth line indicator
class LineIndicator : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int percentage READ percentage WRITE setPercentage)

public:
    explicit LineIndicator(QWidget *parent = nullptr) : QWidget(parent), m_percentage(0) {
        setFixedHeight(2); // Set the desired thickness of the line
    }

    int percentage() const { return m_percentage; }
    void setPercentage(int percentage) {
        m_percentage = qBound(0, percentage, 100);
        update(); // Trigger a repaint
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // Draw background track
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#555"));
        painter.drawRect(rect());

        // Draw progress
        int progressWidth = width() * m_percentage / 100;
        painter.setBrush(QColor("#cadf9e"));
        painter.drawRect(0, 0, progressWidth, height());
    }

private:
    int m_percentage;
};

// Function to load stops from a CSV file
QVector<QString> loadStopsFromFile(const QString& filePath) {
    QVector<QString> stops;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open stops file:" << filePath;
        return {}; // Return empty vector on failure
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.trimmed().isEmpty()) {
            stops.append(line.trimmed());
        }
    }
    file.close();
    return stops;
}

// Function to load ad images from a directory
QVector<QPixmap> loadAds(const QString& path) {
    QVector<QPixmap> ads;
    QDir dir(path);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg";
    dir.setNameFilters(filters);
    for (const QFileInfo &fileInfo : dir.entryInfoList()) {
        QPixmap pixmap(fileInfo.absoluteFilePath());
        if (!pixmap.isNull()) {
            ads.append(pixmap);
        }
    }
    return ads;
}

// A helper function to create a styled box for visualization
QFrame* createStyledBox(const QString& title) {
    QFrame* box = new QFrame();
    box->setFrameShape(QFrame::StyledPanel);
    box->setFrameShadow(QFrame::Raised);
    QVBoxLayout* layout = new QVBoxLayout(box);
    QLabel* titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 16pt; color: grey;");
    layout->addWidget(titleLabel);
    layout->addStretch(); // Pushes content to the top
    return box;
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // --- Load Configuration ---
    QString configPath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(configPath, QSettings::IniFormat);
    QString busNumber = settings.value("BusInfo/BusNumber", "123").toString();
    QString routeName = settings.value("BusInfo/RouteName", "Downtown - Uptown").toString();
    QString weather = settings.value("Display/Weather", "☀️ 25°C").toString();

    // Main window
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Bus Info Board");
    mainWindow.setStyleSheet("QMainWindow { background-color: #222; color: #EEE; }");

    // Central widget to hold the main layout
    QWidget *centralWidget = new QWidget();
    mainWindow.setCentralWidget(centralWidget);

    // Main vertical layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(20, 10, 20, 10);

    // 1. Header section
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QFrame* busNameBox = new QFrame();
    busNameBox->setStyleSheet("background-color: darkgreen; border-radius: 10px;");
    QHBoxLayout* busNameLayout = new QHBoxLayout(busNameBox);
    QLabel *busNameLabel = new QLabel(busNumber);
    busNameLabel->setStyleSheet("font-size: 40pt; font-weight: bold; padding: 5px;");
    busNameLayout->addWidget(busNameLabel);
    QLabel *routeNameLabel = new QLabel(routeName);
    routeNameLabel->setStyleSheet("font-size: 28pt; font-style: italic;");
    QLabel *clockLabel = new QLabel();
    clockLabel->setStyleSheet("font-size: 28pt; font-weight: bold;");
    QLabel *weatherLabel = new QLabel(weather);
    weatherLabel->setStyleSheet("font-size: 28pt;");
    headerLayout->addWidget(busNameBox);
    headerLayout->addSpacing(30);
    headerLayout->addWidget(routeNameLabel, 1);
    headerLayout->addStretch(2);
    headerLayout->addWidget(weatherLabel);
    headerLayout->addSpacing(30);
    headerLayout->addWidget(clockLabel);

    // --- Data Sources ---
    QString stopsPath = QCoreApplication::applicationDirPath() + "/stops.csv";
    QVector<QString> stopList = loadStopsFromFile(stopsPath);
    int currentStopIndex = 0;
    QString adsPath = QCoreApplication::applicationDirPath() + "/ads";
    QVector<QPixmap> adImages = loadAds(adsPath);
    int currentAdIndex = 0;

    // 2. Main content section
    QHBoxLayout *contentLayout = new QHBoxLayout();
    QFrame *infoBox = createStyledBox("Route Information");
    QVBoxLayout* infoLayout = qobject_cast<QVBoxLayout*>(infoBox->layout());
    infoLayout->takeAt(1);

    // Ad Area Widget
    QWidget* adAreaWidget = new QWidget();
    QVBoxLayout* adAreaLayout = new QVBoxLayout(adAreaWidget);
    adAreaLayout->setContentsMargins(0,0,0,0);
    adAreaLayout->setSpacing(5);
    QLabel* adLabel = new QLabel();
    adLabel->setAlignment(Qt::AlignCenter);
    adLabel->setStyleSheet("background-color: #333;");
    adLabel->setGraphicsEffect(new QGraphicsOpacityEffect(adLabel));
    LineIndicator* adProgressBar = new LineIndicator();
    adProgressBar->setVisible(false);
    adAreaLayout->addWidget(adLabel, 1);
    adAreaLayout->addWidget(adProgressBar);

    contentLayout->addWidget(infoBox, 3);
    contentLayout->addWidget(adAreaWidget, 5);

    // 3. Footer section
    QLabel* nextStopLabel = nullptr;
    QFrame *footerBox = createStyledBox("Upcoming Stations");
    QVBoxLayout* footerMainLayout = qobject_cast<QVBoxLayout*>(footerBox->layout());
    footerMainLayout->takeAt(1);
    QFrame* nextStopBox = new QFrame();
    nextStopBox->setStyleSheet("background-color: #444; border-radius: 10px;");
    QHBoxLayout* nextStopLayout = new QHBoxLayout(nextStopBox);
    nextStopLayout->addStretch();
    nextStopLabel = new QLabel();
    nextStopLabel->setStyleSheet("font-size: 28pt; font-weight: bold; padding: 20px;");
    nextStopLayout->addWidget(nextStopLabel);
    nextStopLayout->addStretch();
    footerMainLayout->addWidget(nextStopBox);

    // --- Stop Display Setup ---
    QVector<QWidget*> animatedStopWidgets;
    QVector<QLabel*> upcomingStopLabels;
    LineIndicator* stopsProgressBar = new LineIndicator();
    stopsProgressBar->setVisible(false);
    if (infoLayout) {
        infoLayout->addStretch();
        for (int i = 4; i >= 1; --i) {
            QFrame* stopFrame = new QFrame();
            stopFrame->setStyleSheet("background-color: #214539; border-radius: 5px;");
            QHBoxLayout* stopLayout = new QHBoxLayout(stopFrame);
            QLabel* stopLabel = new QLabel();
            stopLabel->setStyleSheet("font-size: 24pt; color: #EEE; padding: 10px;");
            stopLabel->setAlignment(Qt::AlignCenter);
            stopLayout->addWidget(stopLabel);
            infoLayout->addWidget(stopFrame);
            upcomingStopLabels.prepend(stopLabel);
            animatedStopWidgets.append(stopFrame);
            if (i > 1) {
                QLabel* arrowLabel = new QLabel("↑");
                arrowLabel->setStyleSheet("font-size: 20pt; color: #AAA;");
                arrowLabel->setAlignment(Qt::AlignCenter);
                infoLayout->addWidget(arrowLabel);
                animatedStopWidgets.append(arrowLabel);
            }
        }
        infoLayout->addSpacing(10);
        infoLayout->addWidget(stopsProgressBar);
        for (QWidget* w : animatedStopWidgets) {
            w->setGraphicsEffect(new QGraphicsOpacityEffect(w));
        }
    }

    auto updateStopDisplay = [&]() {
        if (stopList.isEmpty()) return;
        nextStopLabel->setText(stopList[currentStopIndex]);
        for (int i = 0; i < upcomingStopLabels.size(); ++i) {
            int stopIndex = (currentStopIndex + i + 1) % stopList.size();
            upcomingStopLabels[i]->setText(stopList[stopIndex]);
        }
    };

    if (infoLayout && nextStopLabel) {
        if (stopList.isEmpty()) {
            QLabel* errorLabel = new QLabel("Could not load stops.csv");
            errorLabel->setStyleSheet("font-size: 24pt; color: red;");
            errorLabel->setAlignment(Qt::AlignCenter);
            infoLayout->addWidget(errorLabel);
            nextStopLabel->setText("---");
        } else {
            updateStopDisplay();
        }
    }
    
    // --- Ad Display Setup ---
    auto updateAdDisplay = [&]() {
        if (adImages.isEmpty()) {
            adLabel->setText("Your Ad Here");
            return;
        }
        const QPixmap& pixmap = adImages[currentAdIndex];
        adLabel->setPixmap(pixmap.scaled(adLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    };
    updateAdDisplay();

    // Add all sections to the main layout
    mainLayout->addLayout(headerLayout, 1);
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("background-color: #cadf9e; min-height: 3px;");
    mainLayout->addWidget(line);
    mainLayout->addLayout(contentLayout, 5);
    mainLayout->addWidget(footerBox, 2);

    // --- Timers & Animations ---
    if (!stopList.isEmpty() && stopList.size() > 1) {
        stopsProgressBar->setVisible(true);
        QTimer *stopsTimer = new QTimer();
        QObject::connect(stopsTimer, &QTimer::timeout, [&]() {
            auto* fadeOutGroup = new QParallelAnimationGroup(&mainWindow);
            for (QWidget* w : animatedStopWidgets) {
                auto* anim = new QPropertyAnimation(w->graphicsEffect(), "opacity");
                anim->setDuration(400);
                anim->setStartValue(1.0);
                anim->setEndValue(0.0);
                fadeOutGroup->addAnimation(anim);
            }
            QObject::connect(fadeOutGroup, &QParallelAnimationGroup::finished, [&]() {
                currentStopIndex = (currentStopIndex + 1) % stopList.size();
                updateStopDisplay();
                auto* fadeInGroup = new QParallelAnimationGroup(&mainWindow);
                for (QWidget* w : animatedStopWidgets) {
                    auto* anim = new QPropertyAnimation(w->graphicsEffect(), "opacity");
                    anim->setDuration(400);
                    anim->setStartValue(0.0);
                    anim->setEndValue(1.0);
                    fadeInGroup->addAnimation(anim);
                }
                fadeInGroup->start(QAbstractAnimation::DeleteWhenStopped);
            });
            fadeOutGroup->start(QAbstractAnimation::DeleteWhenStopped);
        });
        stopsTimer->start(10000);

        auto* stopsProgressAnim = new QPropertyAnimation(stopsProgressBar, "percentage", &mainWindow);
        stopsProgressAnim->setDuration(10000);
        stopsProgressAnim->setStartValue(0);
        stopsProgressAnim->setEndValue(100);
        stopsProgressAnim->setLoopCount(-1);
        stopsProgressAnim->start();
    }

    if (!adImages.isEmpty() && adImages.size() > 1) {
        adProgressBar->setVisible(true);
        QTimer *adTimer = new QTimer();
        QObject::connect(adTimer, &QTimer::timeout, [&]() {
            auto* anim = new QPropertyAnimation(adLabel->graphicsEffect(), "opacity");
            anim->setDuration(500);
            anim->setStartValue(1.0);
            anim->setEndValue(0.0);
            QObject::connect(anim, &QPropertyAnimation::finished, [&]() {
                currentAdIndex = (currentAdIndex + 1) % adImages.size();
                updateAdDisplay();
                auto* fadeInAnim = new QPropertyAnimation(adLabel->graphicsEffect(), "opacity");
                fadeInAnim->setDuration(500);
                fadeInAnim->setStartValue(0.0);
                fadeInAnim->setEndValue(1.0);
                fadeInAnim->start(QAbstractAnimation::DeleteWhenStopped);
            });
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        });
        adTimer->start(15000);

        auto* adProgressAnim = new QPropertyAnimation(adProgressBar, "percentage", &mainWindow);
        adProgressAnim->setDuration(15000);
        adProgressAnim->setStartValue(0);
        adProgressAnim->setEndValue(100);
        adProgressAnim->setLoopCount(-1);
        adProgressAnim->start();
    }

    QTimer *clockTimer = new QTimer();
    QObject::connect(clockTimer, &QTimer::timeout, [=]() {
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        clockLabel->setText(currentTime);
    });
    clockTimer->start(1000);

    // Show the window in full screen
    mainWindow.showFullScreen();

    return app.exec();
}

#include "main.moc"
