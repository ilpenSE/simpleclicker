#include "presetitemwidget.hpp"
#include <QHBoxLayout>
#include "common.hpp"
#include "theme.hpp"
#include "language.hpp"
#include "logger.hpp"
extern Logger *lg;
extern ThemeManager *thememan;
extern LanguageManager *langman;

namespace {
constexpr auto itemSize = 32;
constexpr auto COMMIT_RENAME_KEYBIND = Qt::Key_Return;
constexpr auto ABORT_RENAME_KEYBIND = Qt::Key_Escape;
constexpr auto COMMIT_RENAME_KEYBIND_STR = "ENTER";
constexpr auto ABORT_RENAME_KEYBIND_STR = "ESC";

QPushButton *makeIconButton(const QString& symbol, int size, QWidget *parent) {
  auto btn = new QPushButton(parent);
  btn->setFlat(true);
  btn->setCursor(Qt::PointingHandCursor);
  btn->setFixedSize(size, size);
  makeDynamicIconButton(btn, symbol);
  return btn;
}

} // namespace

PresetItemWidget::PresetItemWidget(const QString &presetName, const PresetConfig &config, QWidget *parent)
    : config(config), QWidget(parent), m_presetName(presetName)
{
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(8, 8, 8, 8);
  layout->setSpacing(4);

  m_nameLabel = new QLabel(presetName, this);
  m_nameEdit = new QLineEdit(this);
  m_nameEdit->installEventFilter(this);
  m_nameEdit->hide();
  layout->addWidget(m_nameLabel, 1);
  layout->addWidget(m_nameEdit, 1);

  m_nameLabel->setFixedHeight(itemSize);
  m_nameEdit->setFixedHeight(itemSize);
  m_nameLabel->installEventFilter(this); // for double click events

  m_saveButton = makeIconButton("save.svg", itemSize, this);
  m_cancelButton = makeIconButton("cancel.svg",  itemSize, this);
  m_editButton = makeIconButton("rename.svg",  itemSize, this);
  m_deleteButton = makeIconButton("trash.svg",  itemSize, this);

  layout->addWidget(m_saveButton);
  layout->addWidget(m_cancelButton);
  layout->addWidget(m_editButton);
  layout->addWidget(m_deleteButton);

  m_saveButton->hide();
  m_cancelButton->hide();

  connect(m_saveButton, &QPushButton::clicked, this, &PresetItemWidget::onSaveClicked);
  connect(m_cancelButton, &QPushButton::clicked, this, &PresetItemWidget::onCancelClicked);
  connect(m_editButton, &QPushButton::clicked, this, &PresetItemWidget::onEditClicked);
  connect(m_deleteButton, &QPushButton::clicked, this, &PresetItemWidget::onDeleteClicked);

  connect(langman, &LanguageManager::languageChanged, this, &PresetItemWidget::retranslate);
  retranslate();

  updateHoverIcons(false);
}

void PresetItemWidget::retranslate() {
  m_editButton->setToolTip(tr("Rename"));
  m_deleteButton->setToolTip(tr("Delete"));

  if (is_set(m_state, PresetItemState::Renaming)) {
    m_saveButton->setToolTip(QString(tr("Rename") + " (%1)").arg(COMMIT_RENAME_KEYBIND_STR));
    m_cancelButton->setToolTip(QString(tr("Cancel") + "(%1)").arg(ABORT_RENAME_KEYBIND_STR));
  } else {
    m_saveButton->setToolTip(QString(tr("Save") + " (%1)").arg(SAVE_CHANGES_KEYBIND.toString()));
    m_cancelButton->setToolTip(QString(tr("Cancel") + " (%1)").arg(ABORT_CHANGES_KEYBIND.toString()));
  }
}

void PresetItemWidget::enterEditMode() {
  onEditClicked();
}

void PresetItemWidget::updateLabelStyle() {
  bool is_mod = is_set(m_state, PresetItemState::Unsaved);
  QFont f = m_nameLabel->font();
  f.setBold(is_mod);
  f.setItalic(is_mod);
  m_nameLabel->setFont(f);
  m_nameLabel->setText(is_mod ? QStringLiteral("* %1").arg(m_presetName) : m_presetName);
}

void PresetItemWidget::updateHoverIcons(bool hovering) {
  m_hovering = hovering;
  update();

  if (is_set(m_state, PresetItemState::Renaming)) return;
  const bool unsaved = is_set(m_state, PresetItemState::Unsaved);

  m_editButton->setVisible(hovering && !unsaved);
  m_deleteButton->setVisible(hovering && !unsaved);
}

void PresetItemWidget::setPresetName(const QString& newName) {
  m_presetName = newName;
  updateLabelStyle();
}

void PresetItemWidget::markSaved() {
  m_state &= ~PresetItemState::Unsaved;
  updateLabelStyle();
  updateButtons();
}

void PresetItemWidget::markUnsaved() {
  m_state |= PresetItemState::Unsaved;
  updateLabelStyle();
  updateButtons();
}

void PresetItemWidget::onDeleteClicked() {
  emit deleteRequested();
}

// beginRename()
void PresetItemWidget::onEditClicked() {
  m_nameEdit->setText(m_presetName);
  m_nameLabel->hide();
  m_nameEdit->show();
  m_nameEdit->setFocus();
  m_nameEdit->selectAll();

  m_state |= PresetItemState::Renaming;
  updateButtons();
}

void PresetItemWidget::onSaveClicked() {
  if (is_set(m_state, PresetItemState::Renaming)) {
    commitRename();
  }
  if (is_set(m_state, PresetItemState::Unsaved)) {
    emit saveRequested();
  }
}

void PresetItemWidget::onCancelClicked() {
  if (is_set(m_state, PresetItemState::Renaming)) {
    abortRename();
  }
  if (is_set(m_state, PresetItemState::Unsaved)) {
    emit cancelRequested();
  }
}

// endRename()
void PresetItemWidget::commitRename() {
  const QString newName = m_nameEdit->text().trimmed();
  m_nameEdit->hide();
  m_nameLabel->show();

  m_state &= ~PresetItemState::Renaming;

  if (!newName.isEmpty() && newName != m_presetName) {
    emit renameRequested(newName);
    updateLabelStyle();
  }
  updateButtons();
}

void PresetItemWidget::abortRename() {
  m_nameEdit->hide();
  m_nameLabel->show();
  m_state &= ~PresetItemState::Renaming;
  updateButtons();
}

void PresetItemWidget::enterEvent(QEnterEvent *event) {
  (void)event; updateHoverIcons(true);
}

void PresetItemWidget::leaveEvent(QEvent *event) {
  (void)event; updateHoverIcons(false);
}

bool PresetItemWidget::eventFilter(QObject *watched, QEvent *event) {
  if (watched == m_nameEdit && event->type() == QEvent::KeyPress) {
    auto keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == ABORT_RENAME_KEYBIND) {
      onCancelClicked();
      return true;
    } else if (keyEvent->key() == COMMIT_RENAME_KEYBIND) {
      onSaveClicked();
      return true;
    }
  }

  if (watched == m_nameLabel && event->type() == QEvent::MouseButtonDblClick) {
    emit doubleClicked();
    return true;
  }

  return QWidget::eventFilter(watched, event);
}

void PresetItemWidget::mouseDoubleClickEvent(QMouseEvent *event) {
  emit doubleClicked();
  QWidget::mouseDoubleClickEvent(event);
}

void PresetItemWidget::updateButtons() {
  const bool renaming = is_set(m_state, PresetItemState::Renaming);
  const bool unsaved  = is_set(m_state, PresetItemState::Unsaved);

  m_saveButton->setVisible(renaming || unsaved);
  m_cancelButton->setVisible(renaming || unsaved);
  m_editButton->setVisible(!renaming && !unsaved);
  m_deleteButton->setVisible(!renaming && !unsaved);

  if (renaming) {
    m_saveButton->setToolTip(QString(tr("Rename") + " (%1)").arg(COMMIT_RENAME_KEYBIND_STR));
    m_cancelButton->setToolTip(QString(tr("Cancel") + " (%1)").arg(ABORT_RENAME_KEYBIND_STR));
  } else {
    m_saveButton->setToolTip(QString(tr("Save") + " (%1)").arg(SAVE_CHANGES_KEYBIND.toString()));
    m_cancelButton->setToolTip(QString(tr("Cancel") + " (%1)").arg(ABORT_CHANGES_KEYBIND.toString()));
  }
}

void PresetItemWidget::setActive(bool isActive) {
  if (m_active == isActive) return;
  m_active = isActive;
  update();
}

void PresetItemWidget::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  if (m_active) {
    if (m_hovering) {
      p.fillRect(rect(), thememan->color("preset-button-active-hover"));
    } else {
      p.fillRect(rect(), thememan->color("preset-button-active"));
    }
    p.fillRect(QRect(0, 0, 3, height()), thememan->color("preset-button-accent"));
  } else {
    if (m_hovering) {
      p.fillRect(rect(), thememan->color("preset-button-hover"));
    } else {
      p.fillRect(rect(), thememan->color("preset-button"));
    }
  }

  QWidget::paintEvent(event);
}
