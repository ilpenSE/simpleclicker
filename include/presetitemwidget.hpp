#pragma once
#include "common.hpp"
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QEnterEvent>
#include <QEvent>
#include <QLineEdit>
#include <cstdint>
#include "presets.hpp"

enum class PresetItemState : uint8_t {
  None = 0,
  Saved = 1 << 0,
  Renaming = 1 << 1,
  Unsaved = 1 << 2,
  Count = 4,
};

constexpr bool any(PresetItemState s) {
  return static_cast<uint8_t>(s) != 0;
}

constexpr PresetItemState operator |(PresetItemState a, PresetItemState b) {
  return static_cast<PresetItemState>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

constexpr PresetItemState operator &(PresetItemState a, PresetItemState b) {
  return static_cast<PresetItemState>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

constexpr PresetItemState operator ~(PresetItemState a) {
  return static_cast<PresetItemState>(~static_cast<uint8_t>(a));
}

constexpr PresetItemState& operator |=(PresetItemState &a, PresetItemState b) {
  a = a | b;
  return a;
}

constexpr PresetItemState& operator &=(PresetItemState &a, PresetItemState b) {
  a = a & b;
  return a;
}

constexpr bool operator &(PresetItemState a, bool) = delete;

inline constexpr bool is_set(PresetItemState states, PresetItemState value) {
  return (states & value) != PresetItemState::None;
}

class PresetItemWidget : public QWidget {
  Q_OBJECT
public:
  PresetItemWidget(const QString& presetName, const PresetConfig& config, QWidget *parent = nullptr);

  void setActive(bool isActive);
  bool isActive() { return m_is_active; }
  struct PresetConfig config;

  const QString& presetName() const { return m_presetName; }
  void setPresetName(const QString& newName);

  bool isSaved() const { return any(m_state & PresetItemState::Saved); }
  bool isUnsaved() const { return any(m_state & PresetItemState::Unsaved); }
  bool isRenaming() const { return any(m_state & PresetItemState::Renaming); }
  void markSaved();
  void markUnsaved();
  void enterEditMode();

signals:
  void renameRequested(const QString &newName);
  void deleteRequested();
  void saveRequested();
  void cancelRequested();

private slots:
  void onEditClicked();
  void onSaveClicked();
  void onCancelClicked();
  void onDeleteClicked();

protected:
  void enterEvent(QEnterEvent *event) override;
  void leaveEvent(QEvent *event) override;
  bool eventFilter(QObject *watched, QEvent *event) override;

private:
  void commitRename();
  void abortRename();
  void updateButtonVisibility();
  void updateLabelStyle();
  void updateHoverIcons(bool hovering);

  QLabel *m_nameLabel;
  QLineEdit *m_nameEdit;
  QPushButton *m_saveButton;
  QPushButton *m_cancelButton;
  QPushButton *m_editButton;
  QPushButton *m_deleteButton;

  QString m_presetName{};
  PresetItemState m_state = PresetItemState::None;
  bool m_is_active = false;
};
