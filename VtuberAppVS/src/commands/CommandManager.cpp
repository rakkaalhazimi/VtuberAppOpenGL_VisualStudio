#include "commands/CommandManager.h"

// When you use std::unique_ptr<Command>, 
// you can store any derived command (e.g., RotateBoneCommand, MoveBoneCommand) 
// through a pointer to the base class, enabling polymorphic behavior.
void CommandManager::executeCommand(std::unique_ptr<Command> command)
{
  command->execute();
  undoStack.push(std::move(command));
}

void CommandManager::undo()
{
  if (!undoStack.empty())
  {
    undoStack.top()->undo();
    undoStack.pop();
  }
}