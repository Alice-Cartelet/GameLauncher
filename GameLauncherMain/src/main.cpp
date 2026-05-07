#include <QApplication>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("GameLauncher");
    app.setApplicationVersion("2.0.1");
    app.setOrganizationName("AliceCartelet");    // 公司/组织名称
    app.setOrganizationDomain("nuist.com.cn");   // 组织域名
    QFont font("Segoe UI", 10);
    font.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(font);
    app.setStyleSheet(R"(
        QToolTip {
            background: #1a1f2e;
            color: rgba(255,255,255,0.85);
            border: 1px solid rgba(255,255,255,0.12);
            border-radius: 6px;
            padding: 6px 10px;
            font-family: 'Segoe UI';
            font-size: 12px;
        }
    )");
    MainWindow w;
    w.show();
    return app.exec();
}
