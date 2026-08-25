#include "click.hpp"
#include "logger.hpp"
extern Logger *lg;

ClickEngine::ClickEngine(PresetConfig config, QObject *parent) : m_preset(config), QObject(parent) {

}

void ClickEngine::start() {
  if (running) return;
  running = true;
}

void ClickEngine::stop() {
  if (!running) return;
  running = false;
}

ClickEngine::~ClickEngine() {
  stop();
}
