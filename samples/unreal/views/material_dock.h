#ifndef UNREAL_MATERIAL_WINDOW_H_
#define UNREAL_MATERIAL_WINDOW_H_

#include <qwidget>
#include <qdialog.h>
#include <qboxlayout.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qlistwidget.h>
#include <qcheckbox.h>
#include <qtimer.h>
#include <qcolordialog.h>
#include <optional>
#include <octoon/game_object.h>

#include "unreal_behaviour.h"
#include "material_edit_dock.h"

#include "../widgets/spoiler.h"
#include "../widgets/color_dialog.h"

namespace unreal
{
	class MaterialListPanel final : public QWidget
	{
		Q_OBJECT
	public:
		MaterialListPanel(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept(false);
		~MaterialListPanel() noexcept;

		void addItem(std::string_view uuid) noexcept;
		void addItem(const nlohmann::json& package) noexcept(false);

		void updateItemList();

		void resizeEvent(QResizeEvent* e) noexcept override;

	public Q_SLOTS:
		void itemClicked(QListWidgetItem* item);
		void itemSelected(QListWidgetItem* item);

	public:
		QListWidget* mainWidget_;
		QVBoxLayout* mainLayout_;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<unreal::UnrealProfile> profile_;
	};

	class MaterialAssetPanel final : public QWidget
	{
		Q_OBJECT
	public:
		MaterialAssetPanel(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept(false);
		~MaterialAssetPanel() noexcept;

		void addItem(std::string_view uuid) noexcept(false);

		void updateItemList();

		void resizeEvent(QResizeEvent* e) noexcept override;
		void keyPressEvent(QKeyEvent* event) noexcept;

	public Q_SLOTS:
		void itemClicked(QListWidgetItem* item);
		void itemSelected(QListWidgetItem* item);

	public:
		QListWidget* mainWidget_;
		QVBoxLayout* mainLayout_;
		QListWidgetItem* clickedItem_;
		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<unreal::UnrealProfile> profile_;
	};

	class MaterialDock final : public QDockWidget
	{
		Q_OBJECT
	public:
		MaterialDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<UnrealProfile>& profile) noexcept(false);
		~MaterialDock() noexcept;

		void addItem(std::string_view uuid) noexcept;

	private:
		void showEvent(QShowEvent* e) noexcept override;
		void resizeEvent(QResizeEvent* e) noexcept override;
		void closeEvent(QCloseEvent* e) override;

	private Q_SLOTS:
		void backEvent();
		void itemDoubleClicked(QListWidgetItem* item);
		void importClickEvent();

	private:
		QLabel* title_;
		QVBoxLayout* materialLayout_;
		QVBoxLayout* mainLayout_;
		MaterialListPanel* materialList_;
		MaterialAssetPanel* materialAssetList_;
		MaterialEditWindow* modifyWidget_;
		QScrollArea* modifyMaterialArea_;
		QTabWidget* widget_;
		QWidget* mainWidget_;
		QListWidgetItem* selectedItem_;
		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<UnrealProfile> profile_;
	};
}

#endif