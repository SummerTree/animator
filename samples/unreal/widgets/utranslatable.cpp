#include "utranslatable.h"

namespace unreal
{
	UTranslatable::UTranslatable()
	{
	}
	UTranslatable::~UTranslatable()
	{
	}
    void UTranslatable::retranslate()
    {
    }

    bool
	UTranslatable::eventFilter(QObject* watched, QEvent* event)
	{
		if (event->type() == QEvent::LanguageChange)
			retranslate();

        return true;
    }
}
