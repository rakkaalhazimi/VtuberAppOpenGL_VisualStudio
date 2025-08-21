#pragma once

#include <memory>
#include <stack>

#include "commands/Command.h"


class CommandManager
{
  private:
    std::stack<std::unique_ptr<Command>> undoStack;

  public:
    void executeCommand(std::unique_ptr<Command> command);
    void undo();
};
