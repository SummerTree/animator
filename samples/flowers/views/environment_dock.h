#ifndef FLOWER_ENVIRONMENT_WINDOW_H_
#define FLOWER_ENVIRONMENT_WINDOW_H_

#include <qdockwidget.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qlistwidget.h>
#include <qspinbox.h>
#include <qlabel.h>

#include "spoiler.h"
#include "flower_profile.h"
#include "flower_behaviour.h"
#include <octoon/hal/graphics.h>

namespace flower
{
	class EnvironmentListDialog final : public QDialog
	{
		Q_OBJECT
	public:
		EnvironmentListDialog(QWidget* parent, const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile) noexcept(false);
		~EnvironmentListDialog() noexcept;

		void resizeEvent(QResizeEvent* e) noexcept override;
		void showEvent(QShowEvent* event) noexcept override;

	public Q_SLOTS:
		void okClickEvent();
		void closeClickEvent();
		void importClickEvent();
		void itemClicked(QListWidgetItem* item);
		void itemDoubleClicked(QListWidgetItem* item);

	Q_SIGNALS:
		void chooseItem(QString uuid);

	private:
		void addItem(std::string_view uuid) noexcept;

	public:
		QListWidget* mainWidget_;
		QVBoxLayout* mainLayout_;

		QToolButton* okButton_;
		QToolButton* closeButton_;
		QToolButton* importButton_;

		QListWidgetItem* clickedItem_;

		octoon::GameObjectPtr behaviour_;
		std::shared_ptr<flower::FlowerProfile> profile_;
	};

   	class EnvironmentDock final : public QDockWidget
	{
		Q_OBJECT
	public:
		EnvironmentDock(const octoon::GameObjectPtr& behaviour, const std::shared_ptr<FlowerProfile>& profile);
		~EnvironmentDock();

		void showEvent(QShowEvent* event) override;
		void closeEvent(QCloseEvent* event) override;

	public Q_SLOTS:
		void previewClickEvent(bool checked);
		void thumbnailClickEvent();
		void thumbnailToggleEvent(int state);
		void backgroundMapCheckEvent(int state);
		void colorClickEvent();
		void colorChangeEvent(const QColor&);
		void resetEvent();
		void intensitySliderEvent(int);
		void intensityEditEvent(double value);
		void horizontalRotationSliderEvent(int);
		void horizontalRotationEditEvent(double value);
		void verticalRotationSliderEvent(int);
		void verticalRotationEditEvent(double value);
		void chooseItem(QString uuid);

	private:
		void setColor(const QColor& c, int w = 50, int h = 26);
		void setPreviewImage(QString name, std::shared_ptr<QImage> image);
		void setThumbnailImage(QString name, const QImage& image);
		void updatePreviewImage();
		
	private:
		QLabel* previewName_;
		QLabel* thumbnailPath;
		QLabel* intensityLabel_;
		QLabel* horizontalRotationLabel_;
		QLabel* verticalRotationLabel_;

		QToolButton* previewButton_;
		QToolButton* colorButton;
		QToolButton* thumbnail;
		QToolButton* resetButton_;

		QCheckBox* thumbnailToggle;
		QCheckBox* backgroundToggle;

		QSlider* intensitySlider;
		QSlider* horizontalRotationSlider;
		QSlider* verticalRotationSlider;

		QDoubleSpinBox* intensitySpinBox;
		QDoubleSpinBox* horizontalRotationSpinBox;
		QDoubleSpinBox* verticalRotationSpinBox;

		EnvironmentListDialog* environmentListDialog_;

		QColorDialog colorSelector_;

		Spoiler* spoiler;

		octoon::GameObjectPtr behaviour_;
		octoon::hal::GraphicsTexturePtr texture_;
		octoon::hal::GraphicsTexturePtr irradianceTexture_;

		std::shared_ptr<QImage> previewImage_;
		std::shared_ptr<flower::FlowerProfile> profile_;
	};
}

#endif