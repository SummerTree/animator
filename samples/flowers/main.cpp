#include <qapplication.h>
#include <qqmlcontext.h>
#include <qquickview.h>
#include <qqmlapplicationengine.h>
#include <qtranslator.h>
#include <QFile>
#include <QMessageBox>

#include "bindings/application.h"

#include "views/main_dock.h"
#include "views/splash_screen.h"

int main(int argc, char *argv[])
{
#ifdef _WINDOWS_
	::SetConsoleOutputCP(CP_UTF8);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif

	QFile styleSheet(":res/qss/default.qss");

	if (styleSheet.open(QIODevice::ReadOnly))
	{
		QApplication app(argc, argv);
		app.setStyleSheet(styleSheet.readAll());

		// Load translation files
		QString local = QLocale::languageToString(QLocale::system().language());

		QTranslator qtTranslator;
		if (local == "Chinese")
			qtTranslator.load("zh_CN.qm", ":res/languages/");
		else if (local == "English")
			qtTranslator.load("en_US.qm", ":res/languages/");
		else
			qtTranslator.load("en_US.qm", ":res/languages/");

		app.installTranslator(&qtTranslator);

		auto splash = std::make_unique<flower::SplashScreen>();
		splash->show();
		app.processEvents();

		flower::MainDock w(splash.get());
		w.show();

		splash.reset();

		app.exec();
}
	else
	{
		qWarning("Can't open the style sheet file.");
		return 0;
	}
}