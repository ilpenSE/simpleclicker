#include "presetitemwidget.hpp"
#include <QHBoxLayout>
#include "common.hpp"
#include "logger.hpp"
extern Logger *lg;

constexpr auto itemSize = 26;

namespace {
QPushButton *makeIconButton(const QString& symbol, const QString& tooltip, QWidget *parent) {
  auto *btn = new QPushButton(parent);
  btn->setIcon(loadIconFromSVG(symbol));
  btn->setFlat(true);
  btn->setCursor(Qt::PointingHandCursor);
  btn->setFixedSize(itemSize, itemSize);
  btn->setToolTip(tooltip);
  return btn;
}
} // namespace

PresetItemWidget::PresetItemWidget(const QString& presetName, const PresetConfig& config, QWidget *parent)
  : config(config), QWidget(parent), m_presetName(presetName)
{
  auto *layout = new QHBoxLayout(this);
  layout->setContentsMargins(8, 4, 8, 4);
  layout->setSpacing(4);

  m_nameLabel = new QLabel(presetName, this);
  m_nameEdit = new QLineEdit(this);
  m_nameEdit->hide();
  layout->addWidget(m_nameLabel, 1);
  layout->addWidget(m_nameEdit, 1);

  m_nameLabel->setFixedHeight(itemSize);
  m_nameEdit->setFixedHeight(itemSize);
  m_nameLabel->installEventFilter(this); // for double click events

  m_saveButton = makeIconButton(":/icons/save.svg", "Save", this);
  m_cancelButton = makeIconButton(":/icons/cancel.svg", "Cancel", this);
  m_editButton = makeIconButton(":/icons/edit.svg", "Edit", this);
  m_deleteButton = makeIconButton(":/icons/trash.svg", "Delete", this);

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
  connect(m_nameEdit, &QLineEdit::returnPressed, this, &PresetItemWidget::onSaveClicked);

  updateHoverIcons(false);
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
  updateButtonVisibility();
}

void PresetItemWidget::markUnsaved() {
  m_state |= PresetItemState::Unsaved;
  updateLabelStyle();
  updateButtonVisibility();
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
  updateButtonVisibility();
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
  updateButtonVisibility();
}

void PresetItemWidget::abortRename() {
  m_nameEdit->hide();
  m_nameLabel->show();
  m_state &= ~PresetItemState::Renaming;
  updateButtonVisibility();
}

void PresetItemWidget::enterEvent(QEnterEvent *event) {
  (void)event; updateHoverIcons(true);
}

void PresetItemWidget::leaveEvent(QEvent *event) {
  (void)event; updateHoverIcons(false);
}

bool PresetItemWidget::eventFilter(QObject *watched, QEvent *event) {
  if (watched == m_nameEdit && event->type() == QEvent::KeyPress) {
    auto *keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      onCancelClicked();
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

void PresetItemWidget::updateButtonVisibility() {
  const bool renaming = is_set(m_state, PresetItemState::Renaming);
  const bool unsaved  = is_set(m_state, PresetItemState::Unsaved);

  m_saveButton->setVisible(renaming || unsaved);
  m_cancelButton->setVisible(renaming || unsaved);
  m_editButton->setVisible(!renaming && !unsaved);
  m_deleteButton->setVisible(!renaming && !unsaved);
}

void PresetItemWidget::setActive(bool isActive) {
  m_is_active = isActive;
}
