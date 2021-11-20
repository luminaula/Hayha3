#pragma once

#include "INeuralNet.hpp"
#include "hcore.hpp"
#include "module.hpp"

class Trainer : public Threadable, public Module {
  private:
    INeuralNet *m_net;
    std::vector<std::string> m_trainData;

  public:
    MFunc<INeuralNet *> m_detectorCreate;

    Trainer(HCore::HCore *core);

    void addImage(std::string &filename);
    void clearData();
    void readFolder(std::string &foldername);

    void work() override;
};