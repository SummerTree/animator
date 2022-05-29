#ifndef UNREAL_MODEL_DOCK_H_
#define UNREAL_MODEL_DOCK_H_

#include <qdockwidget.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qlistwidget.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <QVBoxLayout>

#include "unreal_profile.h"
#include "unreal_behaviour.h"

namespace unreal
{
	class ModelDock final : public QDockWidget
	{
		Q_OBJECT
	public:
		ModelDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept(false);
		~ModelDock() noexcept;

		void resizeEvent(QResizeEvent* e) noexcept override;
		void showEvent(QShowEvent* event) noexcept override;
		void keyPressEvent(QKeyEvent* event) noexcept override;
		bool eventFilter(QObject* watched, QEvent* event);

	public Q_SLOTS:
		void importClickEvent();

		void itemClicked(QListWidgetItem* item);
		void itemSelected(QListWidgetItem* item);

	private:
		void addItem(std::string_view uuid) noexcept;

	public:
		QListWidget* listWidget_;
		QWidget* mainWidget_;
		QVBoxLayout* mainLayout_;

		QHBoxLayout* topLayout_;
		QHBoxLayout* bottomLayout_;

		QToolButton* importButton_;

		QListWidgetItem* clickedItem_;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<unreal::UnrealProfile> profile_;
	};
}

#endif