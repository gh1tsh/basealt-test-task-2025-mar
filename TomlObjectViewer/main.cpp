#include "MainWindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int
main(int argc, char *argv[])
{
        QApplication a(argc, argv);

        QTranslator translator;

        QLocale           locale             = QLocale::system();
        QString           languageAndCountry = locale.name();
        QString           languageCode       = languageAndCountry.split('_').first();
        const QStringList uiLanguages        = locale.uiLanguages();
        for (const QString &locale : uiLanguages) {
                const QString baseName = "TomlObjectViewer_" + QLocale(locale).name();
                if (translator.load(":/i18n/" + baseName)) {
                        a.installTranslator(&translator);
                        break;
                }
        }

        MainWindow w;
        w.show();

        return a.exec();
}
